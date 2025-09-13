#include "../src/okx_websocket_client.h"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "🔥 简单连接测试 - OKX WebSocket" << std::endl;
    std::cout << "=================================" << std::endl;

    OKXWebSocketClient client;


    std::cout << "尝试连接到 wss://ws.okx.com:8443/ws/v5/public" << std::endl;

    // 减少日志噪音
    //lws_set_log_level(LLL_ERR | LLL_WARN, nullptr);

    if (client.connect()) {
        std::cout << "✅ connect() 调用成功" << std::endl;

        // 等待更长时间让连接建立
        std::cout << "⏳ 等待连接建立..." << std::endl;
        for (int i = 0; i < 10; i++) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (client.is_connected()) {
                std::cout << "🎉 连接成功建立!" << std::endl;
                break;
            }
            std::cout << "   等待中... " << (i+1) << "/10" << std::endl;
        }

        if (!client.is_connected()) {
            std::cout << "❌ 连接超时，可能是SSL/TLS握手问题" << std::endl;
        }

        client.disconnect();
    } else {
        std::cout << "❌ 连接初始化失败" << std::endl;
    }

    std::cout << "\n💡 如果连接失败，可能的原因:" << std::endl;
    std::cout << "   1. 网络防火墙阻止了WSS连接" << std::endl;
    std::cout << "   2. SSL/TLS协议版本不兼容" << std::endl;
    std::cout << "   3. 代理服务器干扰" << std::endl;
    std::cout << "   4. OKX服务器临时不可用" << std::endl;

    return 0;
}