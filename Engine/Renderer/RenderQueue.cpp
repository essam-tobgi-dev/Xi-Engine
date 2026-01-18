#include "RenderQueue.h"
#include <algorithm>

namespace Xi {

    void RenderQueue::Clear() {
        m_OpaqueCommands.clear();
        m_TransparentCommands.clear();
    }

    void RenderQueue::Submit(const RenderCommand& command) {
        if (command.transparent) {
            m_TransparentCommands.push_back(command);
        } else {
            m_OpaqueCommands.push_back(command);
        }
    }

    void RenderQueue::Sort(const glm::vec3& cameraPosition) {
        // Calculate distances
        auto calculateDistance = [&cameraPosition](RenderCommand& cmd) {
            glm::vec3 position = glm::vec3(cmd.transform[3]);
            cmd.distanceToCamera = glm::length(position - cameraPosition);
        };

        for (auto& cmd : m_OpaqueCommands) {
            calculateDistance(cmd);
        }
        for (auto& cmd : m_TransparentCommands) {
            calculateDistance(cmd);
        }

        // Sort opaque: front-to-back (minimize overdraw)
        std::sort(m_OpaqueCommands.begin(), m_OpaqueCommands.end(),
            [](const RenderCommand& a, const RenderCommand& b) {
                return a.distanceToCamera < b.distanceToCamera;
            });

        // Sort transparent: back-to-front (correct blending)
        std::sort(m_TransparentCommands.begin(), m_TransparentCommands.end(),
            [](const RenderCommand& a, const RenderCommand& b) {
                return a.distanceToCamera > b.distanceToCamera;
            });
    }

}
