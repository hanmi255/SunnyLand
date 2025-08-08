#pragma once
#include <SDL3_mixer/SDL_mixer.h> // SDL_mixer 主头文件
#include <memory>                 // 用于 std::unique_ptr
#include <string>                 // 用于 std::string
#include <string_view>            // 用于 std::string_view
#include <unordered_map>          // 用于 std::unordered_map

namespace engine::resource {

    /**
     * @brief 管理 SDL_mixer 音效 (Mix_Chunk) 和音乐 (Mix_Music)。
     *
     * 提供音频资源的加载和缓存功能。构造失败时会抛出异常。
     * 仅供 ResourceManager 内部使用。
     */
    class AudioManager final {
        friend class ResourceManager;

    private:
        // Mix_Chunk 的自定义删除器
        struct SDLMixChunkDeleter {
            void operator()(Mix_Chunk* chunk) const
            {
                if (chunk) {
                    Mix_FreeChunk(chunk);
                }
            }
        };

        // Mix_Music 的自定义删除器
        struct SDLMixMusicDeleter {
            void operator()(Mix_Music* music) const
            {
                if (music) {
                    Mix_FreeMusic(music);
                }
            }
        };

        // 音效存储 (文件路径 -> Mix_Chunk)
        std::unordered_map<std::string, std::unique_ptr<Mix_Chunk, SDLMixChunkDeleter>> sounds_;
        // 音乐存储 (文件路径 -> Mix_Music)
        std::unordered_map<std::string, std::unique_ptr<Mix_Music, SDLMixMusicDeleter>> music_;

    public:
        /**
         * @brief 构造函数。初始化 SDL_mixer 并打开音频设备。
         * @throws std::runtime_error 如果 SDL_mixer 初始化或打开音频设备失败。
         */
        AudioManager();

        ~AudioManager();

        AudioManager(const AudioManager &) = delete;
        AudioManager &operator=(const AudioManager &) = delete;
        AudioManager(AudioManager &&) = delete;
        AudioManager &operator=(AudioManager &&) = delete;

    private:
        Mix_Chunk* loadSound(std::string_view file_path);
        Mix_Chunk* getSound(std::string_view file_path);
        void unloadSound(std::string_view file_path);
        void clearSounds();

        Mix_Music* loadMusic(std::string_view file_path);
        Mix_Music* getMusic(std::string_view file_path);
        void unloadMusic(std::string_view file_path);
        void clearMusic();

        void clearAudio();
    };
} // namespace engine::resource
