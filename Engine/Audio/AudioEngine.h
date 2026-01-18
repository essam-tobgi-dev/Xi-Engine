#pragma once

#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

struct ma_engine;
struct ma_sound;

namespace Xi {

    class AudioClip;
    class World;

    class AudioEngine {
    public:
        AudioEngine();
        ~AudioEngine();

        bool Init();
        void Shutdown();
        void Update(World& world);

        // Sound playback
        uint32_t Play(std::shared_ptr<AudioClip> clip, bool loop = false);
        uint32_t Play3D(std::shared_ptr<AudioClip> clip, const glm::vec3& position, bool loop = false);

        void Stop(uint32_t handle);
        void StopAll();

        void SetVolume(uint32_t handle, float volume);
        void SetPitch(uint32_t handle, float pitch);
        void SetPosition(uint32_t handle, const glm::vec3& position);
        void SetLooping(uint32_t handle, bool loop);

        bool IsPlaying(uint32_t handle) const;

        // Listener (camera) position
        void SetListenerPosition(const glm::vec3& position);
        void SetListenerOrientation(const glm::vec3& forward, const glm::vec3& up);

        // Master volume
        void SetMasterVolume(float volume);
        float GetMasterVolume() const { return m_MasterVolume; }

    private:
        struct SoundInstance {
            ma_sound* sound = nullptr;
            std::shared_ptr<AudioClip> clip;
            bool is3D = false;
        };

        ma_engine* m_Engine = nullptr;
        std::unordered_map<uint32_t, SoundInstance> m_Sounds;
        uint32_t m_NextHandle = 1;
        float m_MasterVolume = 1.0f;

        glm::vec3 m_ListenerPosition = glm::vec3(0.0f);
        glm::vec3 m_ListenerForward = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 m_ListenerUp = glm::vec3(0.0f, 1.0f, 0.0f);
    };

}
