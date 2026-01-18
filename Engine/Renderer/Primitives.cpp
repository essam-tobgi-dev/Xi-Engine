#include "Primitives.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>

namespace Xi {

    std::shared_ptr<Mesh> Primitives::CreateCube() {
        auto mesh = std::make_shared<Mesh>();

        std::vector<Vertex> vertices = {
            // Front face
            {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

            // Back face
            {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},

            // Top face
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},

            // Bottom face
            {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},

            // Right face
            {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

            // Left face
            {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        };

        std::vector<uint32_t> indices = {
            0,  1,  2,  2,  3,  0,   // Front
            4,  5,  6,  6,  7,  4,   // Back
            8,  9,  10, 10, 11, 8,   // Top
            12, 13, 14, 14, 15, 12,  // Bottom
            16, 17, 18, 18, 19, 16,  // Right
            20, 21, 22, 22, 23, 20   // Left
        };

        mesh->SetVertices(vertices);
        mesh->SetIndices(indices);
        mesh->Build();

        return mesh;
    }

    std::shared_ptr<Mesh> Primitives::CreateSphere(int segments, int rings) {
        auto mesh = std::make_shared<Mesh>();
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        for (int y = 0; y <= rings; y++) {
            for (int x = 0; x <= segments; x++) {
                float xSegment = static_cast<float>(x) / static_cast<float>(segments);
                float ySegment = static_cast<float>(y) / static_cast<float>(rings);

                float xPos = std::cos(xSegment * 2.0f * glm::pi<float>()) * std::sin(ySegment * glm::pi<float>());
                float yPos = std::cos(ySegment * glm::pi<float>());
                float zPos = std::sin(xSegment * 2.0f * glm::pi<float>()) * std::sin(ySegment * glm::pi<float>());

                Vertex vertex;
                vertex.position = glm::vec3(xPos, yPos, zPos) * 0.5f;
                vertex.normal = glm::normalize(glm::vec3(xPos, yPos, zPos));
                vertex.texCoord = glm::vec2(xSegment, ySegment);
                vertices.push_back(vertex);
            }
        }

        for (int y = 0; y < rings; y++) {
            for (int x = 0; x < segments; x++) {
                uint32_t i0 = y * (segments + 1) + x;
                uint32_t i1 = i0 + 1;
                uint32_t i2 = (y + 1) * (segments + 1) + x;
                uint32_t i3 = i2 + 1;

                indices.push_back(i0);
                indices.push_back(i2);
                indices.push_back(i1);

                indices.push_back(i1);
                indices.push_back(i2);
                indices.push_back(i3);
            }
        }

        mesh->SetVertices(vertices);
        mesh->SetIndices(indices);
        mesh->Build();

        return mesh;
    }

    std::shared_ptr<Mesh> Primitives::CreatePlane(float size) {
        auto mesh = std::make_shared<Mesh>();

        float half = size * 0.5f;
        std::vector<Vertex> vertices = {
            {{-half, 0.0f, -half}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{ half, 0.0f, -half}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{ half, 0.0f,  half}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-half, 0.0f,  half}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        };

        std::vector<uint32_t> indices = {0, 2, 1, 0, 3, 2};

        mesh->SetVertices(vertices);
        mesh->SetIndices(indices);
        mesh->Build();

        return mesh;
    }

    std::shared_ptr<Mesh> Primitives::CreateQuad() {
        auto mesh = std::make_shared<Mesh>();

        std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        };

        std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

        mesh->SetVertices(vertices);
        mesh->SetIndices(indices);
        mesh->Build();

        return mesh;
    }

    std::shared_ptr<Mesh> Primitives::CreateCylinder(int segments) {
        auto mesh = std::make_shared<Mesh>();
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        float height = 1.0f;
        float radius = 0.5f;

        // Side vertices
        for (int i = 0; i <= segments; i++) {
            float angle = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * glm::pi<float>();
            float x = std::cos(angle) * radius;
            float z = std::sin(angle) * radius;

            glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0f, z));
            float u = static_cast<float>(i) / static_cast<float>(segments);

            // Bottom vertex
            vertices.push_back({{x, -height * 0.5f, z}, normal, {u, 0.0f}});
            // Top vertex
            vertices.push_back({{x, height * 0.5f, z}, normal, {u, 1.0f}});
        }

