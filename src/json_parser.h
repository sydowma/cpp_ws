#pragma once
#include <string>
#include <optional>
#include <unordered_map>
#include <vector>
#include <string_view>

struct TickerData {
    std::string inst_type;
    std::string inst_id;
    std::string last;
    std::string last_sz;
    std::string ask_px;
    std::string ask_sz;
    std::string bid_px;
    std::string bid_sz;
    std::string open24h;
    std::string high24h;
    std::string low24h;
    std::string vol_ccy24h;
    std::string vol24h;
    std::string sod_utc0;
    std::string sod_utc8;
    std::string ts;
};

class JsonParser {
public:
    static std::optional<std::unordered_map<std::string, std::string>> parse_simple(const std::string& json);
    static std::optional<std::vector<TickerData>> parse_ticker_data(const std::string& json);
    static std::string create_subscription_message(const std::string& channel, const std::string& inst_id);

private:
    static std::string_view extract_string_value(std::string_view json, std::string_view key);
    static bool parse_ticker_object(std::string_view json, TickerData& ticker);
    static void skip_whitespace(const char*& ptr, const char* end);
};