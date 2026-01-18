#include "AudioClip.h"
#include "../Core/Log.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace Xi {

    AudioClip::AudioClip() = default;

    AudioClip::~AudioClip() {
        Unload();
    }

    bool AudioClip::LoadFromFile(const std::string& path) {
        Unload();

        ma_decoder decoder;
        ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 2, 44100);

        ma_result result = ma_decoder_init_file(path.c_str(), &config, &decoder);
        if (result != MA_SUCCESS) {
            XI_LOG_ERROR("Failed to load audio file: " + path);
            return false;
        }

        m_SampleRate = decoder.outputSampleRate;
        m_Channels = decoder.outputChannels;

        // Get total frame count
        ma_uint64 totalFrames;
        ma_decoder_get_length_in_pcm_frames(&decoder, &totalFrames);

        // Allocate buffer
        m_Samples.resize(totalFrames * m_Channels);

        // Read all samples
        ma_uint64 framesRead;
        ma_decoder_read_pcm_frames(&decoder, m_Samples.data(), totalFrames, &framesRead);

        m_Duration = static_cast<float>(framesRead) / static_cast<float>(m_SampleRate);
        m_Path = path;
        m_Loaded = true;

        ma_decoder_uninit(&decoder);

        XI_LOG_INFO("Audio loaded: " + path + " (" + std::to_string(m_Duration) + "s)");
        return true;
    }

    void AudioClip::Unload() {
        m_Samples.clear();
        m_SampleRate = 0;
        m_Channels = 0;
        m_Duration = 0.0f;
        m_Loaded = false;
        m_Path.clear();
    }

}
