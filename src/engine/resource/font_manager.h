#pragma once
#include <SDL3_ttf/SDL_ttf.h> // SDL_ttf 主头文件
#include <functional>         // 用于 std::hash
#include <memory>             // 用于 std::unique_ptr
#include <string>             // 用于 std::string
#include <string_view>        // 用于 std::string_view
#include <unordered_map>      // 用于 std::unordered_map
#include <utility>            // 用于 std::pair

namespace engine::resource {

    // 定义字体键类型（路径 + 大小）
    using FontKey = std::pair<std::string, int>;

    // FontKey 的自定义哈希函数（std::pair<std::string, int>），用于 std::unordered_map
    struct FontKeyHash {
        std::size_t operator()(const FontKey &key) const
        {
            std::hash<std::string> string_hasher;
            std::hash<int> int_hasher;
            return string_hasher(key.first) ^
                   int_hasher(
                       key.second); // 异或运算符 ^
                                    // 按位计算，每一位的两个值不同为1，相同为0，这是合并两个哈希值的简单方法
        }
    };

    /**
     * @brief 管理 SDL_ttf 字体资源（TTF_Font）。
     *
     * 提供字体的加载和缓存功能，通过文件路径和点大小来标识。
     * 构造失败会抛出异常。仅供 ResourceManager 内部使用。
     */
    class FontManager final {
        friend class ResourceManager;

    private:
        // TTF_Font 的自定义删除器
        struct SDLFontDeleter {
            void operator()(TTF_Font* font) const
            {
                if (font != nullptr) {
                    TTF_CloseFont(font);
                }
            }
        };

        // 字体存储（FontKey -> TTF_Font）。
        // unordered_map 的键需要能转换为哈希值，对于基础数据类型，系统会自动转换
        // 但是对于对于自定义类型（系统无法自动转化），则需要提供自定义哈希函数（第三个模版参数）
        std::unordered_map<FontKey, std::unique_ptr<TTF_Font, SDLFontDeleter>, FontKeyHash> fonts_;

    public:
        /**
         * @brief 构造函数。初始化 SDL_ttf。
         * @throws std::runtime_error 如果 SDL_ttf 初始化失败。
         */
        FontManager();

        ~FontManager(); ///< @brief 需要手动添加析构函数，清理资源并关闭 SDL_ttf。

        // 当前设计中，我们只需要一个FontManager，所有权不变，所以不需要拷贝、移动相关构造及赋值运算符
        FontManager(const FontManager &) = delete;
        FontManager &operator=(const FontManager &) = delete;
        FontManager(FontManager &&) = delete;
        FontManager &operator=(FontManager &&) = delete;

    private: // 仅由 ResourceManager（和内部）访问的方法
        TTF_Font* loadFont(std::string_view file_path, int point_size);
        TTF_Font* getFont(std::string_view file_path, int point_size);
        void unloadFont(std::string_view file_path, int point_size);
        void clearFonts();
    };
} // namespace engine::resource
