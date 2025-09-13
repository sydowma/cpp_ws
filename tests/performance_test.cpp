#include "../src/json_parser.h"
#include <iostream>
#include <chrono>
#include <vector>

int main() {
    // 模拟OKX ticker消息 (紧凑格式以提高性能测试准确性)
    const std::string test_ticker_json = R"({"arg":{"channel":"tickers","instId":"BTC-USDT"},"data":[{"instType":"SPOT","instId":"BTC-USDT","last":"43250.5","lastSz":"0.1234","askPx":"43251.0","askSz":"1.5","bidPx":"43249.5","bidSz":"2.3","open24h":"42000.0","high24h":"43500.0","low24h":"41500.0","volCcy24h":"1234567.89","vol24h":"29.456","sodUtc0":"42100.0","sodUtc8":"42150.0","ts":"1703073600000"}]})";

    const int iterations = 100000;
    std::cout << "🚀 高性能JSON解析器基准测试" << std::endl;
    std::cout << "测试消息大小: " << test_ticker_json.size() << " 字节" << std::endl;
    std::cout << "迭代次数: " << iterations << std::endl << std::endl;

    // 预热
    for (int i = 0; i < 1000; ++i) {
        auto result = JsonParser::parse_ticker_data(test_ticker_json);
    }

    auto start = std::chrono::high_resolution_clock::now();

    int successful_parses = 0;
    for (int i = 0; i < iterations; ++i) {
        auto result = JsonParser::parse_ticker_data(test_ticker_json);
        if (result && !result->empty()) {
            successful_parses++;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "✅ 解析结果:" << std::endl;
    std::cout << "  成功解析: " << successful_parses << "/" << iterations << std::endl;
    std::cout << "  总时间: " << duration.count() << " 微秒" << std::endl;
    std::cout << "  平均每次: " << (duration.count() / (double)iterations) << " 微秒" << std::endl;
    std::cout << "  吞吐量: " << (iterations * 1000000.0 / duration.count()) << " 消息/秒" << std::endl;

    // 验证解析正确性
    auto sample = JsonParser::parse_ticker_data(test_ticker_json);
    if (sample && !sample->empty()) {
        const auto& ticker = (*sample)[0];
        std::cout << std::endl << "📊 解析结果验证:" << std::endl;
        std::cout << "  交易对: " << ticker.inst_id << std::endl;
        std::cout << "  最新价: " << ticker.last << std::endl;
        std::cout << "  买价: " << ticker.bid_px << std::endl;
        std::cout << "  卖价: " << ticker.ask_px << std::endl;
        std::cout << "  24h成交量: " << ticker.vol24h << std::endl;
    }

    std::cout << std::endl << "🏆 性能等级评估:" << std::endl;
    double ops_per_sec = iterations * 1000000.0 / duration.count();
    if (ops_per_sec > 1000000) {
        std::cout << "  ⭐⭐⭐ 超高性能: " << (int)(ops_per_sec/1000) << "K+ ops/sec" << std::endl;
    } else if (ops_per_sec > 500000) {
        std::cout << "  ⭐⭐ 高性能: " << (int)(ops_per_sec/1000) << "K+ ops/sec" << std::endl;
    } else {
        std::cout << "  ⭐ 标准性能: " << (int)(ops_per_sec/1000) << "K+ ops/sec" << std::endl;
    }

    return 0;
}