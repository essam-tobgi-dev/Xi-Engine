#pragma once

#include <glm/glm.hpp>
#include <unordered_map>

struct GLFWwindow;

namespace Xi {

    enum class KeyCode {
        // Letters
        A = 65, B = 66, C = 67, D = 68, E = 69, F = 70, G = 71, H = 72,
        I = 73, J = 74, K = 75, L = 76, M = 77, N = 78, O = 79, P = 80,
        Q = 81, R = 82, S = 83, T = 84, U = 85, V = 86, W = 87, X = 88,
        Y = 89, Z = 90,

        // Numbers
        Num0 = 48, Num1 = 49, Num2 = 50, Num3 = 51, Num4 = 52,
        Num5 = 53, Num6 = 54, Num7 = 55, Num8 = 56, Num9 = 57,

        // Function keys
        F1 = 290, F2 = 291, F3 = 292, F4 = 293, F5 = 294, F6 = 295,
        F7 = 296, F8 = 297, F9 = 298, F10 = 299, F11 = 300, F12 = 301,

        // Special keys
        Space = 32,
        Escape = 256,
        Enter = 257,
        Tab = 258,
        Backspace = 259,
        Insert = 260,
        Delete = 261,
        Right = 262,
        Left = 263,
        Down = 264,
        Up = 265,
        PageUp = 266,
        PageDown = 267,
        Home = 268,
        End = 269,

        // Modifiers
        LeftShift = 340,
        LeftControl = 341,
        LeftAlt = 342,
        RightShift = 344,
        RightControl = 345,
        RightAlt = 346
    };

    enum class MouseButton {
        Left = 0,
        Right = 1,
        Middle = 2
    };

    class Input {
    public:
        static void Init(GLFWwindow* window);
        static void Update();
        static void Shutdown();

        // Keyboard
        static bool IsKeyDown(KeyCode key);
        static bool IsKeyPressed(KeyCode key);  // Just pressed this frame
        static bool IsKeyReleased(KeyCode key); // Just released this frame

        // Mouse
        static bool IsMouseButtonDown(MouseButton button);
        static bool IsMouseButtonPressed(MouseButton button);
        static bool IsMouseButtonReleased(MouseButton button);

        static glm::vec2 GetMousePosition();
        static glm::vec2 GetMouseDelta();
        static float GetMouseScrollDelta();

        // Cursor control
        static void SetCursorMode(bool locked);
        static bool IsCursorLocked() { return s_CursorLocked; }

        // Callbacks (internal use)
        static void KeyCallback(int key, int action);
        static void MouseButtonCallback(int button, int action);
        static void MouseMoveCallback(double xpos, double ypos);
        static void ScrollCallback(double yoffset);

    private:
        static GLFWwindow* s_Window;

        static std::unordered_map<int, bool> s_CurrentKeys;
        static std::unordered_map<int, bool> s_PreviousKeys;

        static std::unordered_map<int, bool> s_CurrentMouseButtons;
        static std::unordered_map<int, bool> s_PreviousMouseButtons;

        static glm::vec2 s_MousePosition;
        static glm::vec2 s_LastMousePosition;
        static glm::vec2 s_MouseDelta;
        static float s_ScrollDelta;
        static bool s_CursorLocked;
        static bool s_FirstMouse;
    };

}
