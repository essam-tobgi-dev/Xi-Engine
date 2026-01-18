#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace Xi {

    class AudioClip {
    public:
        AudioClip();
        ~AudioClip();

        bool LoadFromFile(const std::string& path);
        void Unload();

        bool IsLoaded() const { return m_Loaded; }
        const std::string& GetPath() const { return m_Path; }

        uint32_t GetSampleRate() const { return m_SampleRate; }
        uint32_t GetChannels() const { return m_Channels; }
        float GetDuration() const { return m_Duration; }

        const std::vector<float>& GetSamples() const { return m_Samples; }

    private:
        std::string m_Path;
        std::vector<float> m_Samples;
        uint32_t m_SampleRate = 0;
        uint32_t m_Channels = 0;
        float m_Duration = 0.0f;
        bool m_Loaded = false;
    };

}
