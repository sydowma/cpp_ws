#include "json_parser.h"
#include <sstream>
#include <iostream>
#include <ctime>

std::optional<std::unordered_map<std::string, std::string>> JsonParser::parse_simple(const std::string& json) {
    std::unordered_map<std::string, std::string> result;

    const char* ptr = json.c_str();
    const char* end = ptr + json.length();

    while (ptr < end) {
        // Skip whitespace
        while (ptr < end && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')) {
            ++ptr;
        }

        if (ptr >= end || *ptr != '"') {
            ++ptr;
            continue;
        }

        // Parse key
        ++ptr; // Skip opening quote
        const char* key_start = ptr;
        while (ptr < end && *ptr != '"') {
            ++ptr;
        }
        if (ptr >= end) break;

        std::string key(key_start, ptr - key_start);
        ++ptr; // Skip closing quote

        // Skip to colon
        while (ptr < end && *ptr != ':') {
            ++ptr;
        }
        if (ptr >= end) break;
        ++ptr; // Skip colon

        // Skip whitespace
        while (ptr < end && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')) {
            ++ptr;
        }

        if (ptr >= end) break;

        std::string value;
        if (*ptr == '"') {
            // String value
            ++ptr; // Skip opening quote
            const char* value_start = ptr;
            while (ptr < end && *ptr != '"') {
                ++ptr;
            }
            if (ptr < end) {
                value.assign(value_start, ptr - value_start);
                ++ptr; // Skip closing quote
            }
        } else {
            // Number or other value
            const char* value_start = ptr;
            while (ptr < end && *ptr != ',' && *ptr != '}' && *ptr != ']' &&
                   *ptr != ' ' && *ptr != '\t' && *ptr != '\n' && *ptr != '\r') {
                ++ptr;
            }
            value.assign(value_start, ptr - value_start);
        }

        result[key] = value;
    }

    return result.empty() ? std::nullopt : std::make_optional(result);
}

std::optional<std::vector<TickerData>> JsonParser::parse_ticker_data(const std::string& json) {
    // 高性能ticker解析 - 专门针对OKX ticker消息优化

    // 快速通道检查（更灵活的匹配）
    if (json.find("\"channel\"") == std::string::npos || json.find("\"tickers\"") == std::string::npos) {
        return std::nullopt;
    }

    // 查找data数组的开始和结束位置（更灵活的匹配）
    size_t data_pos = json.find("\"data\"");
    if (data_pos == std::string::npos) {
        return std::nullopt;
    }

    // 从data字段位置开始查找数组开始符号
    size_t data_start = json.find('[', data_pos);
    if (data_start == std::string::npos) {
        return std::nullopt;
    }

    data_start += 1; // 跳过 '['

    // 找到匹配的 ']'
    size_t data_end = data_start;
    int bracket_depth = 1;

    while (data_end < json.length() && bracket_depth > 0) {
        char c = json[data_end];
        if (c == '[') bracket_depth++;
        else if (c == ']') bracket_depth--;
        data_end++;
    }

    if (bracket_depth != 0) {
        return std::nullopt;
    }

    // 预分配向量空间（假设最多几个ticker）
    std::vector<TickerData> tickers;
    tickers.reserve(4);

    // 高效解析ticker对象
    const char* ptr = json.c_str() + data_start;
    const char* end_ptr = json.c_str() + data_end - 1;

    while (ptr < end_ptr) {
        // 跳过分隔符和空白
        while (ptr < end_ptr && (*ptr == ',' || *ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')) {
            ++ptr;
        }

        if (ptr >= end_ptr || *ptr != '{') {
            break;
        }

        // 找到对象的结束位置
        const char* obj_start = ptr;
        int brace_depth = 1;
        ++ptr; // 跳过 '{'

        while (ptr < end_ptr && brace_depth > 0) {
            if (*ptr == '{') brace_depth++;
            else if (*ptr == '}') brace_depth--;
            ++ptr;
        }

        if (brace_depth != 0) {
            break;
        }

        // 使用string_view避免复制，直接解析
        std::string_view ticker_json(obj_start, ptr - obj_start);
        TickerData ticker;

        if (parse_ticker_object(ticker_json, ticker)) {
            tickers.emplace_back(std::move(ticker));
        }
    }

    return tickers.empty() ? std::nullopt : std::make_optional(std::move(tickers));
}

std::string JsonParser::create_subscription_message(const std::string& channel, const std::string& inst_id) {
    std::ostringstream oss;
    oss << "{"
        << "\"id\":\"" << std::time(nullptr) << "\","
        << "\"op\":\"subscribe\","
        << "\"args\":[{\"channel\":\"" << channel << "\","
        << "\"instId\":\"" << inst_id << "\"}]"
        << "}";
    return oss.str();
}

void JsonParser::skip_whitespace(const char*& ptr, const char* end) {
    while (ptr < end && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')) {
        ++ptr;
    }
}

std::string_view JsonParser::extract_string_value(std::string_view json, std::string_view key) {
    // 构建搜索模式: "key":
    std::string search_pattern = "\"";
    search_pattern += key;
    search_pattern += "\":";

    size_t pos = json.find(search_pattern);
    if (pos == std::string_view::npos) {
        return {};
    }

    pos += search_pattern.length();

    // 跳过空白字符
    while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r')) {
        pos++;
    }

    if (pos >= json.length()) {
        return {};
    }

    size_t end_pos;
    if (json[pos] == '"') {
        // 字符串值
        pos++; // 跳过开始的引号
        end_pos = json.find('"', pos);
        if (end_pos == std::string_view::npos) {
            return {};
        }
    } else {
        // 数字或其他非字符串值
        end_pos = pos;
        while (end_pos < json.length() &&
               json[end_pos] != ',' &&
               json[end_pos] != '}' &&
               json[end_pos] != ']' &&
               json[end_pos] != ' ' &&
               json[end_pos] != '\t' &&
               json[end_pos] != '\n' &&
               json[end_pos] != '\r') {
            end_pos++;
        }
    }

    return json.substr(pos, end_pos - pos);
}

bool JsonParser::parse_ticker_object(std::string_view json, TickerData& ticker) {
    // 直接从字符串视图中提取各个字段，零拷贝
    auto extract_and_assign = [&](const char* key, std::string& target) {
        auto value = extract_string_value(json, key);
        target.assign(value.data(), value.size());
    };

    extract_and_assign("instType", ticker.inst_type);
    extract_and_assign("instId", ticker.inst_id);
    extract_and_assign("last", ticker.last);
    extract_and_assign("lastSz", ticker.last_sz);
    extract_and_assign("askPx", ticker.ask_px);
    extract_and_assign("askSz", ticker.ask_sz);
    extract_and_assign("bidPx", ticker.bid_px);
    extract_and_assign("bidSz", ticker.bid_sz);
    extract_and_assign("open24h", ticker.open24h);
    extract_and_assign("high24h", ticker.high24h);
    extract_and_assign("low24h", ticker.low24h);
    extract_and_assign("volCcy24h", ticker.vol_ccy24h);
    extract_and_assign("vol24h", ticker.vol24h);
    extract_and_assign("sodUtc0", ticker.sod_utc0);
    extract_and_assign("sodUtc8", ticker.sod_utc8);
    extract_and_assign("ts", ticker.ts);

    return !ticker.inst_id.empty();
}