#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace Xi {

    class Shader {
    public:
        Shader();
        ~Shader();

        bool LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
        bool LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource);

        void Bind() const;
        void Unbind() const;

        uint32_t GetProgram() const { return m_Program; }
        bool IsValid() const { return m_Program != 0; }

        // Uniform setters
        void SetInt(const std::string& name, int value);
        void SetFloat(const std::string& name, float value);
        void SetVec2(const std::string& name, const glm::vec2& value);
        void SetVec3(const std::string& name, const glm::vec3& value);
        void SetVec4(const std::string& name, const glm::vec4& value);
        void SetMat3(const std::string& name, const glm::mat3& value);
        void SetMat4(const std::string& name, const glm::mat4& value);

    private:
        uint32_t CompileShader(uint32_t type, const std::string& source);
        int GetUniformLocation(const std::string& name);
        std::string ReadFile(const std::string& path);

        uint32_t m_Program = 0;
        mutable std::unordered_map<std::string, int> m_UniformCache;
    };

}
