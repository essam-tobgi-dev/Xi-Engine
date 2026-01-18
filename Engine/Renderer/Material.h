#pragma once

#include <memory>
#include <glm/glm.hpp>

namespace Xi {

    class Shader;
    class Texture;

    class Material {
    public:
        Material();
        ~Material();

        void SetShader(std::shared_ptr<Shader> shader);
        std::shared_ptr<Shader> GetShader() const { return m_Shader; }

        void SetAlbedoTexture(std::shared_ptr<Texture> texture) { m_AlbedoTexture = texture; }
        void SetNormalTexture(std::shared_ptr<Texture> texture) { m_NormalTexture = texture; }

        std::shared_ptr<Texture> GetAlbedoTexture() const { return m_AlbedoTexture; }
        std::shared_ptr<Texture> GetNormalTexture() const { return m_NormalTexture; }

        // Material properties
        glm::vec4 albedoColor = glm::vec4(1.0f);
        float metallic = 0.0f;
        float roughness = 0.5f;
        float ao = 1.0f;
        glm::vec3 emissive = glm::vec3(0.0f);

        // Rendering flags
        bool transparent = false;
        bool doubleSided = false;

        void Bind() const;
        void Unbind() const;

    private:
        std::shared_ptr<Shader> m_Shader;
        std::shared_ptr<Texture> m_AlbedoTexture;
        std::shared_ptr<Texture> m_NormalTexture;
    };

}
