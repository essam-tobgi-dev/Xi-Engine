#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Xi {

    struct Transform {
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f);  // Euler angles in degrees
        glm::vec3 scale = glm::vec3(1.0f);

        glm::mat4 GetMatrix() const {
            glm::mat4 mat = glm::mat4(1.0f);
            mat = glm::translate(mat, position);
            mat = glm::rotate(mat, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            mat = glm::rotate(mat, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            mat = glm::rotate(mat, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            mat = glm::scale(mat, scale);
            return mat;
        }

        glm::vec3 GetForward() const {
            float pitch = glm::radians(rotation.x);
            float yaw = glm::radians(rotation.y);

            glm::vec3 forward;
            forward.x = cos(pitch) * sin(yaw);
            forward.y = -sin(pitch);
            forward.z = -cos(pitch) * cos(yaw);
            return glm::normalize(forward);
        }

        glm::vec3 GetRight() const {
            return glm::normalize(glm::cross(GetForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
        }

        glm::vec3 GetUp() const {
            return glm::normalize(glm::cross(GetRight(), GetForward()));
        }

        glm::quat GetQuaternion() const {
            return glm::quat(glm::radians(rotation));
        }

        void SetFromQuaternion(const glm::quat& q) {
            rotation = glm::degrees(glm::eulerAngles(q));
        }
    };

}
