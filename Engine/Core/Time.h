#pragma once

#include <chrono>

namespace Xi {

    class Time {
    public:
        static void Init();
        static void Update();

        static float GetDeltaTime() { return s_DeltaTime; }
        static float GetTime() { return s_Time; }
        static float GetFixedDeltaTime() { return s_FixedDeltaTime; }
        static void SetFixedDeltaTime(float dt) { s_FixedDeltaTime = dt; }

        static int GetFPS() { return s_FPS; }
        static float GetFrameTime() { return s_FrameTime; }

        // For fixed timestep physics
        static float GetAccumulator() { return s_Accumulator; }
        static void ConsumeAccumulator(float dt) { s_Accumulator -= dt; }
        static bool ShouldRunFixedUpdate() { return s_Accumulator >= s_FixedDeltaTime; }

    private:
        using Clock = std::chrono::high_resolution_clock;
        using TimePoint = std::chrono::time_point<Clock>;

        static TimePoint s_StartTime;
        static TimePoint s_LastFrameTime;
        static float s_DeltaTime;
        static float s_Time;
        static float s_FixedDeltaTime;
        static float s_Accumulator;

        // FPS calculation
        static int s_FPS;
        static int s_FrameCount;
        static float s_FrameTime;
        static float s_FPSTimer;
    };

}
