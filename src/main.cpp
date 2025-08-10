/***
 * @Author: hanmi255 hanmi2550505@gmail.com
 * @Date: 2025-07-29 12:58:45
 * @LastEditTime: 2025-07-29 14:55:03
 * @LastEditors: hanmi255 hanmi2550505@gmail.com
 * @Description:
 * @FilePath: \SunnyLand\src\main.cpp
 * @技术宅拯救世界！！！
 */
#include "engine/core/game_app.h"
#include <SDL3/SDL_main.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

int main(int /* argc */, char* /* argv */[])
{
    // 确保日志目录存在
    std::filesystem::create_directories("logs");
    
    // 生成带时间戳的日志文件名
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm timeinfo;
    localtime_s(&timeinfo, &time_t);
    std::stringstream ss;
    ss << "logs/sunnyland_" << std::put_time(&timeinfo, "%Y%m%d_%H%M%S") << ".log";
    std::string log_filename = ss.str();
    
    // 创建控制台和文件输出
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_filename, true);
    
    // 创建日志记录器
    auto logger = std::make_shared<spdlog::logger>("sunnyland", spdlog::sinks_init_list{console_sink, file_sink});
    
    // 设置日志格式
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    logger->set_level(spdlog::level::trace);
    
    // 设置为默认日志记录器
    spdlog::set_default_logger(logger);

    engine::core::GameApp app;
    app.run();
    return 0;
}
