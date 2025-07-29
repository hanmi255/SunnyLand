cmake_minimum_required(VERSION 3.10.0)

# Export nlohmann_json library
add_library(nlohmann_json INTERFACE)

# 设置nlohmann_json的包含目录
target_include_directories(nlohmann_json INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# nlohmann_json是一个header-only库，不需要链接库文件