#pragma once

#include <glm/glm.hpp>

namespace Xi {

    enum class LightType {
        Directional,
        Point,
        Spot
    };

    struct Light {
        LightType type = LightType::Directional;
        glm::vec3 color = glm::vec3(1.0f);
        float intensity = 1.0f;

        // Point/Spot light properties
        float range = 10.0f;
        float attenuation = 1.0f;

        // Spot light properties
        float innerAngle = 30.0f;
        float outerAngle = 45.0f;

        // Shadow properties
        bool castShadows = false;
        float shadowBias = 0.005f;
        int shadowMapResolution = 1024;
    };

}
