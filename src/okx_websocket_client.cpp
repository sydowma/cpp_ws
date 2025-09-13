#include "okx_websocket_client.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <openssl/ssl.h>

static const struct lws_protocols protocols[] = {
    {
        .name = "okx-websocket",
        .callback = OKXWebSocketClient::callback_function,
        .per_session_data_size = 0,
        .rx_buffer_size = 65536,
        .id = 0,
        .user = nullptr,
        .tx_packet_size = 0
    },
    {
        .name = nullptr,
        .callback = nullptr,
        .per_session_data_size = 0,
        .rx_buffer_size = 0,
        .id = 0,
        .user = nullptr,
        .tx_packet_size = 0
    }
};

OKXWebSocketClient::OKXWebSocketClient()
    : context_(nullptr), wsi_(nullptr), connected_(false), should_run_(false),
      auto_reconnect_(true), ping_interval_(30), reconnect_attempts_(0),
      proxy_port_(0), use_http_proxy_(false), use_socks_proxy_(false) {

    ticker_handler_ = std::make_unique<TickerHandler>([](const TickerData& ticker) {
        std::cout << "[TICKER] " << ticker.inst_id
                  << " Last: " << ticker.last
                  << " Bid: " << ticker.bid_px
                  << " Ask: " << ticker.ask_px
                  << " Volume24h: " << ticker.vol24h << std::endl;
    });

    memset(&info_, 0, sizeof(info_));
    info_.port = CONTEXT_PORT_NO_LISTEN;
    info_.protocols = protocols;
    info_.gid = -1;
    info_.uid = -1;
    info_.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info_.user = this;
    // 设置更兼容的SSL选项
    info_.ssl_options_set = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3;
    info_.ssl_cipher_list = "ECDHE+AESGCM:ECDHE+CHACHA20:DHE+AESGCM:DHE+CHACHA20:!aNULL:!MD5:!DSS";
    info_.ssl_ca_filepath = nullptr; // 不验证CA

    lws_set_log_level(LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO | LLL_DEBUG, nullptr);
}

OKXWebSocketClient::~OKXWebSocketClient() {
    disconnect();
}

bool OKXWebSocketClient::connect(const std::string& host, int port, const std::string& path, bool use_ssl) {
    host_ = host;
    port_ = port;
    path_ = path;

    context_ = lws_create_context(&info_);
    if (!context_) {
        std::cerr << "Failed to create libwebsockets context" << std::endl;
        return false;
    }

    memset(&ccinfo_, 0, sizeof(ccinfo_));
    ccinfo_.context = context_;
    ccinfo_.address = host_.c_str();
    ccinfo_.port = port_;
    ccinfo_.path = path_.c_str();
    ccinfo_.host = lws_canonical_hostname(context_);
    ccinfo_.origin = "origin";
    ccinfo_.protocol = protocols[0].name;

    // 代理配置 - 使用环境变量（libwebsockets会自动检测）
    if (use_http_proxy_) {
        std::cout << "设置HTTP代理环境变量: " << proxy_host_ << ":" << proxy_port_ << std::endl;

        std::string proxy_url = "http://";
        if (!proxy_username_.empty()) {
            proxy_url += proxy_username_ + ":" + proxy_password_ + "@";
        }
        proxy_url += proxy_host_ + ":" + std::to_string(proxy_port_);

        setenv("http_proxy", proxy_url.c_str(), 1);
        setenv("https_proxy", proxy_url.c_str(), 1);
        setenv("HTTP_PROXY", proxy_url.c_str(), 1);
        setenv("HTTPS_PROXY", proxy_url.c_str(), 1);

        std::cout << "代理URL: " << proxy_url << std::endl;
    } else if (use_socks_proxy_) {
        std::cout << "建议使用proxychains进行SOCKS代理:" << std::endl;
        std::cout << "  proxychains4 ./okx_client" << std::endl;
        std::cout << "  参考: PROXYCHAINS_GUIDE.md" << std::endl;
    }
    if (use_ssl) {
        ccinfo_.ssl_connection = LCCSCF_USE_SSL |
                                LCCSCF_ALLOW_SELFSIGNED |
                                LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK |
                                LCCSCF_ALLOW_EXPIRED |
                                LCCSCF_ALLOW_INSECURE;

        std::cout << "SSL配置: 允许自签名证书、跳过主机名验证、允许过期证书" << std::endl;
    } else {
        ccinfo_.ssl_connection = 0;
        std::cout << "使用非SSL连接" << std::endl;
    }
    ccinfo_.userdata = this;

    wsi_ = lws_client_connect_via_info(&ccinfo_);
    if (!wsi_) {
        std::cerr << "Failed to connect to WebSocket" << std::endl;
        lws_context_destroy(context_);
        context_ = nullptr;
        return false;
    }

    should_run_ = true;
    worker_thread_ = std::thread(&OKXWebSocketClient::worker_loop, this);

    return true;
}

