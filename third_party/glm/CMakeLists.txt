cmake_minimum_required(VERSION 3.10.0)

# Export GLM library
add_library(glm INTERFACE)

# 设置GLM的包含目录
target_include_directories(glm INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# GLM是一个header-only库，不需要链接库文件