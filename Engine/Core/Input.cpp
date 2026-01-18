#include "Input.h"
#include <GLFW/glfw3.h>

namespace Xi {

    GLFWwindow* Input::s_Window = nullptr;
    std::unordered_map<int, bool> Input::s_CurrentKeys;
    std::unordered_map<int, bool> Input::s_PreviousKeys;
    std::unordered_map<int, bool> Input::s_CurrentMouseButtons;
    std::unordered_map<int, bool> Input::s_PreviousMouseButtons;
    glm::vec2 Input::s_MousePosition = glm::vec2(0.0f);
    glm::vec2 Input::s_LastMousePosition = glm::vec2(0.0f);
    glm::vec2 Input::s_MouseDelta = glm::vec2(0.0f);
    float Input::s_ScrollDelta = 0.0f;
    bool Input::s_CursorLocked = false;
    bool Input::s_FirstMouse = true;

    void Input::Init(GLFWwindow* window) {
        s_Window = window;
        s_FirstMouse = true;

        // Get initial mouse position
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        s_MousePosition = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
        s_LastMousePosition = s_MousePosition;
    }

    void Input::Update() {
        // Store previous state
        s_PreviousKeys = s_CurrentKeys;
        s_PreviousMouseButtons = s_CurrentMouseButtons;

        // Reset per-frame values
        s_MouseDelta = s_MousePosition - s_LastMousePosition;
        s_LastMousePosition = s_MousePosition;
        s_ScrollDelta = 0.0f;
    }

    void Input::Shutdown() {
        s_Window = nullptr;
        s_CurrentKeys.clear();
        s_PreviousKeys.clear();
        s_CurrentMouseButtons.clear();
        s_PreviousMouseButtons.clear();
    }

    bool Input::IsKeyDown(KeyCode key) {
        int k = static_cast<int>(key);
        auto it = s_CurrentKeys.find(k);
        return it != s_CurrentKeys.end() && it->second;
    }

    bool Input::IsKeyPressed(KeyCode key) {
        int k = static_cast<int>(key);
        bool current = s_CurrentKeys.count(k) && s_CurrentKeys[k];
        bool previous = s_PreviousKeys.count(k) && s_PreviousKeys[k];
        return current && !previous;
    }

    bool Input::IsKeyReleased(KeyCode key) {
        int k = static_cast<int>(key);
        bool current = s_CurrentKeys.count(k) && s_CurrentKeys[k];
        bool previous = s_PreviousKeys.count(k) && s_PreviousKeys[k];
        return !current && previous;
    }

    bool Input::IsMouseButtonDown(MouseButton button) {
        int b = static_cast<int>(button);
        auto it = s_CurrentMouseButtons.find(b);
        return it != s_CurrentMouseButtons.end() && it->second;
    }

    bool Input::IsMouseButtonPressed(MouseButton button) {
        int b = static_cast<int>(button);
        bool current = s_CurrentMouseButtons.count(b) && s_CurrentMouseButtons[b];
        bool previous = s_PreviousMouseButtons.count(b) && s_PreviousMouseButtons[b];
        return current && !previous;
    }

    bool Input::IsMouseButtonReleased(MouseButton button) {
        int b = static_cast<int>(button);
        bool current = s_CurrentMouseButtons.count(b) && s_CurrentMouseButtons[b];
        bool previous = s_PreviousMouseButtons.count(b) && s_PreviousMouseButtons[b];
        return !current && previous;
    }

    glm::vec2 Input::GetMousePosition() {
        return s_MousePosition;
    }

    glm::vec2 Input::GetMouseDelta() {
        return s_MouseDelta;
    }

    float Input::GetMouseScrollDelta() {
        return s_ScrollDelta;
    }

    void Input::SetCursorMode(bool locked) {
        s_CursorLocked = locked;
        if (s_Window) {
            glfwSetInputMode(s_Window, GLFW_CURSOR,
                locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            if (locked) {
                s_FirstMouse = true;
            }
        }
    }

    void Input::KeyCallback(int key, int action) {
        if (action == GLFW_PRESS) {
            s_CurrentKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            s_CurrentKeys[key] = false;
        }
    }

    void Input::MouseButtonCallback(int button, int action) {
        if (action == GLFW_PRESS) {
            s_CurrentMouseButtons[button] = true;
        } else if (action == GLFW_RELEASE) {
            s_CurrentMouseButtons[button] = false;
        }
    }

    void Input::MouseMoveCallback(double xpos, double ypos) {
        glm::vec2 newPos(static_cast<float>(xpos), static_cast<float>(ypos));

        if (s_FirstMouse) {
            s_LastMousePosition = newPos;
            s_FirstMouse = false;
        }

        s_MousePosition = newPos;
    }

    void Input::ScrollCallback(double yoffset) {
        s_ScrollDelta = static_cast<float>(yoffset);
    }

}
