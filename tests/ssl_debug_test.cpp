#include "../src/okx_websocket_client.h"
#include <iostream>
#include <chrono>
#include <thread>

void test_ssl_connection() {
    std::cout << "🔐 SSL连接调试测试" << std::endl;
    std::cout << "===================" << std::endl;

    OKXWebSocketClient client;

    // 启用详细日志
    std::cout << "启用详细SSL调试日志..." << std::endl;

    std::cout << "\n🌐 测试1: 标准HTTPS连接" << std::endl;
    if (client.connect("ws.okx.com", 8443, "/ws/v5/public", true)) {
        std::cout << "✅ 连接初始化成功" << std::endl;

        for (int i = 0; i < 10; i++) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (client.is_connected()) {
                std::cout << "🎉 SSL连接成功!" << std::endl;
                client.disconnect();
                return;
            }
            std::cout << "   等待SSL握手... " << (i+1) << "/10" << std::endl;
        }

        std::cout << "❌ SSL握手超时" << std::endl;
        client.disconnect();
    }

    std::cout << "\n🌐 测试2: 尝试不同端口" << std::endl;
    if (client.connect("ws.okx.com", 443, "/ws/v5/public", true)) {
        std::cout << "✅ 端口443连接初始化成功" << std::endl;

        for (int i = 0; i < 10; i++) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (client.is_connected()) {
                std::cout << "🎉 端口443连接成功!" << std::endl;
                client.disconnect();
                return;
            }
        }
        client.disconnect();
    }

    std::cout << "\n🌐 测试3: 非SSL连接 (测试用)" << std::endl;
    std::cout << "注意: OKX实际不支持非SSL WebSocket，这仅用于测试网络连通性" << std::endl;
    if (client.connect("ws.okx.com", 80, "/ws/v5/public", false)) {
        std::cout << "✅ 非SSL连接尝试" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        client.disconnect();
    }

    std::cout << "\n💡 SSL问题诊断建议:" << std::endl;
    std::cout << "1. 检查系统时间是否正确" << std::endl;
    std::cout << "2. 更新CA证书: brew install ca-certificates" << std::endl;
    std::cout << "3. 检查防火墙是否阻止HTTPS连接" << std::endl;
    std::cout << "4. 尝试使用curl测试: curl -v https://ws.okx.com" << std::endl;
    std::cout << "5. 考虑使用代理或VPN" << std::endl;
}

void test_proxy_with_ssl() {
    std::cout << "\n🌐 代理+SSL连接测试" << std::endl;
    std::cout << "===================" << std::endl;

    OKXWebSocketClient client;

    // 设置代理 (用户需要修改为实际代理地址)
    std::cout << "设置测试代理..." << std::endl;
    client.set_http_proxy("127.0.0.1", 8080);  // 用户需要修改

    std::cout << "尝试通过代理连接..." << std::endl;
    if (client.connect()) {
        std::cout << "✅ 代理连接初始化成功" << std::endl;

        for (int i = 0; i < 15; i++) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (client.is_connected()) {
                std::cout << "🎉 代理+SSL连接成功!" << std::endl;
                client.disconnect();
                return;
            }
            if (i % 3 == 2) {
                std::cout << "   代理SSL握手中... " << (i+1) << "/15" << std::endl;
            }
        }

        std::cout << "❌ 代理SSL握手失败" << std::endl;
        client.disconnect();
    }

    client.clear_proxy();
}

int main() {
    std::cout << "🔍 OKX WebSocket SSL/代理 综合诊断工具" << std::endl;
    std::cout << "=======================================" << std::endl;

    // 测试基础SSL连接
    test_ssl_connection();

    // 测试代理连接 (需要用户有实际代理)
    std::cout << "\n是否要测试代理连接? (需要你有可用的HTTP代理)" << std::endl;
    std::cout << "如果有代理，请修改 test_proxy_with_ssl() 中的代理地址" << std::endl;

    // test_proxy_with_ssl();  // 用户取消注释并设置正确代理地址

    std::cout << "\n🚀 建议的解决步骤:" << std::endl;
    std::cout << "1. 如果都失败，问题可能是网络环境限制" << std::endl;
    std::cout << "2. 尝试手机热点或其他网络环境" << std::endl;
    std::cout << "3. 使用代理工具如Clash、V2Ray等" << std::endl;
    std::cout << "4. 联系网络管理员确认防火墙设置" << std::endl;

    return 0;
}