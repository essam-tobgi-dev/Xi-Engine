#include "Window.h"
#include "Log.h"
#include "Input.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace Xi {

    static bool s_GLFWInitialized = false;

    static void GLFWErrorCallback(int error, const char* description) {
        XI_LOG_ERROR("GLFW Error (" + std::to_string(error) + "): " + description);
    }

    Window::Window(const WindowProps& props)
        : m_Title(props.title)
        , m_Width(props.width)
        , m_Height(props.height)
        , m_VSync(props.vsync)
        , m_Fullscreen(props.fullscreen)
    {
    }

    Window::~Window() {
        Shutdown();
    }

    bool Window::Init() {
        if (!s_GLFWInitialized) {
            if (!glfwInit()) {
                XI_LOG_ERROR("Failed to initialize GLFW");
                return false;
            }
            glfwSetErrorCallback(GLFWErrorCallback);
            s_GLFWInitialized = true;
        }

        // OpenGL context hints
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GLFWmonitor* monitor = nullptr;
        if (m_Fullscreen) {
            monitor = glfwGetPrimaryMonitor();
        }

        m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), monitor, nullptr);
        if (!m_Window) {
            XI_LOG_ERROR("Failed to create GLFW window");
            return false;
        }

        glfwMakeContextCurrent(m_Window);

        // Initialize GLEW
        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (err != GLEW_OK) {
            XI_LOG_ERROR("Failed to initialize GLEW: " + std::string((const char*)glewGetErrorString(err)));
            return false;
        }

        XI_LOG_INFO("OpenGL Info:");
        XI_LOG_INFO("  Vendor: " + std::string((const char*)glGetString(GL_VENDOR)));
        XI_LOG_INFO("  Renderer: " + std::string((const char*)glGetString(GL_RENDERER)));
        XI_LOG_INFO("  Version: " + std::string((const char*)glGetString(GL_VERSION)));

        SetVSync(m_VSync);

        // Set window user pointer for callbacks
        glfwSetWindowUserPointer(m_Window, this);

        // Set callbacks
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
            Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
            win->OnResize(width, height);
        });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
            Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
            win->OnClose();
        });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            (void)window; (void)scancode; (void)mods;
            Input::KeyCallback(key, action);
        });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
            (void)window; (void)mods;
            Input::MouseButtonCallback(button, action);
        });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) {
            (void)window;
            Input::MouseMoveCallback(xpos, ypos);
        });

        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset) {
            (void)window; (void)xoffset;
            Input::ScrollCallback(yoffset);
        });

        XI_LOG_INFO("Window created: " + m_Title + " (" + std::to_string(m_Width) + "x" + std::to_string(m_Height) + ")");
        return true;
    }

    void Window::Shutdown() {
        if (m_Window) {
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;
        }
    }

    void Window::PollEvents() {
        glfwPollEvents();
    }

    void Window::SwapBuffers() {
        glfwSwapBuffers(m_Window);
    }

    bool Window::ShouldClose() const {
        return glfwWindowShouldClose(m_Window);
    }

    void Window::SetShouldClose(bool close) {
        glfwSetWindowShouldClose(m_Window, close ? GLFW_TRUE : GLFW_FALSE);
    }

    void Window::SetVSync(bool enabled) {
        m_VSync = enabled;
        glfwSwapInterval(enabled ? 1 : 0);
    }

    void Window::SetTitle(const std::string& title) {
        m_Title = title;
        glfwSetWindowTitle(m_Window, title.c_str());
    }

    void Window::OnResize(int width, int height) {
        m_Width = width;
        m_Height = height;
        glViewport(0, 0, width, height);

        if (m_ResizeCallback) {
            m_ResizeCallback(width, height);
        }
    }

    void Window::OnClose() {
        if (m_CloseCallback) {
            m_CloseCallback();
        }
    }

}
