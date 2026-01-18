#include "PhysicsWorld.h"
#include "../ECS/World.h"
#include "../ECS/Components/Transform.h"
#include "../ECS/Components/Collider.h"
#include "../ECS/Components/RigidBody.h"
#include "../Core/Log.h"

#include <algorithm>

namespace Xi {

    PhysicsWorld::PhysicsWorld() {
        XI_LOG_INFO("Physics World created");
    }

    PhysicsWorld::~PhysicsWorld() = default;

    void PhysicsWorld::Step(float dt) {
        if (!m_World) return;

        IntegratePhysics(dt);
        DetectCollisions();
        ResolveCollisions();
    }

    void PhysicsWorld::IntegratePhysics(float dt) {
        auto* transformPool = m_World->GetComponentPool<Transform>();
        auto* rigidBodyPool = m_World->GetComponentPool<RigidBody>();

        if (!transformPool || !rigidBodyPool) return;

        for (Entity entity : rigidBodyPool->GetEntities()) {
            if (!m_World->HasComponent<Transform>(entity)) continue;

            Transform& transform = m_World->GetComponent<Transform>(entity);
            RigidBody& rb = m_World->GetComponent<RigidBody>(entity);

            if (rb.type == RigidBodyType::Static) continue;

            // Apply gravity
            if (rb.useGravity && rb.type == RigidBodyType::Dynamic) {
                rb.force += rb.gravity * rb.mass;
            }

            // Integrate velocity
            if (rb.type == RigidBodyType::Dynamic && rb.mass > 0.0f) {
                glm::vec3 acceleration = rb.force / rb.mass;
                rb.velocity += acceleration * dt;

                // Apply drag
                rb.velocity *= (1.0f - rb.drag * dt);
            }

            // Integrate position
            if (!rb.freezePositionX) transform.position.x += rb.velocity.x * dt;
            if (!rb.freezePositionY) transform.position.y += rb.velocity.y * dt;
            if (!rb.freezePositionZ) transform.position.z += rb.velocity.z * dt;

            // Integrate angular velocity
            if (rb.type == RigidBodyType::Dynamic) {
                glm::vec3 angularAcceleration = rb.torque; // Simplified, assumes unit inertia
                rb.angularVelocity += angularAcceleration * dt;
                rb.angularVelocity *= (1.0f - rb.angularDrag * dt);
            }

            // Integrate rotation
            if (!rb.freezeRotationX) transform.rotation.x += glm::degrees(rb.angularVelocity.x) * dt;
            if (!rb.freezeRotationY) transform.rotation.y += glm::degrees(rb.angularVelocity.y) * dt;
            if (!rb.freezeRotationZ) transform.rotation.z += glm::degrees(rb.angularVelocity.z) * dt;

            // Clear forces
            rb.force = glm::vec3(0.0f);
            rb.torque = glm::vec3(0.0f);
        }
    }

    void PhysicsWorld::DetectCollisions() {
        m_Collisions.clear();

        auto* transformPool = m_World->GetComponentPool<Transform>();
        auto* colliderPool = m_World->GetComponentPool<Collider>();

        if (!transformPool || !colliderPool) return;

        const auto& entities = colliderPool->GetEntities();
        size_t count = entities.size();

        // O(n^2) broad phase - could be optimized with spatial partitioning
        for (size_t i = 0; i < count; i++) {
            Entity entityA = entities[i];
            if (!m_World->HasComponent<Transform>(entityA)) continue;

            const Transform& transformA = m_World->GetComponent<Transform>(entityA);
            const Collider& colliderA = m_World->GetComponent<Collider>(entityA);

            AABB aabbA(
                colliderA.GetAABBMin(transformA.position, transformA.scale),
                colliderA.GetAABBMax(transformA.position, transformA.scale)
            );

            for (size_t j = i + 1; j < count; j++) {
                Entity entityB = entities[j];
                if (!m_World->HasComponent<Transform>(entityB)) continue;

                const Transform& transformB = m_World->GetComponent<Transform>(entityB);
                const Collider& colliderB = m_World->GetComponent<Collider>(entityB);

                // Layer filtering
                if (!(colliderA.mask & (1 << colliderB.layer)) ||
                    !(colliderB.mask & (1 << colliderA.layer))) {
                    continue;
                }

                AABB aabbB(
                    colliderB.GetAABBMin(transformB.position, transformB.scale),
                    colliderB.GetAABBMax(transformB.position, transformB.scale)
                );

                CollisionInfo info;
                info.entityA = entityA;
                info.entityB = entityB;
                info.isTrigger = colliderA.isTrigger || colliderB.isTrigger;

                bool collided = false;

                // Test based on collider types
                if (colliderA.type == ColliderType::Box && colliderB.type == ColliderType::Box) {
                    collided = TestAABBAABB(aabbA, aabbB, info);
                } else if (colliderA.type == ColliderType::Sphere && colliderB.type == ColliderType::Sphere) {
                    BoundingSphere sphereA(transformA.position + colliderA.center,
                        colliderA.radius * glm::max(transformA.scale.x, glm::max(transformA.scale.y, transformA.scale.z)));
                    BoundingSphere sphereB(transformB.position + colliderB.center,
                        colliderB.radius * glm::max(transformB.scale.x, glm::max(transformB.scale.y, transformB.scale.z)));
                    collided = TestSphereSphere(sphereA, sphereB, info);
                } else {
                    // Mixed types - use AABB approximation
                    collided = TestAABBAABB(aabbA, aabbB, info);
                }

                if (collided) {
                    m_Collisions.push_back(info);

                    if (m_CollisionCallback) {
                        m_CollisionCallback(info);
                    }
                }
            }
        }
    }

