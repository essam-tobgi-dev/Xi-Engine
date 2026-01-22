#pragma once

#include "SceneHierarchy.h"
#include "Inspector.h"
#include "Console.h"
#include "ScriptEditor.h"
#include "../Renderer/Camera.h"
#include "../Renderer/Framebuffer.h"
#include <memory>
#include <glm/glm.hpp>

struct GLFWwindow;

namespace Xi {

    class World;
    class Renderer;
    class ScriptSystem;
    class ScriptEngine;

    class EditorUI {
    public:
        EditorUI();
        ~EditorUI();

        bool Init(GLFWwindow* window);
        void Shutdown();

        void BeginFrame();
        void EndFrame();

        void Render(World& world, Renderer& renderer, ScriptSystem* scriptSystem, ScriptEngine* engine);

        void BeginSceneRender();
        void EndSceneRender();

        void UpdateSceneViewport();  // Call before rendering to handle resize

        Camera& GetEditorCamera() { return m_EditorCamera; }
        Framebuffer* GetSceneFramebuffer() { return m_SceneFramebuffer.get(); }

        glm::vec2 GetSceneViewportSize() const { return m_SceneViewportSize; }

        // Play mode control
        bool IsPlayMode() const { return m_PlayMode; }
        void SetPlayMode(bool play) { m_PlayMode = play; }

        // Access to script editor
        ScriptEditor& GetScriptEditor() { return m_ScriptEditor; }

    private:
        void SetupImGuiStyle();
        void DrawMenuBar(World& world);
        void DrawToolbar(World& world, ScriptSystem* scriptSystem);
        void DrawStats();
        void UpdateEditorCamera(float dt);

        SceneHierarchy m_Hierarchy;
        Inspector m_Inspector;
        Console m_Console;
        ScriptEditor m_ScriptEditor;

        Camera m_EditorCamera;
        std::unique_ptr<Framebuffer> m_SceneFramebuffer;
        glm::vec2 m_SceneViewportSize = { 1280.0f, 720.0f };

        bool m_ShowHierarchy = true;
        bool m_ShowInspector = true;
        bool m_ShowConsole = true;
        bool m_ShowStats = true;
        bool m_ShowScriptEditor = false;
        bool m_ShowDemo = false;

        bool m_PlayMode = false;

        // Editor camera control
        bool m_CameraActive = false;
        float m_CameraMoveSpeed = 10.0f;
        float m_CameraLookSpeed = 0.1f;
    };

}
