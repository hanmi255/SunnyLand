#pragma once
#include "../utils/math.h"
#include <glm/vec2.hpp>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>

namespace engine::component {
    class AnimationComponent;
    class AudioComponent;
    struct TileInfo;
    enum class TileType;
} // namespace engine::component

namespace engine::object {
    class GameObject;
} // namespace engine::object

namespace engine::scene {
    class Scene;

    /**
     * @brief 负责从 Tiled JSON 文件 (.tmj) 加载关卡数据到 Scene 中。
     */
    class LevelLoader final {
    private:
        // 常量定义
        static constexpr std::string_view DEFAULT_LAYER_NAME = "Unnamed";
        static constexpr int INVALID_GID = 0;

        // 预定义的JSON键名，避免重复字符串创建
        struct JsonKeys {
            static constexpr std::string_view WIDTH = "width";
            static constexpr std::string_view HEIGHT = "height";
            static constexpr std::string_view TILE_WIDTH = "tilewidth";
            static constexpr std::string_view TILE_HEIGHT = "tileheight";
            static constexpr std::string_view TILES = "tiles";
            static constexpr std::string_view TILESETS = "tilesets";
            static constexpr std::string_view LAYERS = "layers";
            static constexpr std::string_view SOURCE = "source";
            static constexpr std::string_view FIRST_GID = "firstgid";
            static constexpr std::string_view TYPE = "type";
            static constexpr std::string_view VISIBLE = "visible";
            static constexpr std::string_view NAME = "name";
            static constexpr std::string_view IMAGE = "image";
            static constexpr std::string_view DATA = "data";
            static constexpr std::string_view OBJECTS = "objects";
            static constexpr std::string_view OBJECT_GROUP = "objectgroup";
            static constexpr std::string_view PROPERTIES = "properties";
            static constexpr std::string_view FRAMES = "frames";
        };

        std::string map_path_;       ///< 地图路径
        glm::ivec2 map_size_{0, 0};  ///< 地图尺寸(瓦片数量)
        glm::ivec2 tile_size_{0, 0}; ///< 瓦片尺寸(像素)
        // 使用 map 解决使用 unordered_map 时，无序查找的错误问题
        std::map<int, nlohmann::json> tileset_data_; ///< firstgid -> 瓦片集数据

    public:
        LevelLoader() = default;
        ~LevelLoader() = default;

        // 禁用拷贝和移动语义
        LevelLoader(const LevelLoader &) = delete;
        LevelLoader &operator=(const LevelLoader &) = delete;
        LevelLoader(LevelLoader &&) = delete;
        LevelLoader &operator=(LevelLoader &&) = delete;

        /**
         * @brief 加载关卡数据到指定的 Scene 对象中。
         * @param level_path Tiled JSON 地图文件的路径。
         * @param scene 要加载数据的目标 Scene 对象。
         * @return bool 是否加载成功。
         */
        bool loadLevel(std::string_view level_path, Scene &scene);

    private:
        // ========== 核心加载流程函数 ==========
        [[nodiscard]] std::optional<nlohmann::json> loadJsonFile(std::string_view file_path) const;
        [[nodiscard]] bool parseMapBasicInfo(const nlohmann::json &json_data);
        [[nodiscard]] bool loadAllTilesets(const nlohmann::json &json_data);
        [[nodiscard]] bool loadAllLayers(const nlohmann::json &json_data, Scene &scene);

        // ========== 图层加载函数 ==========
        void loadImageLayer(const nlohmann::json &layer_json, Scene &scene);
        void loadTileLayer(const nlohmann::json &layer_json, Scene &scene);
        void loadObjectLayer(const nlohmann::json &layer_json, Scene &scene);

        // =========== 游戏对象创建函数 ==========
        [[nodiscard]] bool processCustomShapeObject(const nlohmann::json &object_json,
                                                    Scene &scene);

        [[nodiscard]] std::unique_ptr<engine::object::GameObject>
        createGameObjectFromObject(const nlohmann::json &object_json, int gid, Scene &scene);

        struct ObjectTransformData {
            glm::vec2 position;
            glm::vec2 scale;
            float rotation;
        };
        [[nodiscard]] std::optional<ObjectTransformData>
        parseObjectTransform(const nlohmann::json &object_json, const glm::vec2 &src_size);

        void setupObjectCollision(engine::object::GameObject &game_object,
                                  const engine::component::TileInfo &tile_info,
                                  const std::optional<nlohmann::json> &tile_json,
                                  const glm::vec2 &src_size, Scene &scene);

        void applyObjectProperties(engine::object::GameObject &game_object,
                                   const engine::component::TileInfo &tile_info,
                                   const std::optional<nlohmann::json> &tile_json,
                                   const glm::vec2 &src_size, Scene &scene);

        void addAnimation(const nlohmann::json &anim_json,
                          engine::component::AnimationComponent* ac, const glm::vec2 &sprite_size);

        void addSound(const nlohmann::json &sound_json, engine::component::AudioComponent* ac);

        // ========== 瓦片数据处理函数 ==========
        [[nodiscard]] engine::component::TileInfo getTileInfoByGid(int gid) const;
        std::optional<nlohmann::json> getTileJsonByGid(int gid) const;
        [[nodiscard]] engine::component::TileType
        getTileType(const nlohmann::json &tile_json) const;
        [[nodiscard]] engine::component::TileType
        getTileTypeById(const nlohmann::json &tileset_json, int local_id) const;
        void loadTileset(std::string_view tileset_path, int first_gid);

        // ========== 瓦片属性解析函数 ==========
        template <typename T>
        std::optional<T> getTileProperty(const nlohmann::json &tile_json,
                                         std::string_view property_name) const;
        std::optional<engine::utils::Rect> getColliderRect(const nlohmann::json &tile_json) const;

        // ========== 验证函数 ==========
        [[nodiscard]] bool validateMapData() const;
        [[nodiscard]] bool validateLayerData(const nlohmann::json &layer_json,
                                             std::string_view expected_key) const;

        // ========== 通用工具函数 ==========
        [[nodiscard]] std::string resolvePath(std::string relative_path,
                                              std::string file_path) const;
        template <typename T>
        [[nodiscard]] T getJsonValue(const nlohmann::json &json, std::string_view key,
                                     T default_value) const;
        [[nodiscard]] std::string getLayerName(const nlohmann::json &layer_json) const;
    };

} // namespace engine::scene
