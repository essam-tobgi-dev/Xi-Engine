#pragma once

#include "../../Renderer/Camera.h"

namespace Xi {

    struct CameraComponent {
        Camera camera;
        bool isMain = false;
        int priority = 0;
        float clearColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};
        bool clearDepth = true;

        // Viewport (normalized 0-1)
        float viewportX = 0.0f;
        float viewportY = 0.0f;
        float viewportWidth = 1.0f;
        float viewportHeight = 1.0f;
    };

}
