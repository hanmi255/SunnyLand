cmake_minimum_required(VERSION 3.10.0)

# Export SDL2 library
add_library(SDL2 INTERFACE)

# 设置SDL2的包含目录
target_include_directories(SDL2 INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 设置SDL2的库文件路径 (Windows)
if(WIN32)
    target_link_directories(SDL2 INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/x64
    )
    
    # 链接SDL2库
    target_link_libraries(SDL2 INTERFACE
        SDL2
        SDL2main
    )
endif()

# 添加SDL2的定义
target_compile_definitions(SDL2 INTERFACE
    SDL_MAIN_HANDLED
)