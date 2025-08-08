/***
 * @Author: hanmi255 hanmi2550505@gmail.com
 * @Date: 2025-07-29 15:45:09
 * @LastEditTime: 2025-07-29 15:45:09
 * @LastEditors: hanmi255 hanmi2550505@gmail.com
 * @Description:
 * @FilePath: \SunnyLand\src\resource\resource_manager.h
 * @技术宅拯救世界！！！
 */
#pragma once
#include <glm/glm.hpp>
#include <memory>      // 用于 std::shared_ptr
#include <string_view> // 用于 std::string_view

// 前向声明 SDL 类型
struct SDL_Renderer;
struct SDL_Texture;
struct Mix_Chunk;
struct Mix_Music;
struct TTF_Font;

namespace engine::resource {
    class TextureManager;
    class AudioManager;
    class FontManager;

    /**
     * @brief 作为访问各种资源管理器的中央控制点（外观模式 Facade）。
     * 在构造时初始化其管理的子系统。构造失败会抛出异常。
     */
    class ResourceManager final {
    private:
        // 使用 unique_ptr 确保所有权和自动清理
        std::unique_ptr<TextureManager> texture_manager_;
        std::unique_ptr<AudioManager> audio_manager_;
        std::unique_ptr<FontManager> font_manager_;

    public:
        /**
         * @brief 构造函数，执行初始化。
         * @param renderer SDL_Renderer
         * 的指针，传递给需要它的子管理器。不能为空。
         */
        // explicit 关键字用于防止隐式转换,对于单一参数的构造函数，通常考虑添加
        explicit ResourceManager(SDL_Renderer* renderer);
        ~ResourceManager();

        void clear();

        ResourceManager(const ResourceManager &) = delete;
        ResourceManager &operator=(const ResourceManager &) = delete;
        ResourceManager(ResourceManager &&) = delete;
        ResourceManager &operator=(ResourceManager &&) = delete;

        // --- 统一资源访问接口 ---
        // -- Texture --
        SDL_Texture* loadTexture(std::string_view file_path);
        SDL_Texture* getTexture(std::string_view file_path);
        void unloadTexture(std::string_view file_path);
        glm::vec2 getTextureSize(std::string_view file_path);
        void clearTextures();

        // -- Sound Effects (Chunks) --
        Mix_Chunk* loadSound(std::string_view file_path);
        Mix_Chunk* getSound(std::string_view file_path);
        void unloadSound(std::string_view file_path);
        void clearSounds();

        // -- Music --
        Mix_Music* loadMusic(std::string_view file_path);
        Mix_Music* getMusic(std::string_view file_path);
        void unloadMusic(std::string_view file_path);
        void clearMusic();

        // -- Fonts --
        TTF_Font* loadFont(std::string_view file_path, int point_size);
        TTF_Font* getFont(std::string_view file_path, int point_size);
        void unloadFont(std::string_view file_path, int point_size);
        void clearFonts();
    };
} // namespace engine::resource