void OKXWebSocketClient::disconnect() {
    should_run_ = false;

    if (worker_thread_.joinable()) {
        queue_cv_.notify_all();
        worker_thread_.join();
    }

    if (wsi_) {
        lws_close_reason(wsi_, LWS_CLOSE_STATUS_NORMAL, nullptr, 0);
        wsi_ = nullptr;
    }

    if (context_) {
        lws_context_destroy(context_);
        context_ = nullptr;
    }

    connected_ = false;
}

bool OKXWebSocketClient::subscribe_ticker(const std::string& inst_id) {
    if (!connected_) {
        std::cerr << "Not connected to WebSocket" << std::endl;
        return false;
    }

    std::string subscription = JsonParser::create_subscription_message("tickers", inst_id);
    send_message(subscription);
    return true;
}

void OKXWebSocketClient::set_ticker_callback(TickerHandler::TickerCallback callback) {
    if (ticker_handler_) {
        ticker_handler_->set_callback(std::move(callback));
    }
}

void OKXWebSocketClient::run() {
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

bool OKXWebSocketClient::is_connected() const {
    return connected_.load();
}

int OKXWebSocketClient::callback_function(struct lws* wsi, enum lws_callback_reasons reason, void* /* user */, void* in, size_t len) {
    auto* client = static_cast<OKXWebSocketClient*>(lws_context_user(lws_get_context(wsi)));
    if (!client) return 0;

    switch (reason) {
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            std::cerr << "WebSocket connection error: " << (in ? (char*)in : "unknown") << std::endl;
            client->connected_ = false;
            break;

        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            std::cout << "WebSocket connection established" << std::endl;
            client->handle_connection_established();
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            if (len > 0) {
                std::string data(static_cast<char*>(in), len);
                client->handle_receive(data);
            }
            break;

        case LWS_CALLBACK_CLIENT_CLOSED:
            std::cout << "WebSocket connection closed" << std::endl;
            client->handle_connection_closed();
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
            client->process_send_queue();
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE_PONG:
            client->last_pong_ = std::chrono::steady_clock::now();
            break;

        case LWS_CALLBACK_WSI_DESTROY:
            client->connected_ = false;
            break;

        default:
            break;
    }

    return 0;
}

void OKXWebSocketClient::send_message(const std::string& message) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        send_queue_.push(message);
    }
    queue_cv_.notify_one();
    if (wsi_) {
        lws_callback_on_writable(wsi_);
    }
}

void OKXWebSocketClient::handle_connection_established() {
    connected_ = true;
    reconnect_attempts_ = 0;
    last_ping_ = std::chrono::steady_clock::now();
    last_pong_ = std::chrono::steady_clock::now();
    std::cout << "Connection established successfully" << std::endl;
}

void OKXWebSocketClient::handle_connection_closed() {
    connected_ = false;
    std::cout << "Connection closed" << std::endl;

    if (auto_reconnect_ && should_reconnect()) {
        std::cout << "Attempting to reconnect..." << std::endl;
        attempt_reconnect();
    }
}

void OKXWebSocketClient::handle_receive(const std::string& data) {
    if (ticker_handler_) {
        ticker_handler_->handle_message(data);
    }
}

