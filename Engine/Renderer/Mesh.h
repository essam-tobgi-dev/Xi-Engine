#pragma once

#include <vector>
#include <cstdint>
#include <glm/glm.hpp>

namespace Xi {

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
        glm::vec3 tangent;

        Vertex() : position(0.0f), normal(0.0f, 1.0f, 0.0f), texCoord(0.0f), tangent(1.0f, 0.0f, 0.0f) {}

        Vertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& uv)
            : position(pos), normal(norm), texCoord(uv), tangent(1.0f, 0.0f, 0.0f) {}
    };

    class Mesh {
    public:
        Mesh();
        ~Mesh();

        void SetVertices(const std::vector<Vertex>& vertices);
        void SetIndices(const std::vector<uint32_t>& indices);
        void Build();

        void Bind() const;
        void Unbind() const;
        void Draw() const;

        uint32_t GetVertexCount() const { return static_cast<uint32_t>(m_Vertices.size()); }
        uint32_t GetIndexCount() const { return static_cast<uint32_t>(m_Indices.size()); }
        bool IsValid() const { return m_VAO != 0; }

        const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
        const std::vector<uint32_t>& GetIndices() const { return m_Indices; }

    private:
        void CalculateTangents();

        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;

        uint32_t m_VAO = 0;
        uint32_t m_VBO = 0;
        uint32_t m_EBO = 0;
    };

}
