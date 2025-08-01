#include "renderer.h"
#include "../resource/resource_manager.h"
#include "camera.h"
#include "sprite.h"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>
#include <stdexcept> // 用于 std::runtime_error

namespace engine::render {
    Renderer::Renderer(SDL_Renderer* sdl_renderer,
                       engine::resource::ResourceManager* resource_manager)
        : renderer_(sdl_renderer), resource_manager_(resource_manager)
    {
        spdlog::trace("Renderer 构造中...");

        if (!renderer_) {
            throw std::runtime_error("Renderer 构造失败: 提供的SDL_Renderer 为空指针");
        }
        if (!resource_manager_) {
            throw std::runtime_error("Renderer 构造失败: 提供的ResourceManager 为空指针");
        }

        setDrawColor(0, 0, 0, 255);
        spdlog::trace("Renderer 构造成功");
    }

    void Renderer::drawSprite(const Camera &camera, const Sprite &sprite, const glm::vec2 &position,
                              const glm::vec2 &scale, double angle)
    {
        // 早期验证 - 避免无效的缩放
        if (scale.x <= 0.0f || scale.y <= 0.0f) {
            return;
        }

        auto texture = resource_manager_->getTexture(sprite.getTextureId());
        if (!texture) {
            spdlog::error("ID: {} 纹理不存在", sprite.getTextureId());
            return;
        }

        auto src_rect = getSpriteSrcRect(sprite);
        if (!src_rect.has_value()) {
            spdlog::error("无法获取精灵的源矩形, ID: {}", sprite.getTextureId());
            return;
        }

        // 应用相机变换
        glm::vec2 screen_pos = camera.worldToScreen(position);

        // 计算目标矩形
        const float scaled_w = src_rect.value().w * scale.x;
        const float scaled_h = src_rect.value().h * scale.y;
        // 提前检查纹理是否太小以至于不可见
        if (scaled_w < 0.5f || scaled_h < 0.5f) {
            return;
        }
        SDL_FRect dst_rect = {screen_pos.x, screen_pos.y, scaled_w, scaled_h};

        // 检查精灵的目标矩形是否在视口内
        if (!isRectInViewport(camera, dst_rect)) return;

        // 执行绘制
        if (!SDL_RenderTextureRotated(renderer_, texture, &src_rect.value(), &dst_rect, angle, NULL,
                                      sprite.isFlipped() ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE)) {
            spdlog::error("渲染旋转纹理失败(ID: {}): {}", sprite.getTextureId(), SDL_GetError());
        }
    }

    void Renderer::drawParallax(const Camera &camera, const Sprite &sprite,
                                const glm::vec2 &position, const glm::vec2 &scroll_factor,
                                const glm::bvec2 &repeat, const glm::vec2 &scale)
    {
        // 早期验证 - 避免无效的缩放
        if (scale.x <= 0.0f || scale.y <= 0.0f) {
            return;
        }

        auto texture = resource_manager_->getTexture(sprite.getTextureId());
        if (!texture) {
            spdlog::error("ID: {} 纹理不存在", sprite.getTextureId());
            return;
        }

        auto src_rect = getSpriteSrcRect(sprite);
        if (!src_rect.has_value()) {
            spdlog::error("无法获取精灵的源矩形, ID: {}", sprite.getTextureId());
            return;
        }

        const glm::vec2 viewport_size = camera.getViewportSize();
        const glm::vec2 screen_pos = camera.worldToScreenWithParallax(position, scroll_factor);

        // 计算缩放后纹理尺寸 - 使用const避免重复计算
        const float scaled_w = src_rect.value().w * scale.x;
        const float scaled_h = src_rect.value().h * scale.y;

        // 提前检查纹理是否太小以至于不可见
        if (scaled_w < 0.5f || scaled_h < 0.5f) {
            return;
        }

        // 优化边界计算
        float start_x, stop_x, start_y, stop_y;

        if (repeat.x) {
            start_x = std::fmod(screen_pos.x, scaled_w);
            if (start_x > 0) start_x -= scaled_w; // 确保从负值开始
            stop_x = viewport_size.x;
        } else {
            start_x = screen_pos.x;
            stop_x = std::min(screen_pos.x + scaled_w, viewport_size.x);
            // 如果完全在视口外，直接返回
            if (start_x >= viewport_size.x || stop_x <= 0) {
                return;
            }
        }

        if (repeat.y) {
            start_y = std::fmod(screen_pos.y, scaled_h);
            if (start_y > 0) start_y -= scaled_h;
            stop_y = viewport_size.y;
        } else {
            start_y = screen_pos.y;
            stop_y = std::min(screen_pos.y + scaled_h, viewport_size.y);
            // 如果完全在视口外，直接返回
            if (start_y >= viewport_size.y || stop_y <= 0) {
                return;
            }
        }

        // 预计算循环次数，避免浮点数比较的精度问题
        const int x_count = static_cast<int>(std::ceil((stop_x - start_x) / scaled_w));
        const int y_count = static_cast<int>(std::ceil((stop_y - start_y) / scaled_h));

        // 避免过多的渲染调用
        if (x_count <= 0 || y_count <= 0) {
            return;
        }

        // 限制最大渲染数量，防止性能问题
        constexpr int MAX_TILES = 1000;
        if (x_count * y_count > MAX_TILES) {
            spdlog::warn("视差纹理tile数量过多: {}x{}, 限制渲染", x_count, y_count);
            return;
        }

        // 优化的渲染循环
        SDL_FRect dst_rect;
        dst_rect.w = scaled_w;
        dst_rect.h = scaled_h;

        float current_y = start_y;
        for (int y_idx = 0; y_idx < y_count; ++y_idx, current_y += scaled_h) {
            // 视锥剔除 - Y轴
            if (current_y + scaled_h <= 0 || current_y >= viewport_size.y) {
                continue;
            }

            dst_rect.y = current_y;
            float current_x = start_x;

            for (int x_idx = 0; x_idx < x_count; ++x_idx, current_x += scaled_w) {
                // 视锥剔除 - X轴
                if (current_x + scaled_w <= 0 || current_x >= viewport_size.x) {
                    continue;
                }

                dst_rect.x = current_x;

                if (!SDL_RenderTexture(renderer_, texture, nullptr, &dst_rect)) {
                    spdlog::error("渲染视差纹理失败(ID: {}):{}", sprite.getTextureId(),
                                  SDL_GetError());
                    return;
                }
            }
        }
    }

