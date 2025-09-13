#include "okx_websocket_client.h"
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

std::atomic<bool> keep_running(true);

void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    keep_running = false;
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    OKXWebSocketClient client;

    client.set_ticker_callback([](const TickerData& ticker) {
        std::cout << "🚀 [" << ticker.inst_id << "] "
                  << "Last: $" << ticker.last
                  << " | Bid: $" << ticker.bid_px
                  << " | Ask: $" << ticker.ask_px
                  << " | 24h Volume: " << ticker.vol24h
                  << " | High: $" << ticker.high24h
                  << " | Low: $" << ticker.low24h << std::endl;
    });

    // 代理设置示例 (如果需要代理，取消注释以下行)
    // client.set_http_proxy("127.0.0.1", 6152);  // HTTP代理
    // client.set_http_proxy("proxy.example.com", 8080, "username", "password");  // 带认证的HTTP代理
    // client.set_socks_proxy("127.0.0.1", 1080);  // SOCKS5代理

    std::cout << "Connecting to OKX WebSocket..." << std::endl;
    if (!client.connect()) {
        std::cerr << "Failed to connect to OKX WebSocket" << std::endl;
        std::cerr << "如果遇到连接问题，请尝试:" << std::endl;
        std::cerr << "1. 运行 ./simple_connect_test 进行诊断" << std::endl;
        std::cerr << "2. 如果需要代理，运行 ./proxy_test" << std::endl;
        return 1;
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    if (!client.is_connected()) {
        std::cerr << "Connection not established" << std::endl;
        return 1;
    }

    std::cout << "Subscribing to BTC-USDT ticker..." << std::endl;
    client.subscribe_ticker("BTC-USDT");

    std::cout << "Subscribing to ETH-USDT ticker..." << std::endl;
    client.subscribe_ticker("ETH-USDT");

    std::cout << "Listening for ticker data... (Press Ctrl+C to exit)" << std::endl;

    while (keep_running && client.is_connected()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "Disconnecting..." << std::endl;
    client.disconnect();

    return 0;
}