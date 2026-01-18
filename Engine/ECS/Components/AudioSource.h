#pragma once

#include <memory>
#include <string>

namespace Xi {

    class AudioClip;

    struct AudioSource {
        std::shared_ptr<AudioClip> clip;
        std::string clipPath;

        float volume = 1.0f;
        float pitch = 1.0f;
        float minDistance = 1.0f;
        float maxDistance = 500.0f;

        bool playOnAwake = false;
        bool loop = false;
        bool is3D = true;
        bool mute = false;

        // Runtime state
        bool isPlaying = false;
        float playbackPosition = 0.0f;

        // Internal handle (for audio engine)
        uint32_t internalHandle = 0;
    };

}
