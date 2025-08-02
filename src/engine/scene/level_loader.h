#pragma once
#include <glm/vec2.hpp>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>

namespace engine::component {
    struct TileInfo;
}

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
        };

        std::string map_path_;                       ///< 地图路径
        glm::ivec2 map_size_{0, 0};                  ///< 地图尺寸(瓦片数量)
        glm::ivec2 tile_size_{0, 0};                 ///< 瓦片尺寸(像素)
        // 使用 map 解决使用 unordered_map 时，无序查找的错误问题
        std::map<int, nlohmann::json> tileset_data_; ///< firstgid -> 瓦片集数据

    public:
        LevelLoader() = default;
        ~LevelLoader() = default;

        // 禁用拷贝和移动，避免意外的资源复制
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
        bool loadLevel(const std::string &level_path, Scene &scene);

    private:
        // 核心加载函数
        [[nodiscard]] std::optional<nlohmann::json>
        loadJsonFile(const std::string &file_path) const;
        [[nodiscard]] bool parseMapBasicInfo(const nlohmann::json &json_data);
        [[nodiscard]] bool loadAllTilesets(const nlohmann::json &json_data);
        [[nodiscard]] bool loadAllLayers(const nlohmann::json &json_data, Scene &scene);

        // 图层加载函数
        void loadImageLayer(const nlohmann::json &layer_json, Scene &scene);
        void loadTileLayer(const nlohmann::json &layer_json, Scene &scene);
        void loadObjectLayer(const nlohmann::json &layer_json, Scene &scene);

        // 工具函数
        [[nodiscard]] engine::component::TileInfo getTileInfoByGid(int gid) const;
        void loadTileset(const std::string &tileset_path, int first_gid);
        [[nodiscard]] std::string resolvePath(const std::string &relative_path,
                                              const std::string &file_path) const;

        // 验证函数
        [[nodiscard]] bool validateMapData() const;
        [[nodiscard]] bool validateLayerData(const nlohmann::json &layer_json,
                                             std::string_view expected_key) const;

        // 辅助函数
        template <typename T>
        [[nodiscard]] T getJsonValue(const nlohmann::json &json, std::string_view key,
                                     T default_value) const;

        [[nodiscard]] std::string getLayerName(const nlohmann::json &layer_json) const;
    };

} // namespace engine::scene
