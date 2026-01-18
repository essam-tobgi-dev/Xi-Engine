#pragma once

#include <glm/glm.hpp>

namespace Xi {

    enum class RigidBodyType {
        Static,
        Kinematic,
        Dynamic
    };

    struct RigidBody {
        RigidBodyType type = RigidBodyType::Dynamic;

        float mass = 1.0f;
        float drag = 0.0f;
        float angularDrag = 0.05f;

        glm::vec3 velocity = glm::vec3(0.0f);
        glm::vec3 angularVelocity = glm::vec3(0.0f);

        bool useGravity = true;
        glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);

        // Constraints
        bool freezePositionX = false;
        bool freezePositionY = false;
        bool freezePositionZ = false;
        bool freezeRotationX = false;
        bool freezeRotationY = false;
        bool freezeRotationZ = false;

        // Physics material properties
        float friction = 0.5f;
        float bounciness = 0.0f;

        // Forces to apply (cleared each frame)
        glm::vec3 force = glm::vec3(0.0f);
        glm::vec3 torque = glm::vec3(0.0f);

        void AddForce(const glm::vec3& f) {
            force += f;
        }

        void AddImpulse(const glm::vec3& impulse) {
            if (mass > 0.0f) {
                velocity += impulse / mass;
            }
        }

        void AddTorque(const glm::vec3& t) {
            torque += t;
        }
    };

}
