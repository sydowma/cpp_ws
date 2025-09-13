#include "../src/okx_websocket_client.h"
#include <iostream>
#include <chrono>
#include <thread>

void test_connection(const std::string& description, const std::string& host, int port, const std::string& path, bool use_ssl) {
    std::cout << "🔗 测试: " << description << std::endl;
    std::cout << "   端点: " << (use_ssl ? "wss://" : "ws://") << host << ":" << port << path << std::endl;

    OKXWebSocketClient client;

    // 设置简单的回调来测试连接
    bool received_data = false;
    client.set_ticker_callback([&received_data](const TickerData& ticker) {
        std::cout << "   ✅ 收到ticker数据: " << ticker.inst_id << std::endl;
        received_data = true;
    });

    // 尝试连接
    if (client.connect(host, port, path, use_ssl)) {
        std::cout << "   ✅ 初始连接成功" << std::endl;

        // 等待连接建立
        std::this_thread::sleep_for(std::chrono::seconds(3));

        if (client.is_connected()) {
            std::cout << "   ✅ WebSocket连接已建立" << std::endl;

            // 尝试订阅
            if (client.subscribe_ticker("BTC-USDT")) {
                std::cout << "   ✅ 订阅请求已发送" << std::endl;

                // 等待数据
                std::cout << "   ⏳ 等待ticker数据..." << std::endl;
                for (int i = 0; i < 10 && !received_data && client.is_connected(); i++) {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }

                if (received_data) {
                    std::cout << "   🎉 测试成功!" << std::endl;
                } else {
                    std::cout << "   ⚠️  未收到数据，但连接正常" << std::endl;
                }
            } else {
                std::cout << "   ❌ 订阅失败" << std::endl;
            }
        } else {
            std::cout << "   ❌ WebSocket连接未建立" << std::endl;
        }

        client.disconnect();
    } else {
        std::cout << "   ❌ 初始连接失败" << std::endl;
    }

    std::cout << std::endl;
}

int main() {
    std::cout << "🚀 OKX WebSocket连接测试工具" << std::endl;
    std::cout << "===============================" << std::endl << std::endl;

    // 测试不同的连接配置
    test_connection("标准OKX WSS连接", "ws.okx.com", 8443, "/ws/v5/public", true);

    // 如果SSL失败，尝试其他配置
    std::cout << "如果上面的SSL连接失败，这可能是由于:" << std::endl;
    std::cout << "1. 网络防火墙阻止WSS连接" << std::endl;
    std::cout << "2. SSL/TLS版本不兼容" << std::endl;
    std::cout << "3. 证书验证问题" << std::endl << std::endl;

    std::cout << "建议的解决方案:" << std::endl;
    std::cout << "1. 检查网络连接和防火墙设置" << std::endl;
    std::cout << "2. 使用VPN或代理" << std::endl;
    std::cout << "3. 联系网络管理员" << std::endl;

    return 0;
}