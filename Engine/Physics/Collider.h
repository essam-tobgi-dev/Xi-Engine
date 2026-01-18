#pragma once

#include <glm/glm.hpp>

namespace Xi {

    // AABB collision structure
    struct AABB {
        glm::vec3 min = glm::vec3(0.0f);
        glm::vec3 max = glm::vec3(0.0f);

        AABB() = default;
        AABB(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}

        glm::vec3 GetCenter() const { return (min + max) * 0.5f; }
        glm::vec3 GetExtents() const { return (max - min) * 0.5f; }
        glm::vec3 GetSize() const { return max - min; }

        bool Contains(const glm::vec3& point) const {
            return point.x >= min.x && point.x <= max.x &&
                   point.y >= min.y && point.y <= max.y &&
                   point.z >= min.z && point.z <= max.z;
        }

        bool Intersects(const AABB& other) const {
            return min.x <= other.max.x && max.x >= other.min.x &&
                   min.y <= other.max.y && max.y >= other.min.y &&
                   min.z <= other.max.z && max.z >= other.min.z;
        }

        void Expand(const glm::vec3& point) {
            min = glm::min(min, point);
            max = glm::max(max, point);
        }

        void Expand(const AABB& other) {
            min = glm::min(min, other.min);
            max = glm::max(max, other.max);
        }
    };

    // Sphere collision structure
    struct BoundingSphere {
        glm::vec3 center = glm::vec3(0.0f);
        float radius = 0.5f;

        BoundingSphere() = default;
        BoundingSphere(const glm::vec3& c, float r) : center(c), radius(r) {}

        bool Contains(const glm::vec3& point) const {
            return glm::length(point - center) <= radius;
        }

        bool Intersects(const BoundingSphere& other) const {
            float distance = glm::length(other.center - center);
            return distance <= (radius + other.radius);
        }

        bool Intersects(const AABB& aabb) const {
            // Find closest point on AABB to sphere center
            glm::vec3 closest;
            closest.x = glm::clamp(center.x, aabb.min.x, aabb.max.x);
            closest.y = glm::clamp(center.y, aabb.min.y, aabb.max.y);
            closest.z = glm::clamp(center.z, aabb.min.z, aabb.max.z);

            float distanceSquared = glm::dot(closest - center, closest - center);
            return distanceSquared <= (radius * radius);
        }
    };

    // Ray for raycasting
    struct Ray {
        glm::vec3 origin = glm::vec3(0.0f);
        glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);

        Ray() = default;
        Ray(const glm::vec3& o, const glm::vec3& d) : origin(o), direction(glm::normalize(d)) {}

        glm::vec3 GetPoint(float t) const {
            return origin + direction * t;
        }
    };

}
