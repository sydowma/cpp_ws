#pragma once
#include "ticker_handler.h"
#include <libwebsockets.h>
#include <memory>
#include <string>
#include <atomic>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>

class OKXWebSocketClient {
public:
    OKXWebSocketClient();
    ~OKXWebSocketClient();

    bool connect(const std::string& host = "ws.okx.com", int port = 8443, const std::string& path = "/ws/v5/public", bool use_ssl = true);
    void disconnect();
    bool subscribe_ticker(const std::string& inst_id);
    void set_ticker_callback(TickerHandler::TickerCallback callback);
    void run();
    bool is_connected() const;
    void enable_auto_reconnect(bool enable = true);
    void set_ping_interval(int seconds = 30);

    // 代理设置
    void set_http_proxy(const std::string& proxy_host, int proxy_port, const std::string& username = "", const std::string& password = "");
    void set_socks_proxy(const std::string& proxy_host, int proxy_port, const std::string& username = "", const std::string& password = "");
    void clear_proxy();

    static int callback_function(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len);

private:
    struct lws_context* context_;
    struct lws* wsi_;
    struct lws_context_creation_info info_;
    struct lws_client_connect_info ccinfo_;

    std::unique_ptr<TickerHandler> ticker_handler_;
    std::atomic<bool> connected_;
    std::atomic<bool> should_run_;
    std::thread worker_thread_;

    std::queue<std::string> send_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;

    std::string host_;
    int port_;
    std::string path_;

    bool auto_reconnect_;
    int ping_interval_;
    std::chrono::steady_clock::time_point last_ping_;
    std::chrono::steady_clock::time_point last_pong_;
    int reconnect_attempts_;
    static constexpr int max_reconnect_attempts_ = 10;

    // 代理配置
    std::string proxy_host_;
    int proxy_port_;
    std::string proxy_username_;
    std::string proxy_password_;
    bool use_http_proxy_;
    bool use_socks_proxy_;

    void send_message(const std::string& message);
    void handle_connection_established();
    void handle_connection_closed();
    void handle_receive(const std::string& data);
    void worker_loop();
    void process_send_queue();
    void attempt_reconnect();
    void send_ping();
    bool should_reconnect() const;
};