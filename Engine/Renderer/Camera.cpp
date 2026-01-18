#include "Camera.h"

namespace Xi {

    Camera::Camera() {
        UpdateViewMatrix();
        UpdateProjectionMatrix();
    }

    void Camera::SetPerspective(float fov, float aspectRatio, float nearClip, float farClip) {
        m_ProjectionType = ProjectionType::Perspective;
        m_FOV = fov;
        m_AspectRatio = aspectRatio;
        m_NearClip = nearClip;
        m_FarClip = farClip;
        UpdateProjectionMatrix();
    }

    void Camera::SetOrthographic(float size, float aspectRatio, float nearClip, float farClip) {
        m_ProjectionType = ProjectionType::Orthographic;
        m_OrthoSize = size;
        m_AspectRatio = aspectRatio;
        m_NearClip = nearClip;
        m_FarClip = farClip;
        UpdateProjectionMatrix();
    }

    glm::vec3 Camera::GetForward() const {
        float pitch = glm::radians(m_Rotation.x);
        float yaw = glm::radians(m_Rotation.y);

        glm::vec3 forward;
        forward.x = cos(pitch) * sin(yaw);
        forward.y = -sin(pitch);
        forward.z = -cos(pitch) * cos(yaw);
        return glm::normalize(forward);
    }

    glm::vec3 Camera::GetRight() const {
        return glm::normalize(glm::cross(GetForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
    }

    glm::vec3 Camera::GetUp() const {
        return glm::normalize(glm::cross(GetRight(), GetForward()));
    }

    void Camera::UpdateViewMatrix() {
        glm::vec3 forward = GetForward();
        glm::vec3 target = m_Position + forward;
        m_ViewMatrix = glm::lookAt(m_Position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void Camera::UpdateProjectionMatrix() {
        if (m_ProjectionType == ProjectionType::Perspective) {
            m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
        } else {
            float halfWidth = m_OrthoSize * m_AspectRatio * 0.5f;
            float halfHeight = m_OrthoSize * 0.5f;
            m_ProjectionMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, m_NearClip, m_FarClip);
        }
    }

}