    void PhysicsWorld::ResolveCollisions() {
        for (const CollisionInfo& info : m_Collisions) {
            if (info.isTrigger) continue;

            bool hasRbA = m_World->HasComponent<RigidBody>(info.entityA);
            bool hasRbB = m_World->HasComponent<RigidBody>(info.entityB);

            if (!hasRbA && !hasRbB) continue;

            Transform* transformA = hasRbA ? &m_World->GetComponent<Transform>(info.entityA) : nullptr;
            Transform* transformB = hasRbB ? &m_World->GetComponent<Transform>(info.entityB) : nullptr;
            RigidBody* rbA = hasRbA ? &m_World->GetComponent<RigidBody>(info.entityA) : nullptr;
            RigidBody* rbB = hasRbB ? &m_World->GetComponent<RigidBody>(info.entityB) : nullptr;

            // Calculate masses
            float massA = (rbA && rbA->type == RigidBodyType::Dynamic) ? rbA->mass : 0.0f;
            float massB = (rbB && rbB->type == RigidBodyType::Dynamic) ? rbB->mass : 0.0f;
            float totalMass = massA + massB;

            if (totalMass <= 0.0f) continue;

            // Position correction
            float ratioA = (massA > 0.0f) ? massB / totalMass : 0.0f;
            float ratioB = (massB > 0.0f) ? massA / totalMass : 0.0f;

            if (transformA && rbA && rbA->type == RigidBodyType::Dynamic) {
                transformA->position += info.contactNormal * info.penetrationDepth * ratioA;
            }
            if (transformB && rbB && rbB->type == RigidBodyType::Dynamic) {
                transformB->position -= info.contactNormal * info.penetrationDepth * ratioB;
            }

            // Velocity response
            if (rbA && rbB) {
                glm::vec3 relativeVel = rbA->velocity - rbB->velocity;
                float velAlongNormal = glm::dot(relativeVel, info.contactNormal);

                if (velAlongNormal > 0) continue; // Objects moving apart

                float restitution = glm::min(rbA->bounciness, rbB->bounciness);
                float j = -(1.0f + restitution) * velAlongNormal;
                j /= (1.0f / massA + 1.0f / massB);

                glm::vec3 impulse = j * info.contactNormal;

                if (rbA->type == RigidBodyType::Dynamic) {
                    rbA->velocity += impulse / massA;
                }
                if (rbB->type == RigidBodyType::Dynamic) {
                    rbB->velocity -= impulse / massB;
                }
            }
        }
    }

