#pragma once

#include "Collision.h"
#include "Collider.h"
#include "../ECS/Entity.h"
#include <vector>
#include <functional>

namespace Xi {

    class World;

    using CollisionCallback = std::function<void(const CollisionInfo&)>;

    class PhysicsWorld {
    public:
        PhysicsWorld();
        ~PhysicsWorld();

        void SetWorld(World* world) { m_World = world; }

        void Step(float dt);

        // Raycasting
        RaycastHit Raycast(const Ray& ray, float maxDistance = 1000.0f, uint32_t layerMask = 0xFFFFFFFF);
        std::vector<RaycastHit> RaycastAll(const Ray& ray, float maxDistance = 1000.0f, uint32_t layerMask = 0xFFFFFFFF);

        // Overlap tests
        std::vector<Entity> OverlapSphere(const glm::vec3& center, float radius, uint32_t layerMask = 0xFFFFFFFF);
        std::vector<Entity> OverlapBox(const glm::vec3& center, const glm::vec3& halfExtents, uint32_t layerMask = 0xFFFFFFFF);

        // Collision callbacks
        void SetCollisionCallback(CollisionCallback callback) { m_CollisionCallback = callback; }

        // Settings
        void SetGravity(const glm::vec3& gravity) { m_Gravity = gravity; }
        glm::vec3 GetGravity() const { return m_Gravity; }

        // Debug
        const std::vector<CollisionInfo>& GetCollisions() const { return m_Collisions; }

    private:
        void IntegratePhysics(float dt);
        void DetectCollisions();
        void ResolveCollisions();

        bool TestAABBAABB(const AABB& a, const AABB& b, CollisionInfo& info);
        bool TestSphereSphere(const BoundingSphere& a, const BoundingSphere& b, CollisionInfo& info);
        bool TestSphereAABB(const BoundingSphere& sphere, const AABB& aabb, CollisionInfo& info);
        bool TestRayAABB(const Ray& ray, const AABB& aabb, float& tMin, float& tMax);
        bool TestRaySphere(const Ray& ray, const BoundingSphere& sphere, float& t);

        World* m_World = nullptr;
        glm::vec3 m_Gravity = glm::vec3(0.0f, -9.81f, 0.0f);

        std::vector<CollisionInfo> m_Collisions;
        CollisionCallback m_CollisionCallback;
    };

}
