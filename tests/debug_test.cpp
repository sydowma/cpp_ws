#include "../src/json_parser.h"
#include <iostream>

int main() {
    const std::string test_json = R"({
        "arg": {
            "channel": "tickers",
            "instId": "BTC-USDT"
        },
        "data": [
            {
                "instType": "SPOT",
                "instId": "BTC-USDT",
                "last": "43250.5"
            }
        ]
    })";

    std::cout << "测试JSON: " << test_json << std::endl;

    // 检查通道
    if (test_json.find("\"channel\":\"tickers\"") != std::string::npos) {
        std::cout << "✓ 找到channel:tickers" << std::endl;
    } else {
        std::cout << "✗ 未找到channel:tickers" << std::endl;
    }

    // 检查data标记
    if (test_json.find("\"data\":[") != std::string::npos) {
        std::cout << "✓ 找到data数组" << std::endl;
    } else {
        std::cout << "✗ 未找到data数组" << std::endl;
    }

    auto result = JsonParser::parse_ticker_data(test_json);
    if (result) {
        std::cout << "解析成功，ticker数量: " << result->size() << std::endl;
        if (!result->empty()) {
            std::cout << "第一个ticker: " << (*result)[0].inst_id << std::endl;
        }
    } else {
        std::cout << "解析失败" << std::endl;
    }

    return 0;
}