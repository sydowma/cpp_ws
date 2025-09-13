#pragma once
#include "json_parser.h"
#include <functional>
#include <memory>

class TickerHandler {
public:
    using TickerCallback = std::function<void(const TickerData&)>;

    TickerHandler(TickerCallback callback);

    void handle_message(const std::string& message);
    void set_callback(TickerCallback callback);

private:
    TickerCallback callback_;
    void process_ticker_data(const std::vector<TickerData>& tickers);
};