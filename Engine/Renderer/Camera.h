#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Xi {

    enum class ProjectionType {
        Perspective,
        Orthographic
    };

    class Camera {
    public:
        Camera();

        void SetPerspective(float fov, float aspectRatio, float nearClip, float farClip);
        void SetOrthographic(float size, float aspectRatio, float nearClip, float farClip);

        void SetPosition(const glm::vec3& position) { m_Position = position; UpdateViewMatrix(); }
        void SetRotation(const glm::vec3& rotation) { m_Rotation = rotation; UpdateViewMatrix(); }

        const glm::vec3& GetPosition() const { return m_Position; }
        const glm::vec3& GetRotation() const { return m_Rotation; }

        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
        glm::mat4 GetViewProjectionMatrix() const { return m_ProjectionMatrix * m_ViewMatrix; }

        glm::vec3 GetForward() const;
        glm::vec3 GetRight() const;
        glm::vec3 GetUp() const;

        float GetFOV() const { return m_FOV; }
        float GetAspectRatio() const { return m_AspectRatio; }
        float GetNearClip() const { return m_NearClip; }
        float GetFarClip() const { return m_FarClip; }
        float GetOrthographicSize() const { return m_OrthoSize; }

        void SetFOV(float fov) { m_FOV = fov; UpdateProjectionMatrix(); }
        void SetAspectRatio(float ratio) { m_AspectRatio = ratio; UpdateProjectionMatrix(); }
        void SetNearClip(float nearClip) { m_NearClip = nearClip; UpdateProjectionMatrix(); }
        void SetFarClip(float farClip) { m_FarClip = farClip; UpdateProjectionMatrix(); }
        void SetOrthographicSize(float size) { m_OrthoSize = size; UpdateProjectionMatrix(); }

        ProjectionType GetProjectionType() const { return m_ProjectionType; }
        void SetProjectionType(ProjectionType type) { m_ProjectionType = type; UpdateProjectionMatrix(); }

    private:
        void UpdateViewMatrix();
        void UpdateProjectionMatrix();

        glm::vec3 m_Position = glm::vec3(0.0f, 0.0f, 5.0f);
        glm::vec3 m_Rotation = glm::vec3(0.0f); // Pitch, Yaw, Roll in degrees

        glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
        glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);

        ProjectionType m_ProjectionType = ProjectionType::Perspective;

        float m_FOV = 45.0f;
        float m_AspectRatio = 16.0f / 9.0f;
        float m_NearClip = 0.1f;
        float m_FarClip = 1000.0f;
        float m_OrthoSize = 10.0f;
    };

}
