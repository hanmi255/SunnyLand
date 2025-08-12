#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

namespace game::data {

    /**
     * @brief 管理不同游戏场景之间的游戏状态
     *
     * 存储玩家生命值、分数、当前关卡等信息，
     * 使这些数据在场景切换时能够保持。
     */
    class SessionData final {
    private:
        int current_health_ = 3;
        int max_health_ = 3;
        int current_score_ = 0;
        int high_score_ = 0;
        bool is_win_ = false;

        int level_health_ = 3; ///< @brief 进入关卡时的生命值（读/存档用）
        int level_score_ = 0;  ///< @brief 进入关卡时的得分（读/存档用）
        std::string map_path_ = "assets/maps/level1.tmj";

    public:
        SessionData() = default;
        ~SessionData() = default;

        // 禁止拷贝和移动语义
        SessionData(const SessionData &) = delete;
        SessionData &operator=(const SessionData &) = delete;
        SessionData(SessionData &&) = delete;
        SessionData &operator=(SessionData &&) = delete;

        // --- getters ---
        int getCurrentHealth() const { return current_health_; }
        int getMaxHealth() const { return max_health_; }
        int getCurrentScore() const { return current_score_; }
        int getHighScore() const { return high_score_; }
        int getLevelHealth() const { return level_health_; }
        int getLevelScore() const { return level_score_; }
        std::string_view getMapPath() const { return map_path_; }
        bool getIsWin() const { return is_win_; }

        // --- setters ---
        void setCurrentHealth(int health);
        void setMaxHealth(int max_health);
        void addScore(int score_to_add);
        void setHighScore(int high_score) { high_score_ = high_score; }
        void setLevelHealth(int level_health) { level_health_ = level_health; }
        void setLevelScore(int level_score) { level_score_ = level_score; }
        void setMapPath(std::string_view map_path) { map_path_ = map_path; }
        void setIsWin(bool is_win) { is_win_ = is_win; }

        // 核心逻辑
        void reset();
        void setNextLevel(std::string_view map_path);
        bool saveToFile(std::string_view filename) const;
        bool loadFromFile(std::string_view filename);
        bool syncHighScore(std::string_view filename);
    };

} // namespace game::data