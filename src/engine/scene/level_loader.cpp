#include "level_loader.h"
#include "../component/parallax_component.h"
#include "../component/sprite_component.h"
#include "../component/tilelayer_component.h"
#include "../component/transform_component.h"
#include "../core/context.h"
#include "../object/game_object.h"
#include "../render/sprite.h"
#include "../resource/resource_manager.h"
#include "../scene/scene.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <ranges> // 用于 std::views::transform
#include <spdlog/spdlog.h>
#include "../component/collider_component.h"
#include "../component/physics_component.h"

namespace engine::scene {

    bool LevelLoader::loadLevel(const std::string &level_path, Scene &scene)
    {
        // 清理之前的状态
        tileset_data_.clear();
        map_path_ = level_path;

        // 1. 加载并解析JSON文件
        auto json_data_opt = loadJsonFile(level_path);
        if (!json_data_opt) {
            return false;
        }
        const auto &json_data = *json_data_opt;

        // 2. 解析基本地图信息
        if (!parseMapBasicInfo(json_data)) {
            return false;
        }

        // 3. 验证地图数据
        if (!validateMapData()) {
            return false;
        }

        // 4. 加载tileset数据
        if (!loadAllTilesets(json_data)) {
            return false;
        }

        // 5. 加载所有图层
        if (!loadAllLayers(json_data, scene)) {
            return false;
        }

        spdlog::info("关卡加载完成: {} ({}x{} tiles, {} tilesets)", level_path, map_size_.x,
                     map_size_.y, tileset_data_.size());
        return true;
    }

    std::optional<nlohmann::json> LevelLoader::loadJsonFile(const std::string &file_path) const
    {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            spdlog::error("无法打开文件: {}", file_path);
            return std::nullopt;
        }

