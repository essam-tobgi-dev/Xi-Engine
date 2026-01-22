#pragma once

#include "../ECS/Entity.h"
#include <string>
#include <vector>
#include <imgui.h>

namespace Xi {

    class World;
    class ScriptSystem;
    class ScriptEngine;

    class ScriptEditor {
    public:
        ScriptEditor();
        ~ScriptEditor();

        void Draw(World& world, ScriptSystem* scriptSystem, ScriptEngine* engine);

        void SetEditingEntity(Entity entity) { m_EditingEntity = entity; }
        Entity GetEditingEntity() const { return m_EditingEntity; }

        void LoadFromEntity(World& world);
        void SaveToEntity(World& world);

    private:
        void DrawToolbar(World& world, ScriptSystem* scriptSystem, ScriptEngine* engine);
        void DrawCodeArea();
        void DrawLineNumbers();
        void DrawErrorPanel();

        // Syntax highlighting
        struct HighlightedSegment {
            std::string text;
            ImVec4 color;
        };
        std::vector<HighlightedSegment> HighlightLine(const std::string& line);
        ImVec4 GetTokenColor(const std::string& token, bool inString, bool inComment);
        bool IsKeyword(const std::string& word) const;
        bool IsBuiltin(const std::string& word) const;

        // Text helpers
        std::string GetFullText() const;
        void SetFullText(const std::string& text);
        int GetLineCount() const;

        Entity m_EditingEntity = INVALID_ENTITY;

        // Text storage (simple line-based)
        std::vector<std::string> m_Lines;

        // UI state
        bool m_ShowLineNumbers = true;
        float m_LineHeight = 0;

        // Error state
        bool m_HasError = false;
        std::string m_ErrorMessage;
        int m_ErrorLine = -1;

        // Colors (dark theme)
        static constexpr ImVec4 COLOR_KEYWORD = ImVec4(0.78f, 0.45f, 0.82f, 1.0f);   // Purple
        static constexpr ImVec4 COLOR_BUILTIN = ImVec4(0.40f, 0.75f, 0.90f, 1.0f);   // Cyan
        static constexpr ImVec4 COLOR_STRING = ImVec4(0.87f, 0.63f, 0.45f, 1.0f);    // Orange
        static constexpr ImVec4 COLOR_NUMBER = ImVec4(0.70f, 0.87f, 0.53f, 1.0f);    // Light green
        static constexpr ImVec4 COLOR_COMMENT = ImVec4(0.50f, 0.55f, 0.50f, 1.0f);   // Gray
        static constexpr ImVec4 COLOR_DEFAULT = ImVec4(0.90f, 0.90f, 0.90f, 1.0f);   // White
        static constexpr ImVec4 COLOR_BACKGROUND = ImVec4(0.08f, 0.08f, 0.10f, 1.0f);
        static constexpr ImVec4 COLOR_LINE_NUM = ImVec4(0.45f, 0.45f, 0.50f, 1.0f);
        static constexpr ImVec4 COLOR_ERROR_LINE = ImVec4(0.90f, 0.30f, 0.30f, 1.0f);
    };

}
