#include "Shader.h"
#include "../Core/Log.h"

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>

namespace Xi {

    Shader::Shader() = default;

    Shader::~Shader() {
        if (m_Program) {
            glDeleteProgram(m_Program);
        }
    }

    bool Shader::LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath) {
        std::string vertexSource = ReadFile(vertexPath);
        std::string fragmentSource = ReadFile(fragmentPath);

        if (vertexSource.empty() || fragmentSource.empty()) {
            return false;
        }

        return LoadFromSource(vertexSource, fragmentSource);
    }

    bool Shader::LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
        uint32_t vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
        uint32_t fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

        if (!vertexShader || !fragmentShader) {
            if (vertexShader) glDeleteShader(vertexShader);
            if (fragmentShader) glDeleteShader(fragmentShader);
            return false;
        }

        m_Program = glCreateProgram();
        glAttachShader(m_Program, vertexShader);
        glAttachShader(m_Program, fragmentShader);
        glLinkProgram(m_Program);

        int success;
        glGetProgramiv(m_Program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(m_Program, 512, nullptr, infoLog);
            XI_LOG_ERROR("Shader link error: " + std::string(infoLog));
            glDeleteProgram(m_Program);
            m_Program = 0;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return m_Program != 0;
    }

    void Shader::Bind() const {
        glUseProgram(m_Program);
    }

    void Shader::Unbind() const {
        glUseProgram(0);
    }

    void Shader::SetInt(const std::string& name, int value) {
        glUniform1i(GetUniformLocation(name), value);
    }

    void Shader::SetFloat(const std::string& name, float value) {
        glUniform1f(GetUniformLocation(name), value);
    }

    void Shader::SetVec2(const std::string& name, const glm::vec2& value) {
        glUniform2fv(GetUniformLocation(name), 1, glm::value_ptr(value));
    }

    void Shader::SetVec3(const std::string& name, const glm::vec3& value) {
        glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(value));
    }

    void Shader::SetVec4(const std::string& name, const glm::vec4& value) {
        glUniform4fv(GetUniformLocation(name), 1, glm::value_ptr(value));
    }

    void Shader::SetMat3(const std::string& name, const glm::mat3& value) {
        glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
    }

    void Shader::SetMat4(const std::string& name, const glm::mat4& value) {
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
    }

    uint32_t Shader::CompileShader(uint32_t type, const std::string& source) {
        uint32_t shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::string shaderType = (type == GL_VERTEX_SHADER) ? "Vertex" : "Fragment";
            XI_LOG_ERROR(shaderType + " shader compile error: " + std::string(infoLog));
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    int Shader::GetUniformLocation(const std::string& name) {
        auto it = m_UniformCache.find(name);
        if (it != m_UniformCache.end()) {
            return it->second;
        }

        int location = glGetUniformLocation(m_Program, name.c_str());
        m_UniformCache[name] = location;
        return location;
    }

    std::string Shader::ReadFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            XI_LOG_ERROR("Failed to open shader file: " + path);
            return "";
        }

        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

}
