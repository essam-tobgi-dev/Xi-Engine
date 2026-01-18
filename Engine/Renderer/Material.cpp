#include "Material.h"
#include "Shader.h"
#include "Texture.h"

#include <GL/glew.h>

namespace Xi {

    Material::Material() = default;
    Material::~Material() = default;

    void Material::SetShader(std::shared_ptr<Shader> shader) {
        m_Shader = shader;
    }

    void Material::Bind() const {
        if (m_Shader) {
            m_Shader->Bind();

            m_Shader->SetVec4("u_AlbedoColor", albedoColor);
            m_Shader->SetFloat("u_Metallic", metallic);
            m_Shader->SetFloat("u_Roughness", roughness);
            m_Shader->SetFloat("u_AO", ao);
            m_Shader->SetVec3("u_Emissive", emissive);

            int textureSlot = 0;

            if (m_AlbedoTexture) {
                m_AlbedoTexture->Bind(textureSlot);
                m_Shader->SetInt("u_AlbedoMap", textureSlot);
                m_Shader->SetInt("u_HasAlbedoMap", 1);
                textureSlot++;
            } else {
                m_Shader->SetInt("u_HasAlbedoMap", 0);
            }

            if (m_NormalTexture) {
                m_NormalTexture->Bind(textureSlot);
                m_Shader->SetInt("u_NormalMap", textureSlot);
                m_Shader->SetInt("u_HasNormalMap", 1);
                textureSlot++;
            } else {
                m_Shader->SetInt("u_HasNormalMap", 0);
            }
        }

        if (doubleSided) {
            glDisable(GL_CULL_FACE);
        } else {
            glEnable(GL_CULL_FACE);
        }

        if (transparent) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }

    void Material::Unbind() const {
        if (transparent) {
            glDisable(GL_BLEND);
        }
        glEnable(GL_CULL_FACE);

        if (m_Shader) {
            m_Shader->Unbind();
        }
    }

}
