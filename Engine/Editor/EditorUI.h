#pragma once

#include "SceneHierarchy.h"
#include "Inspector.h"
#include "Console.h"
#include "../Renderer/Camera.h"
#include "../Renderer/Framebuffer.h"
#include <memory>
#include <glm/glm.hpp>

struct GLFWwindow;

namespace Xi {

    class World;
    class Renderer;

    class EditorUI {
    public:
        EditorUI();
        ~EditorUI();

        bool Init(GLFWwindow* window);
        void Shutdown();

        void BeginFrame();
        void EndFrame();

        void Render(World& world, Renderer& renderer);

        void BeginSceneRender();
        void EndSceneRender();

        void UpdateSceneViewport();  // Call before rendering to handle resize

        Camera& GetEditorCamera() { return m_EditorCamera; }
        Framebuffer* GetSceneFramebuffer() { return m_SceneFramebuffer.get(); }

        glm::vec2 GetSceneViewportSize() const { return m_SceneViewportSize; }

    private:
        void SetupImGuiStyle();
        void DrawMenuBar(World& world);
        void DrawToolbar();
        void DrawStats();
        void UpdateEditorCamera(float dt);

        SceneHierarchy m_Hierarchy;
        Inspector m_Inspector;
        Console m_Console;

        Camera m_EditorCamera;
        std::unique_ptr<Framebuffer> m_SceneFramebuffer;
        glm::vec2 m_SceneViewportSize = { 1280.0f, 720.0f };

        bool m_ShowHierarchy = true;
        bool m_ShowInspector = true;
        bool m_ShowConsole = true;
        bool m_ShowStats = true;
        bool m_ShowDemo = false;

        // Editor camera control
        bool m_CameraActive = false;
        float m_CameraMoveSpeed = 10.0f;
        float m_CameraLookSpeed = 0.1f;
    };

}
