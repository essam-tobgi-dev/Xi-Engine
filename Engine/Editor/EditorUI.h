#pragma once

#include "SceneHierarchy.h"
#include "Inspector.h"
#include "Console.h"
#include "../Renderer/Camera.h"
#include <memory>

struct GLFWwindow;

namespace Xi {

    class World;

    class EditorUI {
    public:
        EditorUI();
        ~EditorUI();

        bool Init(GLFWwindow* window);
        void Shutdown();

        void BeginFrame();
        void EndFrame();

        void Render(World& world);

        Camera& GetEditorCamera() { return m_EditorCamera; }

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
