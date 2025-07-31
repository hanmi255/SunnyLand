/***
 * @Author: hanmi255 hanmi2550505@gmail.com
 * @Date: 2025-07-31 10:14:00
 * @LastEditTime: 2025-07-31 10:14:00
 * @LastEditors: hanmi255 hanmi2550505@gmail.com
 * @Description:
 * @FilePath: \SunnyLand\src\engine\core\config.h
 * @技术宅拯救世界！！！
 */
#pragma once
#include <nlohmann/json_fwd.hpp> // nlohmann_json 提供的前向声明
#include <string>
#include <unordered_map>
#include <vector>

namespace engine::core {

    /**
     * @brief 管理配置设置
     *
     * 提供配置项的默认值，支持从 JSON 文件中读取配置项
     * 如果加载失败或文件不存在，则使用默认值
     */
    class Config final {
    public:
        explicit Config(const std::string &file_path);

        // 禁用拷贝和移动语义
        Config(const Config &) = delete;
        Config &operator=(const Config &) = delete;
        Config(Config &&) = delete;
        Config &operator=(Config &&) = delete;

        bool loadFromFile(const std::string &file_path);
        [[nodiscard]] bool saveToFile(const std::string &file_path) const;

        // --- getters ---
        const std::string &getWindowTitle() const;
        const int getWindowWidth() const;
        const int getWindowHeight() const;
        const bool isVSyncEnabled() const;
        const int getTargetFps() const;
        const float getMusicVolume() const;
        const float getSoundVolume() const;

    private:
        void fromJson(const nlohmann::json &json);
        nlohmann::ordered_json toJson() const;

    private:
        // --- 默认配置项 ---
        // 窗口设置
        std::string window_title_ = "SunnyLand";
        int window_width_ = 1280;
        int window_height_ = 720;
        bool window_resizable_ = true;

        // 图形设置
        bool vsync_enabled_ = true;

        // 性能设置
        int target_fps_ = 144;

        // 音频设置
        float music_volume_ = 0.5f;
        float sound_volume_ = 0.5f;

        // 存储动作名称到 SDL Scancode 名称列表的映射
        std::unordered_map<std::string, std::vector<std::string>> input_mappings_ = {
            // 提供一些合理的默认值，以防配置文件加载失败或缺少此部分
            {"move_left", {"A", "Left"}}, {"move_right", {"D", "Right"}},
            {"move_up", {"W", "Up"}},     {"move_down", {"S", "Down"}},
            {"jump", {"J", "Space"}},     {"attack", {"K", "MouseLeft"}},
            {"pause", {"P", "Escape"}},
            // 可以继续添加更多默认动作
        };
    };

} // namespace engine::core
