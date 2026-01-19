#include "Application.h"
#include "Log.h"
#include "Time.h"
#include "Input.h"

#include "../ECS/World.h"
#include "../Renderer/Renderer.h"
#include "../Physics/PhysicsWorld.h"
#include "../Audio/AudioEngine.h"
#include "../Editor/EditorUI.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace Xi {

    Application* Application::s_Instance = nullptr;

    Application::Application(const WindowProps& props) {
        s_Instance = this;
        m_Window = std::make_unique<Window>(props);
    }

    Application::~Application() {
        s_Instance = nullptr;
    }

    void Application::Run() {
        Init();
        MainLoop();
        Shutdown();
    }

    void Application::Quit() {
        m_Running = false;
    }

    void Application::Init() {
        Log::Init();
        XI_LOG_INFO("Xi Engine initializing...");

        if (!m_Window->Init()) {
            XI_LOG_ERROR("Failed to initialize window");
            m_Running = false;
            return;
        }

        Time::Init();
        Input::Init(m_Window->GetNativeWindow());

        // Initialize subsystems
        m_World = std::make_unique<World>();
        m_Renderer = std::make_unique<Renderer>();
        m_Physics = std::make_unique<PhysicsWorld>();
        m_Audio = std::make_unique<AudioEngine>();

        m_Renderer->Init();
        m_Audio->Init();

        if (m_EditorMode) {
            m_Editor = std::make_unique<EditorUI>();
            m_Editor->Init(m_Window->GetNativeWindow());
        }

        // Register default systems
        m_World->RegisterDefaultSystems(*m_Renderer, *m_Physics);

        OnInit();

        XI_LOG_INFO("Xi Engine initialized successfully");
    }

    void Application::MainLoop() {
        while (m_Running && !m_Window->ShouldClose()) {
            Time::Update();
            float dt = Time::GetDeltaTime();

            m_Window->PollEvents();
            Input::Update();

            // Fixed timestep physics updates
            while (Time::ShouldRunFixedUpdate()) {
                float fixedDt = Time::GetFixedDeltaTime();
                OnFixedUpdate(fixedDt);
                m_Physics->Step(fixedDt);
                Time::ConsumeAccumulator(fixedDt);
            }

            // Variable timestep update
            OnUpdate(dt);
            m_World->Update(dt);

            // Editor UI
            if (m_EditorMode && m_Editor) {
                // Handle framebuffer resize BEFORE rendering
                m_Editor->UpdateSceneViewport();

                // Sync editor camera to renderer BEFORE rendering
                m_Renderer->SetCamera(m_Editor->GetEditorCamera());

                // Render scene to framebuffer
                m_Editor->BeginSceneRender();

                m_Renderer->BeginFrame();
                m_World->Render(*m_Renderer);
                OnRender();
                m_Renderer->EndFrame();

                m_Editor->EndSceneRender();

                // Restore main framebuffer and clear for ImGui
                int windowWidth, windowHeight;
                glfwGetFramebufferSize(m_Window->GetNativeWindow(), &windowWidth, &windowHeight);
                glViewport(0, 0, windowWidth, windowHeight);
                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // Render ImGui
                m_Editor->BeginFrame();
                m_Editor->Render(*m_World, *m_Renderer);
                OnImGui();
                m_Editor->EndFrame();
            } else {
                // Non-editor mode: render directly to screen
                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                m_Renderer->BeginFrame();
                m_World->Render(*m_Renderer);
                OnRender();
                m_Renderer->EndFrame();
            }

            m_Window->SwapBuffers();
        }
    }

    void Application::Shutdown() {
        XI_LOG_INFO("Xi Engine shutting down...");

        OnShutdown();

        if (m_Editor) {
            m_Editor->Shutdown();
        }

        m_Audio->Shutdown();
        m_Renderer->Shutdown();

        m_World.reset();
        m_Renderer.reset();
        m_Physics.reset();
        m_Audio.reset();
        m_Editor.reset();

        Input::Shutdown();
        m_Window->Shutdown();
        Log::Shutdown();
    }

}