    void Renderer::drawUISprite(const Sprite &sprite, const glm::vec2 &position,
                                const std::optional<glm::vec2> &size)
    {
        auto texture = resource_manager_->getTexture(sprite.getTextureId());
        if (!texture) {
            spdlog::error("ID: {} 纹理不存在", sprite.getTextureId());
            return;
        }

        auto src_rect = getSpriteSrcRect(sprite);
        if (!src_rect.has_value()) {
            spdlog::error("无法获取精灵的源矩形, ID: {}", sprite.getTextureId());
            return;
        }

        SDL_FRect dst_rect = {position.x, position.y, 0, 0};
        if (size.has_value()) { // 如果提供尺寸，则绘制指定大小
            dst_rect.w = size.value().x;
            dst_rect.h = size.value().y;
        } else { // 否则绘制原始大小
            dst_rect.w = src_rect.value().w;
            dst_rect.h = src_rect.value().h;
        }

        // 执行绘制(未考虑UI旋转)
        if (!SDL_RenderTextureRotated(renderer_, texture, &src_rect.value(), &dst_rect, 0.0,
                                      nullptr,
                                      sprite.isFlipped() ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE)) {
            spdlog::error("渲染 UI Sprite 失败 (ID: {}): {}", sprite.getTextureId(),
                          SDL_GetError());
        }
    }

    void Renderer::drawUIFilledRect(const engine::utils::Rect &rect,
                                    const engine::utils::FColor &color)
    {
        setDrawColorFloat(color.r, color.g, color.b, color.a);
        SDL_FRect sdl_rect = {rect.position.x, rect.position.y, rect.size.x, rect.size.y};
        if (!SDL_RenderFillRect(renderer_, &sdl_rect)) {
            spdlog::error("绘制填充矩形失败：{}", SDL_GetError());
        }
        setDrawColor(0, 0, 0, 255);
    }

    void Renderer::present()
    {
        SDL_RenderPresent(renderer_);
    }

    void Renderer::clearScreen()
    {
        if (!SDL_RenderClear(renderer_)) {
            spdlog::error("清除渲染器失败：{}", SDL_GetError());
        }
    }

    void Renderer::setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
    {
        if (!SDL_SetRenderDrawColor(renderer_, r, g, b, a)) {
            spdlog::error("设置渲染器颜色失败：{}", SDL_GetError());
        }
    }

    void Renderer::setDrawColorFloat(float r, float g, float b, float a)
    {
        if (!SDL_SetRenderDrawColorFloat(renderer_, r, g, b, a)) {
            spdlog::error("设置渲染器颜色失败（浮点型）：{}", SDL_GetError());
        }
    }

    std::optional<SDL_FRect> Renderer::getSpriteSrcRect(const Sprite &sprite)
    {
        auto texture = resource_manager_->getTexture(sprite.getTextureId());
        if (!texture) {
            spdlog::error("ID: {} 纹理不存在", sprite.getTextureId());
            return std::nullopt;
        }

        auto src_rect = sprite.getSrcRect();
        if (!src_rect.has_value()) { // 如果Sprite中没有源矩形，则获取纹理尺寸
            SDL_FRect result = {0, 0, 0, 0};
            if (!SDL_GetTextureSize(texture, &result.w, &result.h)) {
                spdlog::error("获取纹理尺寸失败, ID: {}", sprite.getTextureId());
                return std::nullopt;
            }
            return result;
        }

        // 如果Sprite中有源矩形，则检查尺寸是否有效
        if (src_rect.value().w <= 0 || src_rect.value().h <= 0) {
            spdlog::error("精灵的源矩形无效, ID: {}", sprite.getTextureId());
            return std::nullopt;
        }

        return src_rect;
    }

    bool Renderer::isRectInViewport(const Camera &camera, const SDL_FRect &rect)
    {
        glm::vec2 viewport_size = camera.getViewportSize();
        return rect.x + rect.w >= 0 && rect.x <= viewport_size.x && rect.y + rect.h >= 0 &&
               rect.y <= viewport_size.y;
    }
} // namespace engine::render
