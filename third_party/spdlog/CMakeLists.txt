cmake_minimum_required(VERSION 3.10.0)

# Export spdlog library
add_library(spdlog INTERFACE)

# 设置spdlog的包含目录
target_include_directories(spdlog INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# spdlog是一个header-only库，不需要链接库文件