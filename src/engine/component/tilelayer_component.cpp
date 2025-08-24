#include "tilelayer_component.h"
#include "../core/context.h"
#include "../object/game_object.h"
#include "../physics/physics_engine.h"
#include "../render/camera.h"
#include "../render/renderer.h"
#include <spdlog/spdlog.h>

namespace engine::component {
    TileLayerComponent::TileLayerComponent(glm::ivec2 tile_size, glm::ivec2 map_size,
                                           std::vector<TileInfo> &&tiles)
        : tile_size_(tile_size), map_size_(map_size), tiles_(std::move(tiles))
    {
        if (tiles_.size() != static_cast<size_t>(map_size_.x * map_size_.y)) {
            spdlog::error(
                "TileLayerComponent: 地图尺寸与提供的瓦片向量大小不匹配。瓦片数据将被清除。");
            tiles_.clear();
            map_size_ = {0, 0};
        }
        spdlog::trace("TileLayerComponent 构造完成");
    }

    void TileLayerComponent::init()
    {
        if (owner_ == nullptr) {
            spdlog::error("ParallaxComponent 在初始化前未设置 owner_。");
            return;
        }
        spdlog::trace("TileLayerComponent 初始化完成");
    }

    void TileLayerComponent::render(engine::core::Context &context)
    {
        // 早期退出检查
        if (tile_size_.x <= 0 || tile_size_.y <= 0 || tiles_.empty()) {
            return;
        }

        auto &renderer = context.getRenderer();
        auto &camera = context.getCamera();

        // 预计算常用值
        const auto tile_width = static_cast<float>(tile_size_.x);
        const auto tile_height = static_cast<float>(tile_size_.y);
        const int map_width = map_size_.x;

        // 视锥剔除：计算需要渲染的瓦片范围
        const auto camera_bounds_opt = camera.getLimitBounds(); // 返回std::optional<Rect>

        // 确定渲染边界：如果相机有限制边界则使用它，否则使用相机视口
        float left, right, top, bottom;

        if (camera_bounds_opt.has_value()) {
            const auto &camera_bounds = camera_bounds_opt.value();
            left = camera_bounds.position.x;
            right = camera_bounds.position.x + camera_bounds.size.x;
            top = camera_bounds.position.y;
            bottom = camera_bounds.position.y + camera_bounds.size.y;
        } else {
            // 使用相机位置和视口大小作为渲染边界
            const glm::vec2 camera_pos = camera.getPosition();
            const glm::vec2 viewport_size = camera.getViewportSize();
            left = camera_pos.x;
            right = camera_pos.x + viewport_size.x;
            top = camera_pos.y;
            bottom = camera_pos.y + viewport_size.y;
        }

        // 计算可见瓦片的边界（添加一点缓冲以避免边缘问题）
        const int start_x = std::max(0, static_cast<int>((left - offset_.x) / tile_width) - 1);
        const int end_x =
            std::min(map_width, static_cast<int>((right - offset_.x) / tile_width) + 2);
        const int start_y = std::max(0, static_cast<int>((top - offset_.y) / tile_height) - 1);
        const int end_y =
            std::min(map_size_.y, static_cast<int>((bottom - offset_.y) / tile_height) + 2);

        // 只渲染可见区域内的瓦片
        for (int y = start_y; y < end_y; ++y) {
            // 预计算y相关的值
            const size_t row_offset = static_cast<size_t>(y) * map_width;
            const float base_y = offset_.y + (y * tile_height);

            for (int x = start_x; x < end_x; ++x) {
                const size_t index = row_offset + x;

                // 检查索引有效性以及瓦片是否需要渲染
                if (index >= tiles_.size() || tiles_[index].type == TileType::EMPTY) {
                    continue;
                }

                const auto &tile_info = tiles_[index];

                // 计算瓦片位置
                glm::vec2 tile_pos = {offset_.x + (x * tile_width), base_y};

                // 处理图片尺寸与瓦片尺寸不匹配的情况
                const auto src_rect = tile_info.sprite.getSrcRect();
                if (src_rect && static_cast<float>(src_rect->h) != tile_height) {
                    tile_pos.y -= (static_cast<float>(src_rect->h) - tile_height);
                }

                // 执行绘制
                renderer.drawSprite(camera, tile_info.sprite, tile_pos);
            }
        }
    }

    void TileLayerComponent::clean()
    {
        if (physics_engine_ != nullptr) {
            physics_engine_->unregisterCollisionTileLayer(this);
        }
    }

    const TileInfo* TileLayerComponent::getTileInfoAt(glm::ivec2 pos) const
    {
        if (pos.x < 0 || pos.x >= map_size_.x || pos.y < 0 || pos.y >= map_size_.y) {
            spdlog::warn("TileLayerComponent: 瓦片坐标越界: ({}, {})", pos.x, pos.y);
            return nullptr;
        }
        auto index = static_cast<size_t>((pos.y * map_size_.x) + pos.x);
        // 瓦片索引不能越界
        if (index < tiles_.size()) {
            return &tiles_[index];
        }
        spdlog::warn("TileLayerComponent: 瓦片索引越界: {}", index);
        return nullptr;
    }

    TileType TileLayerComponent::getTileTypeAt(glm::ivec2 pos) const
    {
        const TileInfo* info = getTileInfoAt(pos);
        return (info != nullptr) ? info->type : TileType::EMPTY;
    }

    TileType TileLayerComponent::getTileTypeAtWorldPos(const glm::vec2 &world_pos) const
    {
        glm::vec2 relative_pos = world_pos - offset_;

        int tile_x = static_cast<int>(std::floor(relative_pos.x / tile_size_.x));
        int tile_y = static_cast<int>(std::floor(relative_pos.y / tile_size_.y));

        return getTileTypeAt(glm::ivec2{tile_x, tile_y});
    }

} // namespace engine::component
