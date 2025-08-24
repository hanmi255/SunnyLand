
#include "ui_interactive.h"
#include "../audio/audio_player.h"
#include "../core/context.h"
#include "../render/renderer.h"
#include "../resource/resource_manager.h"
#include "state/ui_state.h"
#include <spdlog/spdlog.h>

namespace engine::ui {

    UIInteractive::~UIInteractive() = default;

    UIInteractive::UIInteractive(engine::core::Context &context, glm::vec2 position, glm::vec2 size)
        : UIElement(position, size), context_(context)
    {
        spdlog::trace("UIInteractive 构造完成");
    }

    void UIInteractive::setState(std::unique_ptr<engine::ui::state::UIState> state)
    {
        if (!state) {
            spdlog::warn("尝试设置空的状态！");
            return;
        }

        state_ = std::move(state);
        state_->enter();
    }

    void UIInteractive::addSprite(std::string_view name,
                                  std::unique_ptr<engine::render::Sprite> sprite)
    {
        // 可交互UI元素必须有一个 size 用于交互检测，因此如果参数列表中没有指定，则用图片大小作为
        // size
        if (size_.x == 0.0F && size_.y == 0.0F) {
            size_ = context_.getResourceManager().getTextureSize(sprite->getTextureId());
        }
        // 添加精灵
        sprites_[std::string(name)] = std::move(sprite);
    }

    void UIInteractive::setSprite(std::string_view name)
    {
        if (sprites_.contains(std::string(name))) {
            current_sprite_ = sprites_[std::string(name)].get();
        } else {
            spdlog::warn("Sprite '{}' 未找到", name);
        }
    }

    void UIInteractive::addSound(std::string_view name, std::string_view path)
    {
        sounds_[std::string(name)] = path;
    }

    void UIInteractive::playSound(std::string_view name)
    {
        if (sounds_.contains(std::string(name))) {
            context_.getAudioPlayer().playSound(sounds_[std::string(name)]);
        } else {
            spdlog::error("Sound '{}' 未找到", name);
        }
    }

    bool UIInteractive::handleInput(engine::core::Context &context)
    {
        if (UIElement::handleInput(context)) {
            return true;
        }

        // 使用if初始化语句检查状态和交互性，避免重复检查
        if (const bool can_handle_input = state_ && interactive_; !can_handle_input) {
            return false;
        }

        // 处理状态输入，如果状态改变则更新状态并返回true
        if (auto next_state = state_->handleInput(context); next_state) {
            setState(std::move(next_state));
            return true;
        }

        return false;
    }

    void UIInteractive::render(engine::core::Context &context)
    {
        if (!visible_) return;

        // 先渲染自身
        context.getRenderer().drawUISprite(*current_sprite_, getScreenPosition(), size_);

        // 再渲染子元素（调用基类方法）
        UIElement::render(context);
    }

} // namespace engine::ui