        nlohmann::json json_data;
        try {
            file >> json_data;
            return json_data;
        } catch (const nlohmann::json::parse_error &e) {
            spdlog::error("解析JSON文件 '{}' 失败: {} (at byte {})", file_path, e.what(), e.byte);
            return std::nullopt;
        }
    }

    bool LevelLoader::parseMapBasicInfo(const nlohmann::json &json_data)
    {
        map_size_ = glm::ivec2(getJsonValue(json_data, JsonKeys::WIDTH, 0),
                               getJsonValue(json_data, JsonKeys::HEIGHT, 0));

        tile_size_ = glm::ivec2(getJsonValue(json_data, JsonKeys::TILE_WIDTH, 0),
                                getJsonValue(json_data, JsonKeys::TILE_HEIGHT, 0));

        return true;
    }

    bool LevelLoader::validateMapData() const
    {
        if (map_size_.x <= 0 || map_size_.y <= 0) {
            spdlog::error("无效的地图尺寸: {}x{}", map_size_.x, map_size_.y);
            return false;
        }

        if (tile_size_.x <= 0 || tile_size_.y <= 0) {
            spdlog::error("无效的瓦片尺寸: {}x{}", tile_size_.x, tile_size_.y);
            return false;
        }

        return true;
    }

    bool LevelLoader::loadAllTilesets(const nlohmann::json &json_data)
    {
        if (!json_data.contains(JsonKeys::TILESETS) || !json_data[JsonKeys::TILESETS].is_array()) {
            // tilesets是可选的，没有也不报错
            return true;
        }

        const auto &tilesets = json_data[JsonKeys::TILESETS];

        for (const auto &tileset_json : tilesets) {
            if (!tileset_json.contains(JsonKeys::SOURCE) ||
                !tileset_json[JsonKeys::SOURCE].is_string() ||
                !tileset_json.contains(JsonKeys::FIRST_GID) ||
                !tileset_json[JsonKeys::FIRST_GID].is_number_integer()) {
                spdlog::error("tileset对象缺少有效的 'source' 或 'firstgid' 字段");
                continue;
            }

            const auto tileset_path = resolvePath(tileset_json[JsonKeys::SOURCE], map_path_);
            const auto first_gid = tileset_json[JsonKeys::FIRST_GID].get<int>();
            loadTileset(tileset_path, first_gid);
        }

        return true;
    }

    bool LevelLoader::loadAllLayers(const nlohmann::json &json_data, Scene &scene)
    {
        if (!validateLayerData(json_data, JsonKeys::LAYERS)) {
            return false;
        }

        const auto &layers = json_data[JsonKeys::LAYERS];
        for (const auto &layer_json : layers) {
            // 跳过不可见图层
            if (!getJsonValue(layer_json, JsonKeys::VISIBLE, true)) {
                spdlog::debug("图层 '{}' 不可见，跳过加载", getLayerName(layer_json));
                continue;
            }

            const auto layer_type = getJsonValue<std::string>(layer_json, JsonKeys::TYPE, "none");

            try {
                if (layer_type == "imagelayer") {
                    loadImageLayer(layer_json, scene);
                } else if (layer_type == "tilelayer") {
                    loadTileLayer(layer_json, scene);
                } else if (layer_type == "objectgroup") {
                    loadObjectLayer(layer_json, scene);
                } else {
                    spdlog::warn("不支持的图层类型: {}", layer_type);
                }
            } catch (const std::exception &e) {
                spdlog::error("加载图层 '{}' 时发生异常: {}", getLayerName(layer_json), e.what());
                continue;
            }
        }

        return true;
    }

    void LevelLoader::loadImageLayer(const nlohmann::json &layer_json, Scene &scene)
    {
        const auto image_path = getJsonValue<std::string>(layer_json, JsonKeys::IMAGE, "");
        if (image_path.empty()) {
            spdlog::error("图层 '{}' 缺少 'image' 属性", getLayerName(layer_json));
            return;
        }

        const auto texture_id = resolvePath(image_path, map_path_);
        const auto layer_name = getLayerName(layer_json);

        // 获取图层属性（使用结构化绑定会更清晰，但为了兼容性使用传统方式）
        const glm::vec2 offset(getJsonValue(layer_json, "offsetx", 0.0f),
                               getJsonValue(layer_json, "offsety", 0.0f));

        const glm::vec2 scroll_factor(getJsonValue(layer_json, "parallaxx", 1.0f),
                                      getJsonValue(layer_json, "parallaxy", 1.0f));

        const glm::bvec2 repeat(getJsonValue(layer_json, "repeatx", false),
                                getJsonValue(layer_json, "repeaty", false));

        // 创建游戏对象
        auto game_object = std::make_unique<engine::object::GameObject>(layer_name);
        game_object->addComponent<engine::component::TransformComponent>(offset);
        game_object->addComponent<engine::component::ParallaxComponent>(texture_id, scroll_factor,
                                                                        repeat);

        scene.addGameObject(std::move(game_object));
        spdlog::debug("加载图像图层: '{}' 完成", layer_name);
    }

    void LevelLoader::loadTileLayer(const nlohmann::json &layer_json, Scene &scene)
    {
        if (!validateLayerData(layer_json, JsonKeys::DATA)) {
            return;
        }

        const auto &data = layer_json[JsonKeys::DATA];
        const auto layer_name = getLayerName(layer_json);

        // 预分配瓦片信息向量空间
        std::vector<engine::component::TileInfo> tiles;
        const size_t total_tiles = static_cast<size_t>(map_size_.x) * map_size_.y;
        tiles.reserve(total_tiles);

        // 批量处理瓦片数据
        std::transform(data.begin(), data.end(), std::back_inserter(tiles),
                       [this](const auto &gid) { return getTileInfoByGid(gid.get<int>()); });

        // 创建游戏对象
        auto game_object = std::make_unique<engine::object::GameObject>(layer_name);
        game_object->addComponent<engine::component::TileLayerComponent>(tile_size_, map_size_,
                                                                         std::move(tiles));

        scene.addGameObject(std::move(game_object));
        spdlog::debug("加载瓦片图层: '{}' 完成 ({} 瓦片)", layer_name, total_tiles);
    }

    void LevelLoader::loadObjectLayer(const nlohmann::json &layer_json, Scene &scene)
    {
        if (!validateLayerData(layer_json, JsonKeys::OBJECTS)) {
            return;
        }

        const auto &objects = layer_json[JsonKeys::OBJECTS];
        size_t loaded_objects = 0;

        for (const auto &object : objects) {
            const auto gid = getJsonValue(object, "gid", INVALID_GID);

            if (gid == INVALID_GID) {
                // TODO: 处理自定义形状（碰撞盒、触发器等）
                continue;
            }

            auto tile_info = getTileInfoByGid(gid);
            if (tile_info.sprite.getTextureId().empty()) {
                spdlog::warn("gid为 {} 的瓦片没有图像纹理", gid);
                continue;
            }

            // 计算变换信息
            auto position =
                glm::vec2(getJsonValue(object, "x", 0.0f), getJsonValue(object, "y", 0.0f));

            const auto dst_size = glm::vec2(getJsonValue(object, "width", 0.0f),
                                            getJsonValue(object, "height", 0.0f));

            // 调整位置（从左下角到左上角）
            position.y -= dst_size.y;

            const auto rotation = getJsonValue(object, "rotation", 0.0f);

            const auto src_size_opt = tile_info.sprite.getSrcRect();
            if (!src_size_opt) {
                spdlog::warn("gid为 {} 的瓦片没有源矩形", gid);
                continue;
            }

            const auto src_size = glm::vec2(src_size_opt->w, src_size_opt->h);
            const auto scale = dst_size / src_size;
            const auto object_name =
                getJsonValue<std::string>(object, JsonKeys::NAME, std::string(DEFAULT_LAYER_NAME));

            // 创建游戏对象
            auto game_object = std::make_unique<engine::object::GameObject>(object_name);
            game_object->addComponent<engine::component::TransformComponent>(position, scale,
                                                                             rotation);
            game_object->addComponent<engine::component::SpriteComponent>(
                std::move(tile_info.sprite), scene.getContext().getResourceManager());

            scene.addGameObject(std::move(game_object));
            ++loaded_objects;
        }

        spdlog::debug("加载对象图层: '{}' 完成 ({} 对象)", getLayerName(layer_json),
                      loaded_objects);
    }

    engine::component::TileType LevelLoader::getTileType(const nlohmann::json &tile_json) const
    {
        if (tile_json.contains(JsonKeys::PROPERTIES)) {
            const auto &properties = tile_json[JsonKeys::PROPERTIES];
            if (properties.is_array()) {
                for (const auto &property : properties) {
                    const auto property_name = getJsonValue<std::string>(property, "name", "");
                    if (property_name == "solid") {
                        const auto is_solid = getJsonValue(property, "value", false);
                        return is_solid ? engine::component::TileType::SOLID
                                        : engine::component::TileType::NORMAL;
                    }
                    // TODO: 可以在这里添加更多的自定义属性处理逻辑
                }
            }
        }
        return engine::component::TileType::NORMAL;
    }

    engine::component::TileType LevelLoader::getTileTypeById(const nlohmann::json &tileset_json,
                                                             int local_id) const
    {
        if (tileset_json.contains(JsonKeys::TILES)) {
            const auto &tiles = tileset_json[JsonKeys::TILES];
            if (tiles.is_array()) {
                for (const auto &tile : tiles) {
                    const auto id = getJsonValue(tile, "id", -1);
                    if (id == local_id) {
                        return getTileType(tile);
                    }
                }
            }
        }
        return engine::component::TileType::NORMAL;
    }

    engine::component::TileInfo LevelLoader::getTileInfoByGid(int gid) const
    {
        if (gid == INVALID_GID) {
            return engine::component::TileInfo();
        }

        // upper_bound：查找tileset_data_中键大于 gid 的第一个元素，返回迭代器
        auto tileset_it = tileset_data_.upper_bound(gid);
        if (tileset_it == tileset_data_.begin()) {
            spdlog::error("gid为 {} 的瓦片未找到图块集。", gid);
            return engine::component::TileInfo();
        }
        --tileset_it; // 前移一个位置，这样就得到不大于gid的最近一个目标元素

        const auto &tileset = tileset_it->second;
        const auto local_id = gid - tileset_it->first;
        const auto file_path = getJsonValue<std::string>(tileset, "file_path", "");

        if (file_path.empty()) {
            spdlog::warn("图块集缺少文件路径信息");
            return engine::component::TileInfo();
        }

        try {
            if (tileset.contains(JsonKeys::IMAGE)) {
                // 单一图片模式
                const auto texture_id = resolvePath(tileset[JsonKeys::IMAGE], file_path);
                const auto columns = getJsonValue(tileset, "columns", 1);

                const auto coordinate_x = local_id % columns;
                const auto coordinate_y = local_id / columns;

                const SDL_FRect texture_rect = {static_cast<float>(coordinate_x * tile_size_.x),
                                                static_cast<float>(coordinate_y * tile_size_.y),
                                                static_cast<float>(tile_size_.x),
                                                static_cast<float>(tile_size_.y)};

                engine::render::Sprite sprite{texture_id, texture_rect};
                auto tile_type = getTileTypeById(tileset, local_id);
                return engine::component::TileInfo(std::move(sprite), tile_type);
            } else if (tileset.contains(JsonKeys::TILES)) {
                // 多图片模式
                const auto &tiles_json = tileset[JsonKeys::TILES];
                for (const auto &tile_json : tiles_json) {
                    if (getJsonValue(tile_json, "id", -1) == local_id) {
                        const auto image_path =
                            getJsonValue<std::string>(tile_json, JsonKeys::IMAGE, "");
                        if (image_path.empty()) {
                            break;
                        }

                        const auto texture_id = resolvePath(image_path, file_path);
                        const auto image_width = getJsonValue(tile_json, "imagewidth", 0);
                        const auto image_height = getJsonValue(tile_json, "imageheight", 0);

                        const SDL_FRect texture_rect = {
                            static_cast<float>(getJsonValue(tile_json, "x", 0)),
                            static_cast<float>(getJsonValue(tile_json, "y", 0)),
                            static_cast<float>(getJsonValue(tile_json, "width", image_width)),
                            static_cast<float>(getJsonValue(tile_json, "height", image_height))};

                        engine::render::Sprite sprite{texture_id, texture_rect};
                        auto tile_type = getTileType(tile_json);
                        return engine::component::TileInfo(std::move(sprite), tile_type);
                    }
                }
            }
        } catch (const std::exception &e) {
            spdlog::error("处理gid {} 时发生异常: {}", gid, e.what());
        }

        spdlog::warn("图块集中未找到gid为 {} 的瓦片", gid);
        return engine::component::TileInfo();
    }

    template <typename T>
    std::optional<T> LevelLoader::getTileProperty(const nlohmann::json &tile_json,
                                                  std::string_view property_name) const
    {
        if (!tile_json.contains(JsonKeys::PROPERTIES)) return std::nullopt;

        const auto &properties = tile_json[JsonKeys::PROPERTIES];
        for (const auto &property : properties) {
            if (getJsonValue<std::string>(property, "name", "") == property_name) {
                return getJsonValue<T>(property, "value", T{});
            }
        }
        return std::nullopt;
    }

    std::optional<engine::utils::Rect>
    LevelLoader::getColliderRect(const nlohmann::json &tile_json) const
    {
        if(!tile_json.contains(JsonKeys::OBJECT_GROUP)) return std::nullopt;
        const auto &object_group = tile_json[JsonKeys::OBJECT_GROUP];
        if(!object_group.contains(JsonKeys::OBJECTS)) return std::nullopt;
        const auto &objects = object_group[JsonKeys::OBJECTS];
        for (const auto &object : objects) {
            auto rect = engine::utils::Rect{glm::vec2(getJsonValue(object, "x", 0.0f),
                                                  getJsonValue(object, "y", 0.0f)),
                                        glm::vec2(getJsonValue(object, "width", 0.0f),
                                                  getJsonValue(object, "height", 0.0f))};
            if (rect.size.x > 0 && rect.size.y > 0) 
        return rect;
        }
        return std::nullopt;
    }

    void LevelLoader::loadTileset(const std::string &tileset_path, int first_gid)
    {
        auto tileset_json_opt = loadJsonFile(tileset_path);
        if (!tileset_json_opt) {
            return;
        }

        auto tileset_json = std::move(*tileset_json_opt);
        tileset_json["file_path"] = tileset_path;
        tileset_data_.emplace(first_gid, std::move(tileset_json));

        spdlog::debug("图块集文件 '{}' 加载完成，firstgid: {}", tileset_path, first_gid);
    }

    std::string LevelLoader::resolvePath(const std::string &relative_path,
                                         const std::string &file_path) const
    {
        try {
            const auto map_dir = std::filesystem::path(file_path).parent_path();
            const auto final_path = std::filesystem::canonical(map_dir / relative_path);
            return final_path.string();
        } catch (const std::exception &e) {
            spdlog::warn("解析路径失败，使用原始路径: {} (错误: {})", relative_path, e.what());
            return relative_path;
        }
    }

    bool LevelLoader::validateLayerData(const nlohmann::json &layer_json,
                                        std::string_view expected_key) const
    {
        if (!layer_json.contains(expected_key) || !layer_json[expected_key].is_array()) {
            spdlog::error("图层 '{}' 缺少或无效的 '{}' 属性", getLayerName(layer_json),
                          expected_key);
            return false;
        }
        return true;
    }

    template <typename T>
    T LevelLoader::getJsonValue(const nlohmann::json &json, std::string_view key,
                                T default_value) const
    {
        try {
            if (json.contains(key)) {
                return json[key].get<T>();
            }
        } catch (const std::exception &e) {
            spdlog::debug("获取JSON值 '{}' 失败，使用默认值: {}", key, e.what());
        }
        return default_value;
    }

    std::string LevelLoader::getLayerName(const nlohmann::json &layer_json) const
    {
        return getJsonValue<std::string>(layer_json, JsonKeys::NAME,
                                         std::string(DEFAULT_LAYER_NAME));
    }
} // namespace engine::scene
