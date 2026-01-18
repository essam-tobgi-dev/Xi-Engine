#include "AudioEngine.h"
#include "AudioClip.h"
#include "../Core/Log.h"
#include "../ECS/World.h"
#include "../ECS/Components/Transform.h"
#include "../ECS/Components/AudioSource.h"

#include <miniaudio.h>

namespace Xi {

    AudioEngine::AudioEngine() = default;

    AudioEngine::~AudioEngine() {
        Shutdown();
    }

    bool AudioEngine::Init() {
        m_Engine = new ma_engine();

        ma_engine_config config = ma_engine_config_init();
        config.channels = 2;
        config.sampleRate = 44100;

        ma_result result = ma_engine_init(&config, m_Engine);
        if (result != MA_SUCCESS) {
            XI_LOG_ERROR("Failed to initialize audio engine");
            delete m_Engine;
            m_Engine = nullptr;
            return false;
        }

        XI_LOG_INFO("Audio engine initialized");
        return true;
    }

    void AudioEngine::Shutdown() {
        StopAll();

        if (m_Engine) {
            ma_engine_uninit(m_Engine);
            delete m_Engine;
            m_Engine = nullptr;
        }
    }

    void AudioEngine::Update(World& world) {
        if (!m_Engine) return;

        // Update audio sources from ECS
        auto* transformPool = world.GetComponentPool<Transform>();
        auto* audioPool = world.GetComponentPool<AudioSource>();

        if (!transformPool || !audioPool) return;

        for (Entity entity : audioPool->GetEntities()) {
            if (!world.HasComponent<Transform>(entity)) continue;

            const Transform& transform = world.GetComponent<Transform>(entity);
            AudioSource& source = world.GetComponent<AudioSource>(entity);

            // Update position for 3D sounds
            if (source.internalHandle != 0 && source.is3D) {
                SetPosition(source.internalHandle, transform.position);
            }

            // Handle play on awake
            if (source.playOnAwake && !source.isPlaying && source.clip) {
                if (source.is3D) {
                    source.internalHandle = Play3D(source.clip, transform.position, source.loop);
                } else {
                    source.internalHandle = Play(source.clip, source.loop);
                }
                SetVolume(source.internalHandle, source.volume);
                SetPitch(source.internalHandle, source.pitch);
                source.isPlaying = true;
                source.playOnAwake = false;
            }

            // Check if sound finished
            if (source.internalHandle != 0 && !IsPlaying(source.internalHandle)) {
                source.isPlaying = false;
                source.internalHandle = 0;
            }
        }
    }

    uint32_t AudioEngine::Play(std::shared_ptr<AudioClip> clip, bool loop) {
        if (!m_Engine || !clip || !clip->IsLoaded()) return 0;

        uint32_t handle = m_NextHandle++;

        SoundInstance instance;
        instance.clip = clip;
        instance.is3D = false;
        instance.sound = new ma_sound();

        ma_result result = ma_sound_init_from_file(m_Engine, clip->GetPath().c_str(),
            MA_SOUND_FLAG_DECODE, nullptr, nullptr, instance.sound);

        if (result != MA_SUCCESS) {
            XI_LOG_ERROR("Failed to play sound: " + clip->GetPath());
            delete instance.sound;
            return 0;
        }

        ma_sound_set_looping(instance.sound, loop ? MA_TRUE : MA_FALSE);
        ma_sound_start(instance.sound);

        m_Sounds[handle] = instance;
        return handle;
    }

    uint32_t AudioEngine::Play3D(std::shared_ptr<AudioClip> clip, const glm::vec3& position, bool loop) {
        if (!m_Engine || !clip || !clip->IsLoaded()) return 0;

        uint32_t handle = m_NextHandle++;

        SoundInstance instance;
        instance.clip = clip;
        instance.is3D = true;
        instance.sound = new ma_sound();

        ma_result result = ma_sound_init_from_file(m_Engine, clip->GetPath().c_str(),
            MA_SOUND_FLAG_DECODE, nullptr, nullptr, instance.sound);

        if (result != MA_SUCCESS) {
            XI_LOG_ERROR("Failed to play 3D sound: " + clip->GetPath());
            delete instance.sound;
            return 0;
        }

        ma_sound_set_spatialization_enabled(instance.sound, MA_TRUE);
        ma_sound_set_position(instance.sound, position.x, position.y, position.z);
        ma_sound_set_looping(instance.sound, loop ? MA_TRUE : MA_FALSE);
        ma_sound_start(instance.sound);

        m_Sounds[handle] = instance;
        return handle;
    }

    void AudioEngine::Stop(uint32_t handle) {
        auto it = m_Sounds.find(handle);
        if (it == m_Sounds.end()) return;

        ma_sound_stop(it->second.sound);
        ma_sound_uninit(it->second.sound);
        delete it->second.sound;
        m_Sounds.erase(it);
    }

    void AudioEngine::StopAll() {
        for (auto& [handle, instance] : m_Sounds) {
            ma_sound_stop(instance.sound);
            ma_sound_uninit(instance.sound);
            delete instance.sound;
        }
        m_Sounds.clear();
    }

    void AudioEngine::SetVolume(uint32_t handle, float volume) {
        auto it = m_Sounds.find(handle);
        if (it != m_Sounds.end()) {
            ma_sound_set_volume(it->second.sound, volume * m_MasterVolume);
        }
    }

    void AudioEngine::SetPitch(uint32_t handle, float pitch) {
        auto it = m_Sounds.find(handle);
        if (it != m_Sounds.end()) {
            ma_sound_set_pitch(it->second.sound, pitch);
        }
    }

    void AudioEngine::SetPosition(uint32_t handle, const glm::vec3& position) {
        auto it = m_Sounds.find(handle);
        if (it != m_Sounds.end() && it->second.is3D) {
            ma_sound_set_position(it->second.sound, position.x, position.y, position.z);
        }
    }

    void AudioEngine::SetLooping(uint32_t handle, bool loop) {
        auto it = m_Sounds.find(handle);
        if (it != m_Sounds.end()) {
            ma_sound_set_looping(it->second.sound, loop ? MA_TRUE : MA_FALSE);
        }
    }

    bool AudioEngine::IsPlaying(uint32_t handle) const {
        auto it = m_Sounds.find(handle);
        if (it == m_Sounds.end()) return false;
        return ma_sound_is_playing(it->second.sound) == MA_TRUE;
    }

    void AudioEngine::SetListenerPosition(const glm::vec3& position) {
        m_ListenerPosition = position;
        if (m_Engine) {
            ma_engine_listener_set_position(m_Engine, 0, position.x, position.y, position.z);
        }
    }

    void AudioEngine::SetListenerOrientation(const glm::vec3& forward, const glm::vec3& up) {
        m_ListenerForward = forward;
        m_ListenerUp = up;
        if (m_Engine) {
            ma_engine_listener_set_direction(m_Engine, 0, forward.x, forward.y, forward.z);
            ma_engine_listener_set_world_up(m_Engine, 0, up.x, up.y, up.z);
        }
    }

    void AudioEngine::SetMasterVolume(float volume) {
        m_MasterVolume = volume;
        if (m_Engine) {
            ma_engine_set_volume(m_Engine, volume);
        }
    }

}
