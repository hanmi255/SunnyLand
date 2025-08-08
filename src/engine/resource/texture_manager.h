/***
 * @Author: hanmi255 hanmi2550505@gmail.com
 * @Date: 2025-07-29 15:48:15
 * @LastEditTime: 2025-07-29 16:11:37
 * @LastEditors: hanmi255 hanmi2550505@gmail.com
 * @Description:
 * @FilePath: \SunnyLand\src\engine\resource\texture_manager.h
 * @技术宅拯救世界！！！
 */
#pragma once
#include <SDL3/SDL_render.h> // 用于 SDL_Texture 和 SDL_Renderer
#include <glm/glm.hpp>
#include <memory>            // 用于 std::shared_ptr
#include <string>            // 用于 std::string
#include <string_view>       // 用于 std::string_view
#include <unordered_map>     // 用于 std::unordered_map

namespace engine::resource {

    /**
     * @brief 管理 SDL_Texture 资源的加载、存储和检索。
     *
     * 在构造时初始化。使用文件路径作为键，确保纹理只加载一次并正确释放。
     * 依赖于一个有效的 SDL_Renderer，构造失败会抛出异常。
     */
    class TextureManager final {
        friend class ResourceManager;

    private:
        struct SDLTextureDeleter {
            void operator()(SDL_Texture* texture) const
            {
                if (texture) {
                    SDL_DestroyTexture(texture);
                }
            }
        };

        // 存储文件路径和指向管理纹理的 unique_ptr 的映射。(容器的键不可使用std::string_view)
        std::unordered_map<std::string, std::unique_ptr<SDL_Texture, SDLTextureDeleter>> textures_;
        SDL_Renderer* renderer_ = nullptr;

    public:
        /**
         * @brief 构造函数，执行初始化。
         * @param renderer 指向有效的 SDL_Renderer 上下文的指针。不能为空。
         * @throws std::runtime_error 如果 renderer 为 nullptr 或初始化失败。
         */
        explicit TextureManager(SDL_Renderer* renderer);

        TextureManager(const TextureManager &) = delete;
        TextureManager &operator=(const TextureManager &) = delete;
        TextureManager(TextureManager &&) = delete;
        TextureManager &operator=(TextureManager &&) = delete;

    private:
        SDL_Texture* loadTexture(std::string_view file_path);
        SDL_Texture* getTexture(std::string_view file_path);
        glm::vec2 getTextureSize(std::string_view file_path);
        void unloadTexture(std::string_view file_path);
        void clearTextures();
    };
} // namespace engine::resource
