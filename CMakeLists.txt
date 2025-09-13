cmake_minimum_required(VERSION 3.20)
project(okx_websocket_client VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

pkg_check_modules(LIBWEBSOCKETS REQUIRED libwebsockets)

include_directories(${LIBWEBSOCKETS_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})
link_directories(${LIBWEBSOCKETS_LIBRARY_DIRS})

add_executable(okx_client
    src/main.cpp
    src/okx_websocket_client.cpp
    src/ticker_handler.cpp
    src/json_parser.cpp
)

target_link_libraries(okx_client
    ${LIBWEBSOCKETS_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Threads::Threads
)

target_compile_definitions(okx_client PRIVATE ${LIBWEBSOCKETS_CFLAGS_OTHER})

add_executable(simple_test
    tests/simple_test.cpp
    src/okx_websocket_client.cpp
    src/ticker_handler.cpp
    src/json_parser.cpp
)

target_link_libraries(simple_test
    ${LIBWEBSOCKETS_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Threads::Threads
)

target_compile_definitions(simple_test PRIVATE ${LIBWEBSOCKETS_CFLAGS_OTHER})

add_executable(performance_test
    tests/performance_test.cpp
    src/json_parser.cpp
)

target_link_libraries(performance_test
    Threads::Threads
)

add_executable(debug_test
    tests/debug_test.cpp
    src/json_parser.cpp
)

target_link_libraries(debug_test
    Threads::Threads
)

add_executable(connection_test
    tests/connection_test.cpp
    src/okx_websocket_client.cpp
    src/ticker_handler.cpp
    src/json_parser.cpp
)

target_link_libraries(connection_test
    ${LIBWEBSOCKETS_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Threads::Threads
)

target_compile_definitions(connection_test PRIVATE ${LIBWEBSOCKETS_CFLAGS_OTHER})

add_executable(simple_connect_test
    tests/simple_connect_test.cpp
    src/okx_websocket_client.cpp
    src/ticker_handler.cpp
    src/json_parser.cpp
)

target_link_libraries(simple_connect_test
    ${LIBWEBSOCKETS_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Threads::Threads
)

target_compile_definitions(simple_connect_test PRIVATE ${LIBWEBSOCKETS_CFLAGS_OTHER})

add_executable(proxy_test
    tests/proxy_test.cpp
    src/okx_websocket_client.cpp
    src/ticker_handler.cpp
    src/json_parser.cpp
)

target_link_libraries(proxy_test
    ${LIBWEBSOCKETS_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Threads::Threads
)

target_compile_definitions(proxy_test PRIVATE ${LIBWEBSOCKETS_CFLAGS_OTHER})

add_executable(ssl_debug_test
    tests/ssl_debug_test.cpp
    src/okx_websocket_client.cpp
    src/ticker_handler.cpp
    src/json_parser.cpp
)

target_link_libraries(ssl_debug_test
    ${LIBWEBSOCKETS_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Threads::Threads
)

target_compile_definitions(ssl_debug_test PRIVATE ${LIBWEBSOCKETS_CFLAGS_OTHER})