    bool PhysicsWorld::TestAABBAABB(const AABB& a, const AABB& b, CollisionInfo& info) {
        if (!a.Intersects(b)) return false;

        // Calculate penetration
        glm::vec3 overlap;
        overlap.x = glm::min(a.max.x, b.max.x) - glm::max(a.min.x, b.min.x);
        overlap.y = glm::min(a.max.y, b.max.y) - glm::max(a.min.y, b.min.y);
        overlap.z = glm::min(a.max.z, b.max.z) - glm::max(a.min.z, b.min.z);

        // Find minimum penetration axis
        if (overlap.x < overlap.y && overlap.x < overlap.z) {
            info.penetrationDepth = overlap.x;
            info.contactNormal = (a.GetCenter().x < b.GetCenter().x) ? glm::vec3(-1, 0, 0) : glm::vec3(1, 0, 0);
        } else if (overlap.y < overlap.z) {
            info.penetrationDepth = overlap.y;
            info.contactNormal = (a.GetCenter().y < b.GetCenter().y) ? glm::vec3(0, -1, 0) : glm::vec3(0, 1, 0);
        } else {
            info.penetrationDepth = overlap.z;
            info.contactNormal = (a.GetCenter().z < b.GetCenter().z) ? glm::vec3(0, 0, -1) : glm::vec3(0, 0, 1);
        }

        info.contactPoint = (a.GetCenter() + b.GetCenter()) * 0.5f;
        return true;
    }

    bool PhysicsWorld::TestSphereSphere(const BoundingSphere& a, const BoundingSphere& b, CollisionInfo& info) {
        glm::vec3 diff = b.center - a.center;
        float distance = glm::length(diff);
        float sumRadii = a.radius + b.radius;

        if (distance > sumRadii) return false;

        info.penetrationDepth = sumRadii - distance;
        info.contactNormal = (distance > 0.0001f) ? diff / distance : glm::vec3(0, 1, 0);
        info.contactPoint = a.center + info.contactNormal * a.radius;

        return true;
    }

    bool PhysicsWorld::TestSphereAABB(const BoundingSphere& sphere, const AABB& aabb, CollisionInfo& info) {
        glm::vec3 closest;
        closest.x = glm::clamp(sphere.center.x, aabb.min.x, aabb.max.x);
        closest.y = glm::clamp(sphere.center.y, aabb.min.y, aabb.max.y);
        closest.z = glm::clamp(sphere.center.z, aabb.min.z, aabb.max.z);

        glm::vec3 diff = sphere.center - closest;
        float distanceSquared = glm::dot(diff, diff);

        if (distanceSquared > sphere.radius * sphere.radius) return false;

        float distance = std::sqrt(distanceSquared);
        info.penetrationDepth = sphere.radius - distance;
        info.contactNormal = (distance > 0.0001f) ? diff / distance : glm::vec3(0, 1, 0);
        info.contactPoint = closest;

        return true;
    }

    RaycastHit PhysicsWorld::Raycast(const Ray& ray, float maxDistance, uint32_t layerMask) {
        RaycastHit closestHit;
        closestHit.distance = maxDistance;

        auto* transformPool = m_World->GetComponentPool<Transform>();
        auto* colliderPool = m_World->GetComponentPool<Collider>();

        if (!transformPool || !colliderPool) return closestHit;

        for (Entity entity : colliderPool->GetEntities()) {
            if (!m_World->HasComponent<Transform>(entity)) continue;

            const Transform& transform = m_World->GetComponent<Transform>(entity);
            const Collider& collider = m_World->GetComponent<Collider>(entity);

            if (!(layerMask & (1 << collider.layer))) continue;

            AABB aabb(
                collider.GetAABBMin(transform.position, transform.scale),
                collider.GetAABBMax(transform.position, transform.scale)
            );

            float tMin, tMax;
            if (TestRayAABB(ray, aabb, tMin, tMax) && tMin < closestHit.distance && tMin >= 0) {
                closestHit.hit = true;
                closestHit.entity = entity;
                closestHit.distance = tMin;
                closestHit.point = ray.GetPoint(tMin);

                // Calculate normal (simplified)
                glm::vec3 localPoint = closestHit.point - aabb.GetCenter();
                glm::vec3 extents = aabb.GetExtents();
                glm::vec3 absLocal = glm::abs(localPoint / extents);

                if (absLocal.x > absLocal.y && absLocal.x > absLocal.z) {
                    closestHit.normal = glm::vec3(glm::sign(localPoint.x), 0, 0);
                } else if (absLocal.y > absLocal.z) {
                    closestHit.normal = glm::vec3(0, glm::sign(localPoint.y), 0);
                } else {
                    closestHit.normal = glm::vec3(0, 0, glm::sign(localPoint.z));
                }
            }
        }

        return closestHit;
    }

