cmake_minimum_required(VERSION 3.10.0)

# Export SDL3 library
add_library(SDL3 INTERFACE)

# 设置SDL3的包含目录
target_include_directories(SDL3 INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 设置SDL3的库文件路径 (Windows)
if(WIN32)
    target_link_directories(SDL3 INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/x64
    )
    
    # 链接SDL3库
    target_link_libraries(SDL3 INTERFACE
        SDL3
    )
endif()

# 添加SDL3的定义
target_compile_definitions(SDL3 INTERFACE
    SDL_MAIN_USE_CALLBACKS
)