void OKXWebSocketClient::worker_loop() {
    while (should_run_) {
        if (context_) {
            lws_service(context_, 50);

            auto now = std::chrono::steady_clock::now();
            if (connected_ && std::chrono::duration_cast<std::chrono::seconds>(now - last_ping_).count() >= ping_interval_) {
                send_ping();
            }

            if (connected_ && std::chrono::duration_cast<std::chrono::seconds>(now - last_pong_).count() > ping_interval_ * 2) {
                std::cerr << "Ping timeout, connection may be dead" << std::endl;
                lws_close_reason(wsi_, LWS_CLOSE_STATUS_ABNORMAL_CLOSE, nullptr, 0);
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

void OKXWebSocketClient::process_send_queue() {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    while (!send_queue_.empty() && connected_) {
        const std::string& message = send_queue_.front();

        size_t message_len = message.length();
        unsigned char* buf = new unsigned char[LWS_PRE + message_len];
        memcpy(&buf[LWS_PRE], message.c_str(), message_len);

        int n = lws_write(wsi_, &buf[LWS_PRE], message_len, LWS_WRITE_TEXT);

        delete[] buf;

        if (n < 0) {
            std::cerr << "Failed to send message" << std::endl;
            break;
        }

        send_queue_.pop();
        std::cout << "Sent: " << message << std::endl;
    }
}

void OKXWebSocketClient::enable_auto_reconnect(bool enable) {
    auto_reconnect_ = enable;
}

void OKXWebSocketClient::set_ping_interval(int seconds) {
    ping_interval_ = seconds;
}

void OKXWebSocketClient::attempt_reconnect() {
    if (reconnect_attempts_ >= max_reconnect_attempts_) {
        std::cerr << "Max reconnection attempts reached. Giving up." << std::endl;
        return;
    }

    reconnect_attempts_++;
    int delay = std::min(1000 * (1 << (reconnect_attempts_ - 1)), 30000);
    std::cout << "Reconnect attempt " << reconnect_attempts_ << " in " << delay << "ms..." << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(delay));

    if (wsi_) {
        lws_close_reason(wsi_, LWS_CLOSE_STATUS_NORMAL, nullptr, 0);
        wsi_ = nullptr;
    }

    if (context_) {
        lws_context_destroy(context_);
        context_ = nullptr;
    }

    connect(host_, port_, path_);
}

void OKXWebSocketClient::send_ping() {
    if (wsi_ && connected_) {
        unsigned char ping_payload[LWS_PRE + 125];
        memset(&ping_payload[LWS_PRE], 0, 125);

        int n = lws_write(wsi_, &ping_payload[LWS_PRE], 0, LWS_WRITE_PING);
        if (n >= 0) {
            last_ping_ = std::chrono::steady_clock::now();
        }
    }
}

bool OKXWebSocketClient::should_reconnect() const {
    return should_run_ && reconnect_attempts_ < max_reconnect_attempts_;
}

void OKXWebSocketClient::set_http_proxy(const std::string& proxy_host, int proxy_port, const std::string& username, const std::string& password) {
    proxy_host_ = proxy_host;
    proxy_port_ = proxy_port;
    proxy_username_ = username;
    proxy_password_ = password;
    use_http_proxy_ = true;
    use_socks_proxy_ = false;

    std::cout << "设置HTTP代理: " << proxy_host << ":" << proxy_port;
    if (!username.empty()) {
        std::cout << " (认证: " << username << ")";
    }
    std::cout << std::endl;
}

void OKXWebSocketClient::set_socks_proxy(const std::string& proxy_host, int proxy_port, const std::string& username, const std::string& password) {
    proxy_host_ = proxy_host;
    proxy_port_ = proxy_port;
    proxy_username_ = username;
    proxy_password_ = password;
    use_socks_proxy_ = true;
    use_http_proxy_ = false;

    std::cout << "设置SOCKS代理: " << proxy_host << ":" << proxy_port;
    if (!username.empty()) {
        std::cout << " (认证: " << username << ")";
    }
    std::cout << std::endl;
}

void OKXWebSocketClient::clear_proxy() {
    proxy_host_.clear();
    proxy_port_ = 0;
    proxy_username_.clear();
    proxy_password_.clear();
    use_http_proxy_ = false;
    use_socks_proxy_ = false;

    // 清除环境变量
    unsetenv("http_proxy");
    unsetenv("https_proxy");
    unsetenv("ALL_PROXY");

    std::cout << "清除代理设置" << std::endl;
}