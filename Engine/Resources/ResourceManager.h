#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace Xi {

    class Shader;
    class Texture;
    class Mesh;
    class Material;
    class AudioClip;

    class ResourceManager {
    public:
        static ResourceManager& Get();

        // Shaders
        std::shared_ptr<Shader> LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
        std::shared_ptr<Shader> GetShader(const std::string& name);

        // Textures
        std::shared_ptr<Texture> LoadTexture(const std::string& path);
        std::shared_ptr<Texture> GetTexture(const std::string& path);

        // Meshes
        std::shared_ptr<Mesh> GetMesh(const std::string& name);
        void RegisterMesh(const std::string& name, std::shared_ptr<Mesh> mesh);

        // Materials
        std::shared_ptr<Material> CreateMaterial(const std::string& name);
        std::shared_ptr<Material> GetMaterial(const std::string& name);

        // Audio
        std::shared_ptr<AudioClip> LoadAudioClip(const std::string& path);
        std::shared_ptr<AudioClip> GetAudioClip(const std::string& path);

        // Resource management
        void UnloadUnused();
        void Clear();

    private:
        ResourceManager() = default;
        ~ResourceManager() = default;

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

        std::unordered_map<std::string, std::shared_ptr<Shader>> m_Shaders;
        std::unordered_map<std::string, std::shared_ptr<Texture>> m_Textures;
        std::unordered_map<std::string, std::shared_ptr<Mesh>> m_Meshes;
        std::unordered_map<std::string, std::shared_ptr<Material>> m_Materials;
        std::unordered_map<std::string, std::shared_ptr<AudioClip>> m_AudioClips;
    };

}
