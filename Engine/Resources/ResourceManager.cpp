#include "ResourceManager.h"
#include "../Renderer/Shader.h"
#include "../Renderer/Texture.h"
#include "../Renderer/Mesh.h"
#include "../Renderer/Material.h"
#include "../Audio/AudioClip.h"
#include "../Core/Log.h"

namespace Xi {

    ResourceManager& ResourceManager::Get() {
        static ResourceManager instance;
        return instance;
    }

    std::shared_ptr<Shader> ResourceManager::LoadShader(const std::string& name,
        const std::string& vertexPath, const std::string& fragmentPath) {

        auto it = m_Shaders.find(name);
        if (it != m_Shaders.end()) {
            return it->second;
        }

        auto shader = std::make_shared<Shader>();
        if (shader->LoadFromFile(vertexPath, fragmentPath)) {
            m_Shaders[name] = shader;
            XI_LOG_INFO("Shader loaded: " + name);
            return shader;
        }

        XI_LOG_ERROR("Failed to load shader: " + name);
        return nullptr;
    }

    std::shared_ptr<Shader> ResourceManager::GetShader(const std::string& name) {
        auto it = m_Shaders.find(name);
        return it != m_Shaders.end() ? it->second : nullptr;
    }

    std::shared_ptr<Texture> ResourceManager::LoadTexture(const std::string& path) {
        auto it = m_Textures.find(path);
        if (it != m_Textures.end()) {
            return it->second;
        }

        auto texture = std::make_shared<Texture>();
        if (texture->LoadFromFile(path)) {
            m_Textures[path] = texture;
            return texture;
        }

        XI_LOG_ERROR("Failed to load texture: " + path);
        return nullptr;
    }

    std::shared_ptr<Texture> ResourceManager::GetTexture(const std::string& path) {
        auto it = m_Textures.find(path);
        return it != m_Textures.end() ? it->second : nullptr;
    }

    std::shared_ptr<Mesh> ResourceManager::GetMesh(const std::string& name) {
        auto it = m_Meshes.find(name);
        return it != m_Meshes.end() ? it->second : nullptr;
    }

    void ResourceManager::RegisterMesh(const std::string& name, std::shared_ptr<Mesh> mesh) {
        m_Meshes[name] = mesh;
    }

    std::shared_ptr<Material> ResourceManager::CreateMaterial(const std::string& name) {
        auto it = m_Materials.find(name);
        if (it != m_Materials.end()) {
            return it->second;
        }

        auto material = std::make_shared<Material>();
        m_Materials[name] = material;
        return material;
    }

    std::shared_ptr<Material> ResourceManager::GetMaterial(const std::string& name) {
        auto it = m_Materials.find(name);
        return it != m_Materials.end() ? it->second : nullptr;
    }

    std::shared_ptr<AudioClip> ResourceManager::LoadAudioClip(const std::string& path) {
        auto it = m_AudioClips.find(path);
        if (it != m_AudioClips.end()) {
            return it->second;
        }

        auto clip = std::make_shared<AudioClip>();
        if (clip->LoadFromFile(path)) {
            m_AudioClips[path] = clip;
            return clip;
        }

        XI_LOG_ERROR("Failed to load audio clip: " + path);
        return nullptr;
    }

    std::shared_ptr<AudioClip> ResourceManager::GetAudioClip(const std::string& path) {
        auto it = m_AudioClips.find(path);
        return it != m_AudioClips.end() ? it->second : nullptr;
    }

    void ResourceManager::UnloadUnused() {
        // Remove resources with only one reference (held by manager)
        for (auto it = m_Textures.begin(); it != m_Textures.end();) {
            if (it->second.use_count() == 1) {
                it = m_Textures.erase(it);
            } else {
                ++it;
            }
        }

        for (auto it = m_Meshes.begin(); it != m_Meshes.end();) {
            if (it->second.use_count() == 1) {
                it = m_Meshes.erase(it);
            } else {
                ++it;
            }
        }

        for (auto it = m_Materials.begin(); it != m_Materials.end();) {
            if (it->second.use_count() == 1) {
                it = m_Materials.erase(it);
            } else {
                ++it;
            }
        }

        for (auto it = m_AudioClips.begin(); it != m_AudioClips.end();) {
            if (it->second.use_count() == 1) {
                it = m_AudioClips.erase(it);
            } else {
                ++it;
            }
        }
    }

    void ResourceManager::Clear() {
        m_Shaders.clear();
        m_Textures.clear();
        m_Meshes.clear();
        m_Materials.clear();
        m_AudioClips.clear();
    }

}
