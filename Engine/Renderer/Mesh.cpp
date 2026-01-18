#include "Mesh.h"
#include <GL/glew.h>

namespace Xi {

    Mesh::Mesh() = default;

    Mesh::~Mesh() {
        if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
        if (m_VBO) glDeleteBuffers(1, &m_VBO);
        if (m_EBO) glDeleteBuffers(1, &m_EBO);
    }

    void Mesh::SetVertices(const std::vector<Vertex>& vertices) {
        m_Vertices = vertices;
    }

    void Mesh::SetIndices(const std::vector<uint32_t>& indices) {
        m_Indices = indices;
    }

    void Mesh::Build() {
        if (m_Vertices.empty()) return;

        CalculateTangents();

        // Clean up existing buffers
        if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
        if (m_VBO) glDeleteBuffers(1, &m_VBO);
        if (m_EBO) glDeleteBuffers(1, &m_EBO);

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), m_Vertices.data(), GL_STATIC_DRAW);

        if (!m_Indices.empty()) {
            glGenBuffers(1, &m_EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(uint32_t), m_Indices.data(), GL_STATIC_DRAW);
        }

        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

        // Normal attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

        // TexCoord attribute
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

        // Tangent attribute
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

        glBindVertexArray(0);
    }

    void Mesh::Bind() const {
        glBindVertexArray(m_VAO);
    }

    void Mesh::Unbind() const {
        glBindVertexArray(0);
    }

    void Mesh::Draw() const {
        glBindVertexArray(m_VAO);
        if (!m_Indices.empty()) {
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_Indices.size()), GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_Vertices.size()));
        }
        glBindVertexArray(0);
    }

    void Mesh::CalculateTangents() {
        if (m_Indices.empty()) return;

        // Reset tangents
        for (auto& v : m_Vertices) {
            v.tangent = glm::vec3(0.0f);
        }

        // Calculate tangents for each triangle
        for (size_t i = 0; i < m_Indices.size(); i += 3) {
            uint32_t i0 = m_Indices[i];
            uint32_t i1 = m_Indices[i + 1];
            uint32_t i2 = m_Indices[i + 2];

            Vertex& v0 = m_Vertices[i0];
            Vertex& v1 = m_Vertices[i1];
            Vertex& v2 = m_Vertices[i2];

            glm::vec3 edge1 = v1.position - v0.position;
            glm::vec3 edge2 = v2.position - v0.position;
            glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
            glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y + 0.0001f);

            glm::vec3 tangent;
            tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
            tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
            tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

            v0.tangent += tangent;
            v1.tangent += tangent;
            v2.tangent += tangent;
        }

        // Normalize tangents
        for (auto& v : m_Vertices) {
            if (glm::length(v.tangent) > 0.0001f) {
                v.tangent = glm::normalize(v.tangent);
            } else {
                v.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
            }
        }
    }

}