    std::vector<RaycastHit> PhysicsWorld::RaycastAll(const Ray& ray, float maxDistance, uint32_t layerMask) {
        std::vector<RaycastHit> hits;

        auto* transformPool = m_World->GetComponentPool<Transform>();
        auto* colliderPool = m_World->GetComponentPool<Collider>();

        if (!transformPool || !colliderPool) return hits;

        for (Entity entity : colliderPool->GetEntities()) {
            if (!m_World->HasComponent<Transform>(entity)) continue;

            const Transform& transform = m_World->GetComponent<Transform>(entity);
            const Collider& collider = m_World->GetComponent<Collider>(entity);

            if (!(layerMask & (1 << collider.layer))) continue;

            AABB aabb(
                collider.GetAABBMin(transform.position, transform.scale),
                collider.GetAABBMax(transform.position, transform.scale)
            );

            float tMin, tMax;
            if (TestRayAABB(ray, aabb, tMin, tMax) && tMin <= maxDistance && tMin >= 0) {
                RaycastHit hit;
                hit.hit = true;
                hit.entity = entity;
                hit.distance = tMin;
                hit.point = ray.GetPoint(tMin);
                hits.push_back(hit);
            }
        }

        std::sort(hits.begin(), hits.end(), [](const RaycastHit& a, const RaycastHit& b) {
            return a.distance < b.distance;
        });

        return hits;
    }

    std::vector<Entity> PhysicsWorld::OverlapSphere(const glm::vec3& center, float radius, uint32_t layerMask) {
        std::vector<Entity> result;
        BoundingSphere sphere(center, radius);

        auto* transformPool = m_World->GetComponentPool<Transform>();
        auto* colliderPool = m_World->GetComponentPool<Collider>();

        if (!transformPool || !colliderPool) return result;

        for (Entity entity : colliderPool->GetEntities()) {
            if (!m_World->HasComponent<Transform>(entity)) continue;

            const Transform& transform = m_World->GetComponent<Transform>(entity);
            const Collider& collider = m_World->GetComponent<Collider>(entity);

            if (!(layerMask & (1 << collider.layer))) continue;

            AABB aabb(
                collider.GetAABBMin(transform.position, transform.scale),
                collider.GetAABBMax(transform.position, transform.scale)
            );

            if (sphere.Intersects(aabb)) {
                result.push_back(entity);
            }
        }

        return result;
    }

    std::vector<Entity> PhysicsWorld::OverlapBox(const glm::vec3& center, const glm::vec3& halfExtents, uint32_t layerMask) {
        std::vector<Entity> result;
        AABB queryBox(center - halfExtents, center + halfExtents);

        auto* transformPool = m_World->GetComponentPool<Transform>();
        auto* colliderPool = m_World->GetComponentPool<Collider>();

        if (!transformPool || !colliderPool) return result;

        for (Entity entity : colliderPool->GetEntities()) {
            if (!m_World->HasComponent<Transform>(entity)) continue;

            const Transform& transform = m_World->GetComponent<Transform>(entity);
            const Collider& collider = m_World->GetComponent<Collider>(entity);

            if (!(layerMask & (1 << collider.layer))) continue;

            AABB aabb(
                collider.GetAABBMin(transform.position, transform.scale),
                collider.GetAABBMax(transform.position, transform.scale)
            );

            if (queryBox.Intersects(aabb)) {
                result.push_back(entity);
            }
        }

        return result;
    }

    bool PhysicsWorld::TestRayAABB(const Ray& ray, const AABB& aabb, float& tMin, float& tMax) {
        tMin = 0.0f;
        tMax = std::numeric_limits<float>::max();

        for (int i = 0; i < 3; i++) {
            float origin = ray.origin[i];
            float dir = ray.direction[i];
            float minB = aabb.min[i];
            float maxB = aabb.max[i];

            if (std::abs(dir) < 0.0001f) {
                if (origin < minB || origin > maxB) return false;
            } else {
                float t1 = (minB - origin) / dir;
                float t2 = (maxB - origin) / dir;

                if (t1 > t2) std::swap(t1, t2);

                tMin = glm::max(tMin, t1);
                tMax = glm::min(tMax, t2);

                if (tMin > tMax) return false;
            }
        }

        return true;
    }

    bool PhysicsWorld::TestRaySphere(const Ray& ray, const BoundingSphere& sphere, float& t) {
        glm::vec3 oc = ray.origin - sphere.center;
        float a = glm::dot(ray.direction, ray.direction);
        float b = 2.0f * glm::dot(oc, ray.direction);
        float c = glm::dot(oc, oc) - sphere.radius * sphere.radius;
        float discriminant = b * b - 4 * a * c;

        if (discriminant < 0) return false;

        t = (-b - std::sqrt(discriminant)) / (2.0f * a);
        return t >= 0;
    }

}
