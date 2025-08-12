#include "audio_component.h"
#include "../audio/audio_player.h"
#include "../object/game_object.h"
#include "../render/camera.h"
#include "transform_component.h"
#include <spdlog/spdlog.h>

namespace engine::component {

    AudioComponent::AudioComponent(engine::audio::AudioPlayer* audio_player,
                                   engine::render::Camera* camera)
        : audio_player_(audio_player), camera_(camera)
    {
        if (!audio_player_ || !camera_) {
            spdlog::error("AudioComponent 初始化失败: 音频播放器或相机为空");
        }
    }

    void AudioComponent::init()
    {
        if (!owner_) {
            spdlog::error("AudioComponent 在初始化前未设置 owner_。");
            return;
        }
        transform_component_ = owner_->getComponent<TransformComponent>();
        if (!transform_component_) {
            spdlog::warn(
                "GameObject '{}' 上的 AudioComponent 需要一个 TransformComponent，但未找到。",
                owner_->getName());
        }
    }

    void AudioComponent::playSound(std::string_view sound_id, int channel, bool use_spatial)
    {
        // 查找音效ID对应的路径，如果找不到则直接使用sound_id作为路径
        const auto sound_path = sound_id_to_path_.find(std::string(sound_id)) !=
                                        sound_id_to_path_.end()
                                    ? sound_id_to_path_[(std::string(sound_id))]
                                    : sound_id;

        if (use_spatial && transform_component_) {
            // 空间定位音效：只在距离相机150像素范围内播放
            // TODO: SDL_Mixer不支持完整空间定位，未来更换音频库时可以方便地实现
            const auto camera_center = camera_->getPosition() + camera_->getViewportSize() / 2.0f;
            const auto object_pos = transform_component_->getPosition();
            const float distance = glm::length(camera_center - object_pos);

            if (distance > 150.0f) {
                spdlog::debug("AudioComponent::playSound: 音效 '{}' 超出范围，不播放。", sound_id);
                return;
            }
        }

        // 播放音效
        audio_player_->playSound(sound_path, channel);
    }

    void AudioComponent::addSound(std::string_view sound_id, std::string_view sound_path)
    {
        if (sound_id_to_path_.find(std::string(sound_id)) != sound_id_to_path_.end()) {
            spdlog::warn("AudioComponent::addSound: 音效 ID '{}' 已存在，覆盖旧路径。", sound_id);
        }
        sound_id_to_path_[std::string(sound_id)] = sound_path;
        spdlog::debug("AudioComponent::addSound: 添加音效 ID '{}' 路径 '{}'", sound_id, sound_path);
    }

} // namespace engine::component