#include "Time.h"

namespace Xi {

    Time::TimePoint Time::s_StartTime;
    Time::TimePoint Time::s_LastFrameTime;
    float Time::s_DeltaTime = 0.0f;
    float Time::s_Time = 0.0f;
    float Time::s_FixedDeltaTime = 1.0f / 60.0f;
    float Time::s_Accumulator = 0.0f;
    int Time::s_FPS = 0;
    int Time::s_FrameCount = 0;
    float Time::s_FrameTime = 0.0f;
    float Time::s_FPSTimer = 0.0f;

    void Time::Init() {
        s_StartTime = Clock::now();
        s_LastFrameTime = s_StartTime;
        s_DeltaTime = 0.0f;
        s_Time = 0.0f;
        s_Accumulator = 0.0f;
        s_FPS = 0;
        s_FrameCount = 0;
        s_FPSTimer = 0.0f;
    }

    void Time::Update() {
        TimePoint currentTime = Clock::now();

        auto duration = std::chrono::duration<float>(currentTime - s_LastFrameTime);
        s_DeltaTime = duration.count();

        // Clamp delta time to prevent spiral of death
        if (s_DeltaTime > 0.25f) {
            s_DeltaTime = 0.25f;
        }

        s_LastFrameTime = currentTime;

        auto totalDuration = std::chrono::duration<float>(currentTime - s_StartTime);
        s_Time = totalDuration.count();

        // Accumulate for fixed timestep
        s_Accumulator += s_DeltaTime;

        // FPS calculation
        s_FrameCount++;
        s_FPSTimer += s_DeltaTime;
        if (s_FPSTimer >= 1.0f) {
            s_FPS = s_FrameCount;
            s_FrameTime = 1000.0f / static_cast<float>(s_FrameCount);
            s_FrameCount = 0;
            s_FPSTimer -= 1.0f;
        }
    }

}
