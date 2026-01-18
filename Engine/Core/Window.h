#pragma once

#include <string>
#include <functional>

struct GLFWwindow;

namespace Xi {

    struct WindowProps {
        std::string title = "Xi Engine";
        int width = 1280;
        int height = 720;
        bool vsync = true;
        bool fullscreen = false;
    };

    class Window {
    public:
        using ResizeCallback = std::function<void(int, int)>;
        using CloseCallback = std::function<void()>;

        Window(const WindowProps& props = WindowProps());
        ~Window();

        bool Init();
        void Shutdown();

        void PollEvents();
        void SwapBuffers();

        bool ShouldClose() const;
        void SetShouldClose(bool close);

        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }
        float GetAspectRatio() const { return static_cast<float>(m_Width) / static_cast<float>(m_Height); }

        GLFWwindow* GetNativeWindow() const { return m_Window; }

        void SetVSync(bool enabled);
        bool IsVSync() const { return m_VSync; }

        void SetTitle(const std::string& title);

        void SetResizeCallback(ResizeCallback callback) { m_ResizeCallback = callback; }
        void SetCloseCallback(CloseCallback callback) { m_CloseCallback = callback; }

        // Internal callbacks
        void OnResize(int width, int height);
        void OnClose();

    private:
        GLFWwindow* m_Window = nullptr;
        std::string m_Title;
        int m_Width;
        int m_Height;
        bool m_VSync;
        bool m_Fullscreen;

        ResizeCallback m_ResizeCallback;
        CloseCallback m_CloseCallback;
    };

}
