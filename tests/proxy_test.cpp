#include "../src/okx_websocket_client.h"
#include <iostream>
#include <chrono>
#include <thread>

void test_connection_with_proxy(const std::string& proxy_type, const std::string& proxy_host, int proxy_port) {
    std::cout << "\n🌐 测试" << proxy_type << "代理连接" << std::endl;
    std::cout << "=================================" << std::endl;

    OKXWebSocketClient client;

    // 设置代理
    if (proxy_type == "HTTP") {
        client.set_http_proxy(proxy_host, proxy_port);
    } else if (proxy_type == "SOCKS") {
        client.set_socks_proxy(proxy_host, proxy_port);
    }

    // 设置回调
    bool received_data = false;
    client.set_ticker_callback([&received_data](const TickerData& ticker) {
        std::cout << "✅ 收到数据: " << ticker.inst_id << " = $" << ticker.last << std::endl;
        received_data = true;
    });

    std::cout << "尝试通过代理连接到 OKX WebSocket..." << std::endl;

    if (client.connect()) {
        std::cout << "✅ 初始连接调用成功" << std::endl;

        // 等待连接建立
        std::cout << "⏳ 等待连接建立..." << std::endl;
        for (int i = 0; i < 15; i++) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (client.is_connected()) {
                std::cout << "🎉 WebSocket连接已建立!" << std::endl;
                break;
            }
            if (i % 3 == 2) {
                std::cout << "   仍在连接中... " << (i+1) << "/15" << std::endl;
            }
        }

        if (client.is_connected()) {
            std::cout << "📡 尝试订阅BTC-USDT ticker数据..." << std::endl;
            if (client.subscribe_ticker("BTC-USDT")) {
                std::cout << "✅ 订阅请求已发送" << std::endl;

                // 等待数据
                for (int i = 0; i < 10 && !received_data && client.is_connected(); i++) {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }

                if (received_data) {
                    std::cout << "🚀 代理连接测试成功!" << std::endl;
                } else {
                    std::cout << "⚠️ 连接成功但未收到数据" << std::endl;
                }
            } else {
                std::cout << "❌ 订阅失败" << std::endl;
            }
        } else {
            std::cout << "❌ 连接超时 - 可能是代理配置问题" << std::endl;
        }

        client.disconnect();
    } else {
        std::cout << "❌ 连接初始化失败" << std::endl;
    }

    // 清理代理设置
    client.clear_proxy();
}

int main() {
    std::cout << "🔗 OKX WebSocket 代理连接测试工具" << std::endl;
    std::cout << "====================================" << std::endl;

    // std::cout << "\n📋 支持的代理类型:" << std::endl;
    // std::cout << "1. HTTP/HTTPS 代理 (推荐)" << std::endl;
    // std::cout << "2. SOCKS5 代理 (实验性)" << std::endl;
    //
    // std::cout << "\n🛠️ 使用方法示例:" << std::endl;
    // std::cout << "代码中设置HTTP代理:" << std::endl;
    // std::cout << "  client.set_http_proxy(\"127.0.0.1\", 6152);" << std::endl;
    // std::cout << "  client.set_http_proxy(\"proxy.example.com\", 6152, \"username\", \"password\");" << std::endl;
    //
    // std::cout << "\n代码中设置SOCKS代理:" << std::endl;
    // std::cout << "  client.set_socks_proxy(\"127.0.0.1\", 1080);" << std::endl;
    // std::cout << "  client.set_socks_proxy(\"proxy.example.com\", 1080, \"username\", \"password\");" << std::endl;
    //
    // std::cout << "\n⚠️ 注意事项:" << std::endl;
    // std::cout << "- 确保代理服务器正在运行且可访问" << std::endl;
    // std::cout << "- HTTP代理比SOCKS代理有更好的兼容性" << std::endl;
    // std::cout << "- 某些代理可能需要认证凭据" << std::endl;
    //
    // std::cout << "\n🧪 测试常见代理设置..." << std::endl;
    //
    // // 测试本地代理（用户需要自己设置）
    // std::cout << "\n如果你有本地代理，请修改代码中的代理地址和端口" << std::endl;
    // std::cout << "常见代理端口:" << std::endl;
    // std::cout << "- HTTP代理: 8080, 3128, 8888" << std::endl;
    // std::cout << "- SOCKS代理: 1080, 1081" << std::endl;

    // 提供一个测试示例（需要用户有实际代理）

    // 取消注释以下代码来测试你的代理
    std::cout << "\n测试本地HTTP代理 127.0.0.1:8080..." << std::endl;
    test_connection_with_proxy("HTTP", "127.0.0.1", 6152);

    std::cout << "\n测试本地SOCKS代理 127.0.0.1:1080..." << std::endl;
    test_connection_with_proxy("SOCKS", "127.0.0.1", 6153);


    std::cout << "\n💡 代理配置建议:" << std::endl;
    std::cout << "1. 使用Clash、V2Ray等工具提供本地代理" << std::endl;
    std::cout << "2. 配置HTTP代理模式以获得最佳兼容性" << std::endl;
    std::cout << "3. 确保代理支持HTTPS CONNECT方法" << std::endl;
    std::cout << "4. 测试代理是否能访问 ws.okx.com:8443" << std::endl;

    return 0;
}