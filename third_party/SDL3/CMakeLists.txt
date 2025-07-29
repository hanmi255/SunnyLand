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

# 包含SDL3扩展库的配置
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# 查找并链接SDL3_image
if(WIN32)
    find_package(SDL3_image REQUIRED CONFIG PATHS "${CMAKE_CURRENT_SOURCE_DIR}/cmake" NO_DEFAULT_PATH)
    target_link_libraries(SDL3 INTERFACE SDL3_image::SDL3_image-shared)
endif()

# 查找并链接SDL3_mixer
if(WIN32)
    find_package(SDL3_mixer REQUIRED CONFIG PATHS "${CMAKE_CURRENT_SOURCE_DIR}/cmake" NO_DEFAULT_PATH)
    target_link_libraries(SDL3 INTERFACE SDL3_mixer::SDL3_mixer-shared)
endif()

# 查找并链接SDL3_ttf
if(WIN32)
    find_package(SDL3_ttf REQUIRED CONFIG PATHS "${CMAKE_CURRENT_SOURCE_DIR}/cmake" NO_DEFAULT_PATH)
    target_link_libraries(SDL3 INTERFACE SDL3_ttf::SDL3_ttf-shared)
endif()