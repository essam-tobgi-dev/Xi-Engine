#pragma once

#include <glm/glm.hpp>

namespace Xi {

    enum class ColliderType {
        Box,
        Sphere,
        Capsule
    };

    struct Collider {
        ColliderType type = ColliderType::Box;
        glm::vec3 center = glm::vec3(0.0f);

        // Box collider
        glm::vec3 size = glm::vec3(1.0f);

        // Sphere collider
        float radius = 0.5f;

        // Capsule collider
        float height = 1.0f;
        // radius is shared with sphere

        bool isTrigger = false;

        // Physics layer for collision filtering
        uint32_t layer = 0;
        uint32_t mask = 0xFFFFFFFF;

        // Computed AABB (world space)
        glm::vec3 GetAABBMin(const glm::vec3& position, const glm::vec3& scale) const {
            glm::vec3 worldCenter = position + center;
            if (type == ColliderType::Box) {
                glm::vec3 halfExtents = size * scale * 0.5f;
                return worldCenter - halfExtents;
            } else if (type == ColliderType::Sphere) {
                float worldRadius = radius * glm::max(scale.x, glm::max(scale.y, scale.z));
                return worldCenter - glm::vec3(worldRadius);
            } else {
                float worldRadius = radius * glm::max(scale.x, scale.z);
                float halfHeight = height * scale.y * 0.5f;
                return worldCenter - glm::vec3(worldRadius, halfHeight + worldRadius, worldRadius);
            }
        }

        glm::vec3 GetAABBMax(const glm::vec3& position, const glm::vec3& scale) const {
            glm::vec3 worldCenter = position + center;
            if (type == ColliderType::Box) {
                glm::vec3 halfExtents = size * scale * 0.5f;
                return worldCenter + halfExtents;
            } else if (type == ColliderType::Sphere) {
                float worldRadius = radius * glm::max(scale.x, glm::max(scale.y, scale.z));
                return worldCenter + glm::vec3(worldRadius);
            } else {
                float worldRadius = radius * glm::max(scale.x, scale.z);
                float halfHeight = height * scale.y * 0.5f;
                return worldCenter + glm::vec3(worldRadius, halfHeight + worldRadius, worldRadius);
            }
        }
    };

}
