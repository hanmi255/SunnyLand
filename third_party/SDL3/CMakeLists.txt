cmake_minimum_required ( VERSION 3.10.0 )

# 设置策略
if ( POLICY CMP0111 )
    cmake_policy ( SET CMP0111 NEW )
endif ()

# Export SDL3 library
add_library ( SDL3 INTERFACE )

# 创建 SDL3::Headers 目标（SDL3_mixer 等扩展库需要）
add_library ( SDL3::Headers INTERFACE IMPORTED )
set_target_properties ( SDL3::Headers PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# 设置SDL3的包含目录
target_include_directories ( SDL3 INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Windows 平台配置
if(WIN32)
    # 查找正确的库文件名
    find_library(SDL3_LIB 
        NAMES SDL3 SDL3.dll
        PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib/x64
        NO_DEFAULT_PATH
    )
    
    if(NOT SDL3_LIB)
        message(FATAL_ERROR "找不到 SDL3 库文件在 ${CMAKE_CURRENT_SOURCE_DIR}/lib/x64")
    endif()
    
    # 链接找到的库
    target_link_libraries(SDL3 INTERFACE ${SDL3_LIB})
    
    # 如果使用 MSVC，可能还需要一些系统库
    if(MSVC)
        target_link_libraries(SDL3 INTERFACE
            winmm
            imm32
            version
            setupapi
        )
    endif()
    
    # 复制 DLL 到输出目录
    file(GLOB SDL3_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/lib/x64/*.dll")
    foreach(DLL ${SDL3_DLLS})
        configure_file(${DLL} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG} COPYONLY)
        configure_file(${DLL} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE} COPYONLY)
    endforeach()
endif()

# 包含SDL3扩展库的配置
list ( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )

# 查找并链接SDL3_image
if ( WIN32 )
    find_package ( SDL3_image REQUIRED CONFIG PATHS "${CMAKE_CURRENT_SOURCE_DIR}/cmake" NO_DEFAULT_PATH )

    if ( TARGET SDL3_image::SDL3_image-shared )
        target_link_libraries ( SDL3 INTERFACE SDL3_image::SDL3_image-shared )
    endif ()
endif ()

# 查找并链接SDL3_mixer
if ( WIN32 )
    find_package ( SDL3_mixer REQUIRED CONFIG PATHS "${CMAKE_CURRENT_SOURCE_DIR}/cmake" NO_DEFAULT_PATH )

    if ( TARGET SDL3_mixer::SDL3_mixer-shared )
        target_link_libraries ( SDL3 INTERFACE SDL3_mixer::SDL3_mixer-shared )
    endif ()
endif ()

# 查找并链接SDL3_ttf
if ( WIN32 )
    find_package ( SDL3_ttf REQUIRED CONFIG PATHS "${CMAKE_CURRENT_SOURCE_DIR}/cmake" NO_DEFAULT_PATH )

    if ( TARGET SDL3_ttf::SDL3_ttf-shared )
        target_link_libraries ( SDL3 INTERFACE SDL3_ttf::SDL3_ttf-shared )
    endif ()
endif ()