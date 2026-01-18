#pragma once

#include "Window.h"
#include <memory>

namespace Xi {

    class World;
    class Renderer;
    class PhysicsWorld;
    class AudioEngine;
    class EditorUI;

    class Application {
    public:
        Application(const WindowProps& props = WindowProps());
        virtual ~Application();

        void Run();
        void Quit();

        Window& GetWindow() { return *m_Window; }
        World& GetWorld() { return *m_World; }
        Renderer& GetRenderer() { return *m_Renderer; }
        PhysicsWorld& GetPhysics() { return *m_Physics; }
        AudioEngine& GetAudio() { return *m_Audio; }

        static Application& Get() { return *s_Instance; }

        bool IsEditorMode() const { return m_EditorMode; }
        void SetEditorMode(bool enabled) { m_EditorMode = enabled; }

    protected:
        virtual void OnInit() {}
        virtual void OnUpdate(float dt) { (void)dt; }
        virtual void OnFixedUpdate(float dt) { (void)dt; }
        virtual void OnRender() {}
        virtual void OnImGui() {}
        virtual void OnShutdown() {}

    private:
        void Init();
        void MainLoop();
        void Shutdown();

        std::unique_ptr<Window> m_Window;
        std::unique_ptr<World> m_World;
        std::unique_ptr<Renderer> m_Renderer;
        std::unique_ptr<PhysicsWorld> m_Physics;
        std::unique_ptr<AudioEngine> m_Audio;
        std::unique_ptr<EditorUI> m_Editor;

        bool m_Running = true;
        bool m_EditorMode = true;

        static Application* s_Instance;
    };

}
