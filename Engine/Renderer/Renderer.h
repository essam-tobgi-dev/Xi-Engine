#pragma once

#include "RenderQueue.h"
#include "Camera.h"
#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace Xi {

    class Shader;
    class Mesh;
    class Material;

    struct LightData {
        enum class Type { Directional, Point, Spot };

        Type type = Type::Directional;
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 color = glm::vec3(1.0f);
        float intensity = 1.0f;
        float range = 10.0f;
        float spotAngle = 45.0f;
    };

    class Renderer {
    public:
        Renderer();
        ~Renderer();

        void Init();
        void Shutdown();

        void BeginFrame();
        void EndFrame();

        void SetCamera(const Camera& camera);
        const Camera& GetCamera() const { return m_Camera; }
        Camera& GetCamera() { return m_Camera; }

        void Submit(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, const glm::mat4& transform);

        void AddLight(const LightData& light);
        void ClearLights();

        // Immediate mode drawing
        void DrawMesh(Mesh& mesh, Material& material, const glm::mat4& transform);

        // Default resources
        std::shared_ptr<Shader> GetDefaultShader() const { return m_DefaultShader; }
        std::shared_ptr<Shader> GetUnlitShader() const { return m_UnlitShader; }
        std::shared_ptr<Shader> GetSpriteShader() const { return m_SpriteShader; }

        // Statistics
        struct Stats {
            uint32_t drawCalls = 0;
            uint32_t triangles = 0;
        };
        const Stats& GetStats() const { return m_Stats; }
        void ResetStats();

    private:
        void CreateDefaultShaders();
        void SetupLightUniforms(Shader& shader);

        Camera m_Camera;
        RenderQueue m_RenderQueue;
        std::vector<LightData> m_Lights;

        std::shared_ptr<Shader> m_DefaultShader;
        std::shared_ptr<Shader> m_UnlitShader;
        std::shared_ptr<Shader> m_SpriteShader;

        Stats m_Stats;
    };

}