        // Side indices
        for (int i = 0; i < segments; i++) {
            uint32_t i0 = i * 2;
            uint32_t i1 = i0 + 1;
            uint32_t i2 = i0 + 2;
            uint32_t i3 = i0 + 3;

            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i1);

            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }

        // Top cap
        uint32_t topCenterIdx = static_cast<uint32_t>(vertices.size());
        vertices.push_back({{0.0f, height * 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 0.5f}});

        for (int i = 0; i <= segments; i++) {
            float angle = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * glm::pi<float>();
            float x = std::cos(angle) * radius;
            float z = std::sin(angle) * radius;
            vertices.push_back({{x, height * 0.5f, z}, {0.0f, 1.0f, 0.0f}, {(x / radius + 1.0f) * 0.5f, (z / radius + 1.0f) * 0.5f}});
        }

        for (int i = 0; i < segments; i++) {
            indices.push_back(topCenterIdx);
            indices.push_back(topCenterIdx + i + 1);
            indices.push_back(topCenterIdx + i + 2);
        }

        // Bottom cap
        uint32_t bottomCenterIdx = static_cast<uint32_t>(vertices.size());
        vertices.push_back({{0.0f, -height * 0.5f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.5f}});

        for (int i = 0; i <= segments; i++) {
            float angle = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * glm::pi<float>();
            float x = std::cos(angle) * radius;
            float z = std::sin(angle) * radius;
            vertices.push_back({{x, -height * 0.5f, z}, {0.0f, -1.0f, 0.0f}, {(x / radius + 1.0f) * 0.5f, (z / radius + 1.0f) * 0.5f}});
        }

        for (int i = 0; i < segments; i++) {
            indices.push_back(bottomCenterIdx);
            indices.push_back(bottomCenterIdx + i + 2);
            indices.push_back(bottomCenterIdx + i + 1);
        }

        mesh->SetVertices(vertices);
        mesh->SetIndices(indices);
        mesh->Build();

        return mesh;
    }

    std::shared_ptr<Mesh> Primitives::CreateCone(int segments) {
        auto mesh = std::make_shared<Mesh>();
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        float height = 1.0f;
        float radius = 0.5f;

        // Apex
        uint32_t apexIdx = 0;
        vertices.push_back({{0.0f, height * 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 1.0f}});

        // Side vertices
        for (int i = 0; i <= segments; i++) {
            float angle = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * glm::pi<float>();
            float x = std::cos(angle) * radius;
            float z = std::sin(angle) * radius;

            // Normal points outward and upward
            glm::vec3 normal = glm::normalize(glm::vec3(x, radius / height, z));
            float u = static_cast<float>(i) / static_cast<float>(segments);

            vertices.push_back({{x, -height * 0.5f, z}, normal, {u, 0.0f}});
        }

        // Side indices
        for (int i = 0; i < segments; i++) {
            indices.push_back(apexIdx);
            indices.push_back(1 + i);
            indices.push_back(1 + i + 1);
        }

        // Bottom cap
        uint32_t bottomCenterIdx = static_cast<uint32_t>(vertices.size());
        vertices.push_back({{0.0f, -height * 0.5f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.5f}});

        for (int i = 0; i <= segments; i++) {
            float angle = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * glm::pi<float>();
            float x = std::cos(angle) * radius;
            float z = std::sin(angle) * radius;
            vertices.push_back({{x, -height * 0.5f, z}, {0.0f, -1.0f, 0.0f}, {(x / radius + 1.0f) * 0.5f, (z / radius + 1.0f) * 0.5f}});
        }

        for (int i = 0; i < segments; i++) {
            indices.push_back(bottomCenterIdx);
            indices.push_back(bottomCenterIdx + i + 2);
            indices.push_back(bottomCenterIdx + i + 1);
        }

        mesh->SetVertices(vertices);
        mesh->SetIndices(indices);
        mesh->Build();

        return mesh;
    }

}
