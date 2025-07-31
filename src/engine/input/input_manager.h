/***
 * @Author: hanmi255 hanmi2550505@gmail.com
 * @Date: 2025-07-31 19:57:14
 * @LastEditTime: 2025-07-31 19:57:14
 * @LastEditors: hanmi255 hanmi2550505@gmail.com
 * @Description: 输入管理器 - 处理输入事件和动作状态管理
 * @FilePath: \SunnyLand\src\engine\input\input_manager.h
 * @技术宅拯救世界！！！
 */
#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_render.h>
#include <glm/vec2.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace engine::core {
    class Config;
} // namespace engine::core

namespace engine::input {

    /**
     * @brief 动作状态枚举
     */
    enum class ActionState {
        INACTIVE,      ///< @brief 动作未激活
        JUST_PRESSED,  ///< @brief 刚刚按下
        HELD_DOWN,     ///< @brief 持续按下
        JUST_RELEASED, ///< @brief 刚刚释放
    };

    /**
     * @brief 输入管理器类，负责处理输入事件和动作状态。
     *
     * 该类管理输入事件，将按键转换为动作状态，并提供查询动作状态的功能。
     * 它还处理鼠标位置的逻辑坐标转换。
     *
     * 特性:
     * - 支持键盘和鼠标输入映射
     * - 提供动作状态查询（按下、持续按下、释放）
     * - 支持屏幕坐标和逻辑坐标转换
     * - 高性能的事件处理和状态管理
     */
    class InputManager final {
    public:
        /**
         * @brief 构造函数
         * @param sdl_renderer 指向 SDL_Renderer 的指针，用于坐标转换
         * @param config 配置对象，包含输入映射信息
         * @throws std::runtime_error 如果任一指针为 nullptr
         */
        InputManager(SDL_Renderer* sdl_renderer, const engine::core::Config* config);

        // --- 主要更新函数 ---

        /**
         * @brief 更新输入状态
         *
         * 每帧调用一次，处理所有待处理的SDL事件并更新动作状态
         */
        void update();

        // --- 动作状态查询 ---

        /**
         * @brief 检查动作是否刚刚被按下
         * @param action_name 动作名称
         * @return true 如果动作在当前帧刚被按下
         */
        bool isActionJustPressed(const std::string &action_name) const;

        /**
         * @brief 检查动作是否被持续按下
         * @param action_name 动作名称
         * @return true 如果动作当前被按下（包括刚按下和持续按下）
         */
        bool isActionHeldDown(const std::string &action_name) const;

        /**
         * @brief 检查动作是否刚刚被释放
         * @param action_name 动作名称
         * @return true 如果动作在当前帧刚被释放
         */
        bool isActionJustReleased(const std::string &action_name) const;

        // --- 应用程序状态 ---

        /**
         * @brief 检查是否应该退出应用程序
         * @return true 如果接收到退出信号
         */
        bool shouldQuit() const noexcept;

        /**
         * @brief 设置退出标志
         * @param should_quit 是否应该退出
         */
        void setShouldQuit(bool should_quit) noexcept;

        // --- 鼠标位置获取 ---

        /**
         * @brief 获取屏幕坐标系下的鼠标位置
         * @return 鼠标在屏幕坐标系中的位置
         */
        glm::vec2 getScreenMousePosition() const noexcept;

        /**
         * @brief 获取逻辑坐标系下的鼠标位置
         * @return 鼠标在渲染器逻辑坐标系中的位置
         */
        glm::vec2 getLogicalMousePosition() const;

    private:
        // --- 核心处理函数 ---

        /**
         * @brief 更新所有动作的状态（JUST_PRESSED -> HELD_DOWN, JUST_RELEASED -> INACTIVE）
         */
        void updateActionStates();

        /**
         * @brief 处理所有待处理的SDL事件
         */
        void processAllEvents();

        /**
         * @brief 处理单个SDL事件
         * @param event SDL事件
         */
        void processEvent(const SDL_Event &event);

        // --- 具体事件处理器 ---

        /**
         * @brief 处理键盘事件
         * @param key_event 键盘事件数据
         */
        void handleKeyboardEvent(const SDL_KeyboardEvent &key_event);

        /**
         * @brief 处理鼠标按钮事件
         * @param button_event 鼠标按钮事件数据
         */
        void handleMouseButtonEvent(const SDL_MouseButtonEvent &button_event);

        /**
         * @brief 处理鼠标移动事件
         * @param motion_event 鼠标移动事件数据
         */
        void handleMouseMotionEvent(const SDL_MouseMotionEvent &motion_event);

        // --- 初始化和映射管理 ---

        /**
         * @brief 初始化输入映射
         * @param config 配置对象
         * @throws std::runtime_error 如果config为nullptr
         */
        void initializeMappings(const engine::core::Config* config);

        /**
         * @brief 添加默认的鼠标映射
         */
        void addDefaultMouseMappings();

        /**
         * @brief 添加默认映射（如果配置中不存在）
         * @param action 动作名称
         * @param key 按键名称
         */
        void addDefaultMapping(std::string_view action, std::string_view key);

        /**
         * @brief 构建输入映射表
         */
        void buildInputMappings();

        /**
         * @brief 添加单个输入映射
         * @param action_name 动作名称
         * @param key_name 按键名称
         */
        void addInputMapping(const std::string &action_name, const std::string &key_name);

        // --- 工具函数 ---

        /**
         * @brief 更新指定动作的状态
         * @param action_name 动作名称
         * @param is_input_active 输入是否激活
         * @param is_repeat_event 是否为重复事件
         */
        void updateActionState(const std::string &action_name, bool is_input_active,
                               bool is_repeat_event);

        /**
         * @brief 从字符串获取SDL扫描码
         * @param key_name 按键名称
         * @return 对应的SDL扫描码，如果无效则返回SDL_SCANCODE_UNKNOWN
         */
        static SDL_Scancode scancodeFromString(const std::string &key_name) noexcept;

        /**
         * @brief 从字符串获取鼠标按钮ID
         * @param button_name 鼠标按钮名称
         * @return 对应的SDL鼠标按钮ID，如果无效则返回0
         */
        static Uint32 mouseButtonFromString(std::string_view button_name) noexcept;

        /**
         * @brief 更新当前鼠标位置
         */
        void updateMousePosition();

    private:
        // --- 成员变量 ---

        SDL_Renderer* sdl_renderer_; ///< @brief 用于获取逻辑坐标的 SDL_Renderer 指针
        bool should_quit_;           ///< @brief 退出标志
        glm::vec2 mouse_position_;   ///< @brief 当前鼠标位置（屏幕坐标）

        /// @brief 动作名称到按键名称列表的映射（从配置加载）
        std::unordered_map<std::string, std::vector<std::string>> actions_to_keyname_map_;

        /// @brief 输入（键盘扫描码或鼠标按钮）到关联动作名称列表的映射
        std::unordered_map<std::variant<SDL_Scancode, Uint32>, std::vector<std::string>>
            input_to_actions_map_;

        /// @brief 每个动作的当前状态
        std::unordered_map<std::string, ActionState> action_states_;
    };

} // namespace engine::input
