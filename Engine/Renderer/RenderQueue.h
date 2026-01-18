#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace Xi {

    class Mesh;
    class Material;

    struct RenderCommand {
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<Material> material;
        glm::mat4 transform;
        float distanceToCamera = 0.0f;
        bool transparent = false;
    };

    class RenderQueue {
    public:
        void Clear();

        void Submit(const RenderCommand& command);
        void Sort(const glm::vec3& cameraPosition);

        const std::vector<RenderCommand>& GetOpaqueCommands() const { return m_OpaqueCommands; }
        const std::vector<RenderCommand>& GetTransparentCommands() const { return m_TransparentCommands; }

        size_t GetOpaqueCount() const { return m_OpaqueCommands.size(); }
        size_t GetTransparentCount() const { return m_TransparentCommands.size(); }
        size_t GetTotalCount() const { return m_OpaqueCommands.size() + m_TransparentCommands.size(); }

    private:
        std::vector<RenderCommand> m_OpaqueCommands;
        std::vector<RenderCommand> m_TransparentCommands;
    };

}
