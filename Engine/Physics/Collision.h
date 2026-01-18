#pragma once

#include "../ECS/Entity.h"
#include <glm/glm.hpp>

namespace Xi {

    struct CollisionInfo {
        Entity entityA = INVALID_ENTITY;
        Entity entityB = INVALID_ENTITY;

        glm::vec3 contactPoint = glm::vec3(0.0f);
        glm::vec3 contactNormal = glm::vec3(0.0f);
        float penetrationDepth = 0.0f;

        bool isTrigger = false;
    };

    struct RaycastHit {
        Entity entity = INVALID_ENTITY;
        glm::vec3 point = glm::vec3(0.0f);
        glm::vec3 normal = glm::vec3(0.0f);
        float distance = 0.0f;
        bool hit = false;
    };

}
