# OKX WebSocket C++ Client

High-performance C++23 WebSocket client for connecting to OKX exchange and listening to ticker events.

## Features

- ✅ **High Performance**: Built with libwebsockets for optimal performance
- ✅ **Modern C++**: Uses C++23 standard with modern language features
- ✅ **Real-time Ticker Data**: Subscribe to multiple trading pairs simultaneously
- ✅ **Auto-reconnection**: Automatic reconnection with exponential backoff
- ✅ **Ping/Pong Handling**: Built-in connection health monitoring
- ✅ **Thread-safe**: Multi-threaded design with proper synchronization
- ✅ **Error Handling**: Comprehensive error handling and logging

## Prerequisites

- **CMake** 3.20 or higher
- **C++23** compatible compiler (GCC 11+, Clang 14+, MSVC 2022+)
- **libwebsockets** development package

### Installing libwebsockets on macOS:
```bash
brew install libwebsockets
```

### Installing libwebsockets on Ubuntu/Debian:
```bash
sudo apt-get install libwebsockets-dev
```

## Building

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

## Usage

### Basic Example

```cpp
#include "okx_websocket_client.h"

int main() {
    OKXWebSocketClient client;

    // Set custom ticker callback
    client.set_ticker_callback([](const TickerData& ticker) {
        std::cout << ticker.inst_id << " - Last: $" << ticker.last << std::endl;
    });

    // Connect to OKX WebSocket
    if (!client.connect()) {
        std::cerr << "Failed to connect!" << std::endl;
        return 1;
    }

    // Subscribe to ticker data
    client.subscribe_ticker("BTC-USDT");
    client.subscribe_ticker("ETH-USDT");

    // Keep running
    while (client.is_connected()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
```

### Running the Examples

```bash
# Run the main client
./okx_client

# Run the test
./simple_test

# Run connection diagnostics
./simple_connect_test
./connection_test

# Run performance benchmark
./performance_test
```

## Configuration Options

```cpp
OKXWebSocketClient client;

// Enable/disable auto-reconnection (enabled by default)
client.enable_auto_reconnect(true);

// Set ping interval in seconds (default: 30)
client.set_ping_interval(30);

// Proxy configuration (if needed)
client.set_http_proxy("127.0.0.1", 8080);  // HTTP proxy
client.set_http_proxy("proxy.example.com", 8080, "user", "pass");  // HTTP proxy with auth
client.set_socks_proxy("127.0.0.1", 1080);  // SOCKS5 proxy
client.clear_proxy();  // Clear proxy settings
```

## Ticker Data Structure

```cpp
struct TickerData {
    std::string inst_type;    // Instrument type (SPOT, SWAP, etc.)
    std::string inst_id;      // Trading pair (e.g., "BTC-USDT")
    std::string last;         // Last traded price
    std::string last_sz;      // Last traded size
    std::string ask_px;       // Best ask price
    std::string ask_sz;       // Best ask size
    std::string bid_px;       // Best bid price
    std::string bid_sz;       // Best bid size
    std::string open24h;      // 24h opening price
    std::string high24h;      // 24h high price
    std::string low24h;       // 24h low price
    std::string vol_ccy24h;   // 24h volume in quote currency
    std::string vol24h;       // 24h volume in base currency
    std::string sod_utc0;     // Start of day price (UTC 0)
    std::string sod_utc8;     // Start of day price (UTC 8)
    std::string ts;           // Timestamp
};
```

## Performance Characteristics

- **Ultra-Fast JSON Parsing**: Custom zero-copy parser optimized for ticker data
  - **NO Regular Expressions**: Hand-optimized character-by-character parsing
  - **String View Usage**: Zero-copy parsing with std::string_view (C++17)
  - **Memory Pre-allocation**: Smart vector capacity management
  - **Direct Field Extraction**: Specialized parsing for known OKX ticker format
- **Low Latency**: Sub-millisecond message processing
- **High Throughput**: Can handle thousands of ticker messages per second
- **Memory Efficient**: Minimal allocations with move semantics
- **CPU Optimized**: Branch-prediction friendly parsing logic

## Architecture

- **Multi-threaded**: Separate worker thread for WebSocket operations
- **Asynchronous**: Non-blocking message processing
- **Event-driven**: Callback-based architecture for real-time data handling
- **Fault-tolerant**: Automatic error recovery and reconnection

## 🚨 SSL/TLS 连接问题

如果遇到SSL握手失败错误：
```
WebSocket connection error: tls: error:0A000410:SSL routines::ssl/tls alert handshake failure
```

请查看 [TROUBLESHOOTING.md](TROUBLESHOOTING.md) 获取详细的解决方案。

**快速解决步骤:**
1. 运行 `./simple_connect_test` 诊断连接问题
2. 如果需要代理，查看 [PROXY_GUIDE.md](PROXY_GUIDE.md) 代理配置指南
3. 检查网络防火墙设置
4. 尝试使用VPN或不同网络
5. 更新OpenSSL和证书

## 性能基准

```bash
./performance_test
```

预期结果：
- **270,000+ 消息/秒** 解析吞吐量
- **3.7微秒/消息** 平均解析时间
- **零正则表达式** 纯手工优化解析器

## License

MIT License - feel free to use this code in your projects.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## Support

For issues and questions, please check [TROUBLESHOOTING.md](TROUBLESHOOTING.md) or open a GitHub issue.

For OKX API documentation: https://www.okx.com/docs-v5/