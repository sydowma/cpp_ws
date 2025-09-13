#include "ticker_handler.h"
#include <iostream>

TickerHandler::TickerHandler(TickerCallback callback) : callback_(std::move(callback)) {}

void TickerHandler::handle_message(const std::string& message) {
    auto ticker_data = JsonParser::parse_ticker_data(message);
    if (ticker_data) {
        process_ticker_data(*ticker_data);
    }
}

void TickerHandler::set_callback(TickerCallback callback) {
    callback_ = std::move(callback);
}

void TickerHandler::process_ticker_data(const std::vector<TickerData>& tickers) {
    if (!callback_) return;

    for (const auto& ticker : tickers) {
        callback_(ticker);
    }
}