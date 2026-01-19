#include "EditorUI.h"
#include "../ECS/World.h"
#include "../Renderer/Renderer.h"
#include "../Core/Time.h"
#include "../Core/Input.h"
#include "../Core/Log.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace Xi {

    EditorUI::EditorUI() {
        m_EditorCamera.SetPerspective(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
        m_EditorCamera.SetPosition(glm::vec3(0.0f, 5.0f, 10.0f));
        m_EditorCamera.SetRotation(glm::vec3(20.0f, 0.0f, 0.0f));  // Pitch down 20 degrees, yaw 0 (look towards -Z)
    }

    EditorUI::~EditorUI() = default;

    bool EditorUI::Init(GLFWwindow* window) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        SetupImGuiStyle();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 450");

        // Create scene framebuffer
        FramebufferSpec fbSpec;
        fbSpec.width = 1280;
        fbSpec.height = 720;
        m_SceneFramebuffer = std::make_unique<Framebuffer>(fbSpec);

        XI_LOG_INFO("Editor UI initialized");
        return true;
    }

    void EditorUI::Shutdown() {
        m_SceneFramebuffer.reset();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void EditorUI::BeginSceneRender() {
        m_SceneFramebuffer->Bind();

        // Ensure proper OpenGL state for 3D rendering
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        // Use a distinct color (dark blue) so we can tell if framebuffer is displayed
        glClearColor(0.05f, 0.05f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void EditorUI::EndSceneRender() {
        m_SceneFramebuffer->Unbind();
    }

    void EditorUI::UpdateSceneViewport() {
        // This should be called before scene rendering to handle resize
        // We use the cached viewport size from the previous frame
        // On first frame, it uses the default 1280x720

        uint32_t fbWidth = m_SceneFramebuffer->GetWidth();
        uint32_t fbHeight = m_SceneFramebuffer->GetHeight();

        if ((uint32_t)m_SceneViewportSize.x != fbWidth ||
            (uint32_t)m_SceneViewportSize.y != fbHeight) {

            if (m_SceneViewportSize.x > 0 && m_SceneViewportSize.y > 0) {
                m_SceneFramebuffer->Resize((uint32_t)m_SceneViewportSize.x,
                                           (uint32_t)m_SceneViewportSize.y);
                m_EditorCamera.SetAspectRatio(m_SceneViewportSize.x / m_SceneViewportSize.y);
            }
        }
    }

    void EditorUI::BeginFrame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void EditorUI::EndFrame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void EditorUI::Render(World& world, Renderer& renderer) {
        DrawMenuBar(world);
        DrawToolbar();

        if (m_ShowHierarchy) {
            m_Hierarchy.Draw(world);
        }

        if (m_ShowInspector) {
            m_Inspector.Draw(world, m_Hierarchy.GetSelectedEntity());
        }

        if (m_ShowConsole) {
            m_Console.Draw();
        }

        if (m_ShowStats) {
            DrawStats();
        }

        if (m_ShowDemo) {
            ImGui::ShowDemoWindow(&m_ShowDemo);
        }

        // Scene viewport
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Scene");

        // Right-click to control camera
        if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            m_CameraActive = true;
            Input::SetCursorMode(true);
        } else if (m_CameraActive && !ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            m_CameraActive = false;
            Input::SetCursorMode(false);
        }

        UpdateEditorCamera(Time::GetDeltaTime());

        // Sync editor camera to renderer (for next frame)
        renderer.SetCamera(m_EditorCamera);

        // Get current viewport size and cache it for next frame's resize
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        if (viewportSize.x > 0 && viewportSize.y > 0) {
            // Update cached size for next frame's resize (before rendering)
            m_SceneViewportSize = { viewportSize.x, viewportSize.y };

            // Display the framebuffer texture (flip UV vertically for OpenGL)
            uint32_t textureID = m_SceneFramebuffer->GetColorAttachment();
            ImGui::Image((ImTextureID)(uintptr_t)textureID, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void EditorUI::SetupImGuiStyle() {
        ImGuiStyle& style = ImGui::GetStyle();

        // Dark theme colors
        ImVec4* colors = style.Colors;
        colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg]                = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
        colors[ImGuiCol_Border]                 = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
        colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
        colors[ImGuiCol_FrameBg]                = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
        colors[ImGuiCol_FrameBgActive]          = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
        colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
        colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
        colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
        colors[ImGuiCol_CheckMark]              = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
        colors[ImGuiCol_SliderGrab]             = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
        colors[ImGuiCol_Button]                 = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
        colors[ImGuiCol_Header]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
        colors[ImGuiCol_Separator]              = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
        colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
        colors[ImGuiCol_SeparatorActive]        = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
        colors[ImGuiCol_ResizeGrip]             = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
        colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
        colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
        colors[ImGuiCol_Tab]                    = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TabHovered]             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
        colors[ImGuiCol_TabUnfocused]           = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);

        style.WindowPadding     = ImVec2(8.00f, 8.00f);
        style.FramePadding      = ImVec2(5.00f, 2.00f);
        style.CellPadding       = ImVec2(6.00f, 6.00f);
        style.ItemSpacing       = ImVec2(6.00f, 6.00f);
        style.ItemInnerSpacing  = ImVec2(6.00f, 6.00f);
        style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
        style.IndentSpacing     = 25;
        style.ScrollbarSize     = 15;
        style.GrabMinSize       = 10;
        style.WindowBorderSize  = 1;
        style.ChildBorderSize   = 1;
        style.PopupBorderSize   = 1;
        style.FrameBorderSize   = 1;
        style.TabBorderSize     = 1;
        style.WindowRounding    = 7;
        style.ChildRounding     = 4;
        style.FrameRounding     = 3;
        style.PopupRounding     = 4;
        style.ScrollbarRounding = 9;
        style.GrabRounding      = 3;
        style.LogSliderDeadzone = 4;
        style.TabRounding       = 4;
    }

    void EditorUI::DrawMenuBar(World& world) {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                    world.Clear();
                }
                if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {
                    // TODO: Open file dialog
                }
                if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                    // TODO: Save scene
                }
                if (ImGui::MenuItem("Save Scene As", "Ctrl+Shift+S")) {
                    // TODO: Save scene as
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4")) {
                    // TODO: Exit application
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
                if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Hierarchy", nullptr, &m_ShowHierarchy);
                ImGui::MenuItem("Inspector", nullptr, &m_ShowInspector);
                ImGui::MenuItem("Console", nullptr, &m_ShowConsole);
                ImGui::MenuItem("Stats", nullptr, &m_ShowStats);
                ImGui::Separator();
                ImGui::MenuItem("ImGui Demo", nullptr, &m_ShowDemo);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Entity")) {
                if (ImGui::MenuItem("Create Empty")) {
                    world.CreateEntity("New Entity");
                }
                if (ImGui::BeginMenu("3D Object")) {
                    if (ImGui::MenuItem("Cube")) { world.CreateEntity("Cube"); }
                    if (ImGui::MenuItem("Sphere")) { world.CreateEntity("Sphere"); }
                    if (ImGui::MenuItem("Plane")) { world.CreateEntity("Plane"); }
                    if (ImGui::MenuItem("Cylinder")) { world.CreateEntity("Cylinder"); }
                    if (ImGui::MenuItem("Cone")) { world.CreateEntity("Cone"); }
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem("Light")) { world.CreateEntity("Light"); }
                if (ImGui::MenuItem("Camera")) { world.CreateEntity("Camera"); }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }

    void EditorUI::DrawToolbar() {
        ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

        if (ImGui::Button("Play")) {
            XI_LOG_INFO("Play mode started");
        }
        ImGui::SameLine();
        if (ImGui::Button("Pause")) {
            XI_LOG_INFO("Paused");
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            XI_LOG_INFO("Stopped");
        }

        ImGui::End();
    }

    void EditorUI::DrawStats() {
        ImGui::Begin("Stats");

        ImGui::Text("FPS: %d", Time::GetFPS());
        ImGui::Text("Frame Time: %.2f ms", Time::GetFrameTime());
        ImGui::Text("Delta Time: %.4f s", Time::GetDeltaTime());

        ImGui::Separator();

        ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)",
            m_EditorCamera.GetPosition().x,
            m_EditorCamera.GetPosition().y,
            m_EditorCamera.GetPosition().z);

        ImGui::End();
    }

    void EditorUI::UpdateEditorCamera(float dt) {
        if (!m_CameraActive) return;

        glm::vec3 position = m_EditorCamera.GetPosition();
        glm::vec3 rotation = m_EditorCamera.GetRotation();

        // Mouse look
        glm::vec2 mouseDelta = Input::GetMouseDelta();
        rotation.y += mouseDelta.x * m_CameraLookSpeed;
        rotation.x += mouseDelta.y * m_CameraLookSpeed;
        rotation.x = glm::clamp(rotation.x, -89.0f, 89.0f);

        m_EditorCamera.SetRotation(rotation);

        // Keyboard movement
        glm::vec3 forward = m_EditorCamera.GetForward();
        glm::vec3 right = m_EditorCamera.GetRight();
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        float speed = m_CameraMoveSpeed * dt;

        if (Input::IsKeyDown(KeyCode::LeftShift)) {
            speed *= 3.0f;
        }

        if (Input::IsKeyDown(KeyCode::W)) {
            position += forward * speed;
        }
        if (Input::IsKeyDown(KeyCode::S)) {
            position -= forward * speed;
        }
        if (Input::IsKeyDown(KeyCode::A)) {
            position -= right * speed;
        }
        if (Input::IsKeyDown(KeyCode::D)) {
            position += right * speed;
        }
        if (Input::IsKeyDown(KeyCode::E)) {
            position += up * speed;
        }
        if (Input::IsKeyDown(KeyCode::Q)) {
            position -= up * speed;
        }

        m_EditorCamera.SetPosition(position);
    }

}
