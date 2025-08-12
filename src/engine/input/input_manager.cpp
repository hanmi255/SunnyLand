#include "input_manager.h"
#include "../core/config.h"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace engine::input {

    // 预定义常用动作名称，减少字符串分配
    namespace Actions {
        constexpr std::string_view MOUSE_LEFT_CLICK = "MouseLeftClick";
        constexpr std::string_view MOUSE_RIGHT_CLICK = "MouseRightClick";
    } // namespace Actions

    namespace KeyNames {
        constexpr std::string_view MOUSE_LEFT = "MouseLeft";
        constexpr std::string_view MOUSE_MIDDLE = "MouseMiddle";
        constexpr std::string_view MOUSE_RIGHT = "MouseRight";
        constexpr std::string_view MOUSE_X1 = "MouseX1";
        constexpr std::string_view MOUSE_X2 = "MouseX2";
    } // namespace KeyNames

    InputManager::InputManager(SDL_Renderer* sdl_renderer, const engine::core::Config* config)
        : sdl_renderer_(sdl_renderer), should_quit_(false)
    {
        if (!sdl_renderer_) {
            spdlog::error("输入管理器: SDL_Renderer 为空指针");
            throw std::runtime_error("输入管理器: SDL_Renderer 为空指针");
        }

        initializeMappings(config);
        updateMousePosition();

        spdlog::trace("初始鼠标位置: ({}, {})", mouse_position_.x, mouse_position_.y);
    }

    // --- 更新和事件处理 ---

    void InputManager::update()
    {
        // 优化：使用范围for循环，减少查找开销
        updateActionStates();
        processAllEvents();
    }

    void InputManager::updateActionStates()
    {
        for (auto &[action_name, state] : action_states_) {
            switch (state) {
                case ActionState::JUST_PRESSED:
                    state = ActionState::HELD_DOWN;
                    break;
                case ActionState::JUST_RELEASED:
                    state = ActionState::INACTIVE;
                    break;
                default:
                    break; // HELD_DOWN 和 INACTIVE 保持不变
            }
        }
    }

    void InputManager::processAllEvents()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            processEvent(event);
        }
    }

    void InputManager::processEvent(const SDL_Event &event)
    {
        switch (event.type) {
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                handleKeyboardEvent(event.key);
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                handleMouseButtonEvent(event.button);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                handleMouseMotionEvent(event.motion);
                break;
            case SDL_EVENT_QUIT:
                should_quit_ = true;
                break;
            default:
                break;
        }
    }

    void InputManager::handleKeyboardEvent(const SDL_KeyboardEvent &key_event)
    {
        const SDL_Scancode scancode = key_event.scancode;
        const bool is_down = key_event.down;
        const bool is_repeat = key_event.repeat;

        // 使用 find 一次查找，避免多次查找
        if (const auto it = input_to_actions_map_.find(scancode);
            it != input_to_actions_map_.end()) {

            for (const auto &action_name : it->second) {
                updateActionState(action_name, is_down, is_repeat);
            }
        }
    }

    void InputManager::handleMouseButtonEvent(const SDL_MouseButtonEvent &button_event)
    {
        const Uint32 button = button_event.button;
        const bool is_down = button_event.down;

        // 更新鼠标位置
        mouse_position_ = {button_event.x, button_event.y};

        if (const auto it = input_to_actions_map_.find(button); it != input_to_actions_map_.end()) {

            for (const auto &action_name : it->second) {
                updateActionState(action_name, is_down, false); // 鼠标事件无重复
            }
        }
    }

    void InputManager::handleMouseMotionEvent(const SDL_MouseMotionEvent &motion_event)
    {
        mouse_position_ = {motion_event.x, motion_event.y};
    }

    // --- 状态查询方法 ---

    bool InputManager::isActionHeldDown(std::string_view action_name) const
    {
        if (const auto it = action_states_.find(std::string(action_name));
            it != action_states_.end()) {
            const ActionState state = it->second;
            return state == ActionState::JUST_PRESSED || state == ActionState::HELD_DOWN;
        }
        return false;
    }

    bool InputManager::isActionJustPressed(std::string_view action_name) const
    {
        if (const auto it = action_states_.find(std::string(action_name));
            it != action_states_.end()) {
            return it->second == ActionState::JUST_PRESSED;
        }
        return false;
    }

    bool InputManager::isActionJustReleased(std::string_view action_name) const
    {
        if (const auto it = action_states_.find(std::string(action_name));
            it != action_states_.end()) {
            return it->second == ActionState::JUST_RELEASED;
        }
        return false;
    }

    bool InputManager::shouldQuit() const noexcept
    {
        return should_quit_;
    }

    void InputManager::setShouldQuit(bool should_quit) noexcept
    {
        should_quit_ = should_quit;
    }

    glm::vec2 InputManager::getScreenMousePosition() const noexcept
    {
        return mouse_position_;
    }

    glm::vec2 InputManager::getLogicalMousePosition() const
    {
        glm::vec2 logical_pos;
        SDL_RenderCoordinatesFromWindow(sdl_renderer_, mouse_position_.x, mouse_position_.y,
                                        &logical_pos.x, &logical_pos.y);
        return logical_pos;
    }

    // --- 初始化输入映射 ---

    void InputManager::initializeMappings(const engine::core::Config* config)
    {
        spdlog::trace("初始化输入映射...");

        if (!config) {
            spdlog::error("输入管理器: Config 为空指针");
            throw std::runtime_error("输入管理器: Config 为空指针");
        }

        actions_to_keyname_map_ = config->getInputMappings();

        // 清空并预留空间以提高性能
        input_to_actions_map_.clear();
        action_states_.clear();

        // 预留空间，假设有合理数量的动作
        action_states_.reserve(actions_to_keyname_map_.size());

        addDefaultMouseMappings();
        buildInputMappings();

        spdlog::trace("输入映射初始化完成.");
    }

    void InputManager::addDefaultMouseMappings()
    {
        // 使用预定义常量，减少字符串分配
        addDefaultMapping(Actions::MOUSE_LEFT_CLICK, KeyNames::MOUSE_LEFT);
        addDefaultMapping(Actions::MOUSE_RIGHT_CLICK, KeyNames::MOUSE_RIGHT);
    }

    void InputManager::addDefaultMapping(std::string_view action, std::string_view key)
    {
        if (actions_to_keyname_map_.find(std::string(action)) == actions_to_keyname_map_.end()) {
            spdlog::debug("配置中没有定义 '{}' 动作,添加默认映射到 '{}'.", action, key);
            actions_to_keyname_map_[std::string(action)] = {std::string(key)};
        }
    }

    void InputManager::buildInputMappings()
    {
        for (const auto &[action_name, key_names] : actions_to_keyname_map_) {
            // 初始化动作状态
            action_states_[action_name] = ActionState::INACTIVE;
            spdlog::trace("映射动作: {}", action_name);

            for (const auto &key_name : key_names) {
                addInputMapping(action_name, key_name);
            }
        }
    }

    void InputManager::addInputMapping(std::string_view action_name, std::string_view key_name)
    {
        // 尝试键盘映射
        if (const SDL_Scancode scancode = scancodeFromString(key_name);
            scancode != SDL_SCANCODE_UNKNOWN) {

            input_to_actions_map_[scancode].push_back(std::string(action_name));
            spdlog::trace("  映射按键: {} (Scancode: {}) 到动作: {}", key_name,
                          static_cast<int>(scancode), action_name);
            return;
        }

        // 尝试鼠标映射
        if (const Uint32 mouse_button = mouseButtonFromString(key_name); mouse_button != 0) {

            input_to_actions_map_[mouse_button].push_back(std::string(action_name));
            spdlog::trace("  映射鼠标按钮: {} (Button ID: {}) 到动作: {}", key_name,
                          static_cast<int>(mouse_button), action_name);
            return;
        }

        // 未知输入类型
        spdlog::warn("输入映射警告: 未知键或按钮名称 '{}' 用于动作 '{}'.", key_name, action_name);
    }

    // --- 工具函数 ---

    SDL_Scancode InputManager::scancodeFromString(std::string_view key_name) noexcept
    {
        return SDL_GetScancodeFromName(key_name.data());
    }

    Uint32 InputManager::mouseButtonFromString(std::string_view button_name) noexcept
    {
        // 使用 static 映射表提高查找性能
        static const std::unordered_map<std::string_view, Uint32> mouse_button_map = {
            {KeyNames::MOUSE_LEFT, SDL_BUTTON_LEFT},
            {KeyNames::MOUSE_MIDDLE, SDL_BUTTON_MIDDLE},
            {KeyNames::MOUSE_RIGHT, SDL_BUTTON_RIGHT},
            {KeyNames::MOUSE_X1, SDL_BUTTON_X1},
            {KeyNames::MOUSE_X2, SDL_BUTTON_X2}};

        if (const auto it = mouse_button_map.find(button_name); it != mouse_button_map.end()) {
            return it->second;
        }
        return 0; // 无效按钮
    }

    void InputManager::updateActionState(std::string_view action_name, bool is_input_active,
                                         bool is_repeat_event)
    {
        const auto it = action_states_.find(std::string(action_name));
        if (it == action_states_.end()) {
            spdlog::warn("尝试更新未注册的动作状态: {}", action_name);
            return;
        }

        if (is_input_active) {
            // 输入被激活 (按下)
            it->second = is_repeat_event ? ActionState::HELD_DOWN : ActionState::JUST_PRESSED;
        } else {
            // 输入被释放 (松开)
            it->second = ActionState::JUST_RELEASED;
        }
    }

    void InputManager::updateMousePosition()
    {
        float x, y;
        SDL_GetMouseState(&x, &y);
        mouse_position_ = {x, y};
    }

} // namespace engine::input
