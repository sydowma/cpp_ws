#include "../src/okx_websocket_client.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "Testing OKX WebSocket Client..." << std::endl;

    OKXWebSocketClient client;

    bool received_ticker = false;
    client.set_ticker_callback([&received_ticker](const TickerData& ticker) {
        std::cout << "✅ Test PASSED: Received ticker data for " << ticker.inst_id << std::endl;
        std::cout << "   Last Price: " << ticker.last << std::endl;
        std::cout << "   Bid: " << ticker.bid_px << std::endl;
        std::cout << "   Ask: " << ticker.ask_px << std::endl;
        received_ticker = true;
    });

    std::cout << "Attempting to connect..." << std::endl;
    if (!client.connect()) {
        std::cerr << "❌ Test FAILED: Could not connect to OKX WebSocket" << std::endl;
        return 1;
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));

    if (!client.is_connected()) {
        std::cerr << "❌ Test FAILED: Connection not established" << std::endl;
        return 1;
    }

    std::cout << "✅ Connection established successfully" << std::endl;

    std::cout << "Subscribing to BTC-USDT ticker..." << std::endl;
    if (!client.subscribe_ticker("BTC-USDT")) {
        std::cerr << "❌ Test FAILED: Could not subscribe to ticker" << std::endl;
        return 1;
    }

    std::cout << "Waiting for ticker data..." << std::endl;
    int timeout_seconds = 30;
    int elapsed = 0;

    while (elapsed < timeout_seconds && !received_ticker && client.is_connected()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        elapsed++;
        if (elapsed % 5 == 0) {
            std::cout << "Still waiting... (" << elapsed << "s)" << std::endl;
        }
    }

    if (received_ticker) {
        std::cout << "✅ ALL TESTS PASSED!" << std::endl;
    } else if (!client.is_connected()) {
        std::cerr << "❌ Test FAILED: Connection lost" << std::endl;
        return 1;
    } else {
        std::cerr << "❌ Test FAILED: No ticker data received within " << timeout_seconds << " seconds" << std::endl;
        return 1;
    }

    client.disconnect();
    std::cout << "Test completed successfully" << std::endl;
    return 0;
}