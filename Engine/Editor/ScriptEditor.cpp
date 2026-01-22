#include "ScriptEditor.h"
#include "../ECS/World.h"
#include "../ECS/Components/Script.h"
#include "../Scripting/ScriptSystem.h"
#include "../Scripting/ScriptEngine.h"
#include "../Core/Log.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace Xi {

    // Keywords for syntax highlighting
    static const std::vector<std::string> s_Keywords = {
        "and", "break", "do", "else", "elseif", "end", "false", "for",
        "function", "goto", "if", "in", "local", "nil", "not", "or",
        "repeat", "return", "then", "true", "until", "while"
    };

    // Built-in functions/globals
    static const std::vector<std::string> s_Builtins = {
        "print", "type", "tonumber", "tostring", "pairs", "ipairs",
        "Input", "Time", "Log", "World", "Vec3", "Vec3Utils", "Key", "Mouse",
        "math", "string", "table", "entity",
        "GetTransform", "SetPosition", "SetRotation", "SetScale",
        "Translate", "Rotate", "GetForward", "GetRight", "GetUp",
        "OnStart", "OnUpdate", "OnDestroy"
    };

    ScriptEditor::ScriptEditor() {
        m_Lines.push_back("");  // Start with one empty line
    }

    ScriptEditor::~ScriptEditor() = default;

    void ScriptEditor::Draw(World& world, ScriptSystem* scriptSystem, ScriptEngine* engine) {
        ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Script Editor")) {
            ImGui::End();
            return;
        }

        if (m_EditingEntity == INVALID_ENTITY || !world.IsEntityValid(m_EditingEntity)) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No script selected.");
            ImGui::TextWrapped("Select an entity with a Script component in the Inspector, "
                              "or add a Script component to an entity.");

            // Show example script
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 0.6f, 1.0f), "Example Script:");
            ImGui::PushStyleColor(ImGuiCol_Text, COLOR_COMMENT);
            ImGui::TextWrapped(
                "-- Example: Rotation script\n"
                "local speed = 90  -- degrees/second\n\n"
                "function OnStart()\n"
                "    Log.Info(\"Script started!\")\n"
                "end\n\n"
                "function OnUpdate(dt)\n"
                "    Rotate(0, speed * dt, 0)\n"
                "end"
            );
            ImGui::PopStyleColor();

            ImGui::End();
            return;
        }

        if (!world.HasComponent<ScriptComponent>(m_EditingEntity)) {
            ImGui::TextColored(ImVec4(0.9f, 0.6f, 0.3f, 1.0f),
                "Selected entity has no Script component.");
            ImGui::End();
            return;
        }

        // Header with entity name
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Editing: %s",
            world.GetEntityName(m_EditingEntity).c_str());
        ImGui::Separator();

        DrawToolbar(world, scriptSystem, engine);
        ImGui::Separator();

        // Reserve space for error panel
        float errorPanelHeight = m_HasError ? 80.0f : 0.0f;
        ImVec2 codeAreaSize = ImGui::GetContentRegionAvail();
        codeAreaSize.y -= errorPanelHeight;

        // Code editor area
        ImGui::PushStyleColor(ImGuiCol_ChildBg, COLOR_BACKGROUND);
        ImGui::BeginChild("CodeEditorArea", codeAreaSize, true,
            ImGuiWindowFlags_HorizontalScrollbar);

        DrawCodeArea();

        ImGui::EndChild();
        ImGui::PopStyleColor();

        // Error panel
        if (m_HasError) {
            DrawErrorPanel();
        }

        ImGui::End();
    }

    void ScriptEditor::DrawToolbar(World& world, ScriptSystem* scriptSystem, ScriptEngine* engine) {
        bool isPlaying = scriptSystem && scriptSystem->IsPlaying();

        // Compile button
        if (ImGui::Button("Compile")) {
            SaveToEntity(world);
            if (engine && scriptSystem) {
                bool success = scriptSystem->CompileScript(world, m_EditingEntity);
                if (success) {
                    m_HasError = false;
                    m_ErrorMessage.clear();
                    m_ErrorLine = -1;
                    XI_LOG_INFO("Script compiled successfully");
                } else {
                    auto& script = world.GetComponent<ScriptComponent>(m_EditingEntity);
                    m_HasError = true;
                    m_ErrorMessage = script.lastError;
                    m_ErrorLine = script.errorLine;
                }
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Check script for errors (Ctrl+B)");
        }

        ImGui::SameLine();

        // Run/Reload button
        if (isPlaying) {
            if (ImGui::Button("Reload")) {
                SaveToEntity(world);
                if (scriptSystem) {
                    scriptSystem->ReloadScript(world, m_EditingEntity);
                    auto& script = world.GetComponent<ScriptComponent>(m_EditingEntity);
                    if (script.hasError) {
                        m_HasError = true;
                        m_ErrorMessage = script.lastError;
                        m_ErrorLine = script.errorLine;
                    } else {
                        m_HasError = false;
                        XI_LOG_INFO("Script reloaded");
                    }
                }
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Reload script while playing");
            }
        }

        ImGui::SameLine();

        // Save button
        if (ImGui::Button("Save")) {
            SaveToEntity(world);
            XI_LOG_INFO("Script saved to component");
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Save changes to component (Ctrl+S)");
        }

        ImGui::SameLine();

        // Reload from entity button
        if (ImGui::Button("Revert")) {
            LoadFromEntity(world);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Revert to saved version");
        }

        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();

        // Line numbers toggle
        ImGui::Checkbox("Lines", &m_ShowLineNumbers);

        // Status indicator
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();

        if (isPlaying) {
            ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "[PLAYING]");
        } else {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "[STOPPED]");
        }

        // Line count
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "| %d lines", GetLineCount());
    }

    void ScriptEditor::DrawCodeArea() {
        // Calculate line height
        m_LineHeight = ImGui::GetTextLineHeightWithSpacing();

        float lineNumberWidth = m_ShowLineNumbers ? 50.0f : 0.0f;

        // Line numbers column
        if (m_ShowLineNumbers) {
            DrawLineNumbers();
            ImGui::SameLine(0, 5);
        }

        // Code display with syntax highlighting
        ImGui::BeginGroup();

        for (int i = 0; i < static_cast<int>(m_Lines.size()); i++) {
            bool isErrorLine = (i + 1 == m_ErrorLine);

            if (isErrorLine) {
                // Highlight error line background
                ImVec2 cursorPos = ImGui::GetCursorScreenPos();
                ImVec2 lineSize(ImGui::GetContentRegionAvail().x, m_LineHeight);
                ImGui::GetWindowDrawList()->AddRectFilled(
                    cursorPos,
                    ImVec2(cursorPos.x + lineSize.x, cursorPos.y + lineSize.y),
                    IM_COL32(80, 30, 30, 255)
                );
            }

            // Render highlighted line
            auto segments = HighlightLine(m_Lines[i]);
            bool first = true;
            for (const auto& seg : segments) {
                if (!first) {
                    ImGui::SameLine(0, 0);
                }
                first = false;
                ImGui::PushStyleColor(ImGuiCol_Text, seg.color);
                ImGui::TextUnformatted(seg.text.c_str());
                ImGui::PopStyleColor();
            }

            // If line is empty, still need to move to next line
            if (segments.empty() || m_Lines[i].empty()) {
                ImGui::TextUnformatted("");
            }
        }

        ImGui::EndGroup();

        // Handle text input using InputTextMultiline
        // We'll overlay an invisible input on top
        ImGui::SetCursorPos(ImVec2(m_ShowLineNumbers ? 55.0f : 5.0f, 5.0f));

        std::string fullText = GetFullText();
        static std::vector<char> buffer(65536, '\0');
        std::copy(fullText.begin(), fullText.end(), buffer.begin());
        buffer[fullText.size()] = '\0';

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 0)); // Invisible text (we draw our own)

        ImVec2 inputSize = ImGui::GetContentRegionAvail();
        inputSize.y = std::max(inputSize.y, m_LineHeight * static_cast<float>(m_Lines.size() + 1));

        ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput |
                                    ImGuiInputTextFlags_CallbackAlways;

        // Actually we need to show the text for editing, so let's use a different approach
        ImGui::PopStyleColor(2);

        // Simple approach: use standard InputTextMultiline
        ImGui::SetCursorPos(ImVec2(m_ShowLineNumbers ? 55.0f : 5.0f, 5.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, COLOR_BACKGROUND);
        ImGui::PushStyleColor(ImGuiCol_Text, COLOR_DEFAULT);

        inputSize = ImGui::GetContentRegionAvail();
        inputSize.x -= 10;
        inputSize.y -= 10;

        // Create a unique ID for the input
        ImGui::PushID("ScriptInput");

        if (ImGui::InputTextMultiline("##code", buffer.data(), buffer.size(),
            inputSize, ImGuiInputTextFlags_AllowTabInput)) {
            SetFullText(buffer.data());
        }

        ImGui::PopID();
        ImGui::PopStyleColor(2);
    }

    void ScriptEditor::DrawLineNumbers() {
        ImGui::BeginChild("LineNumbers", ImVec2(45.0f, 0), false);

        for (int i = 0; i < static_cast<int>(m_Lines.size()); i++) {
            bool isErrorLine = (i + 1 == m_ErrorLine);
            ImVec4 color = isErrorLine ? COLOR_ERROR_LINE : COLOR_LINE_NUM;

            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::Text("%4d", i + 1);
            ImGui::PopStyleColor();
        }

        ImGui::EndChild();
    }

    void ScriptEditor::DrawErrorPanel() {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.25f, 0.10f, 0.10f, 1.0f));
        ImGui::BeginChild("ErrorPanel", ImVec2(0, 75), true);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
        ImGui::TextWrapped("Error: %s", m_ErrorMessage.c_str());
        ImGui::PopStyleColor();

        if (m_ErrorLine > 0) {
            ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.6f, 1.0f), "Line: %d", m_ErrorLine);
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    std::vector<ScriptEditor::HighlightedSegment> ScriptEditor::HighlightLine(const std::string& line) {
        std::vector<HighlightedSegment> segments;

        if (line.empty()) {
            return segments;
        }

        std::string token;
        bool inString = false;
        char stringChar = 0;
        bool inComment = false;

        for (size_t i = 0; i < line.size(); i++) {
            char c = line[i];

            // Check for comment start
            if (!inString && c == '-' && i + 1 < line.size() && line[i + 1] == '-') {
                // Output current token
                if (!token.empty()) {
                    segments.push_back({token, GetTokenColor(token, false, false)});
                    token.clear();
                }
                // Rest of line is comment
                segments.push_back({line.substr(i), COLOR_COMMENT});
                return segments;
            }

            // Check for string start/end
            if (!inComment && (c == '"' || c == '\'')) {
                if (!inString) {
                    // Output current token
                    if (!token.empty()) {
                        segments.push_back({token, GetTokenColor(token, false, false)});
                        token.clear();
                    }
                    inString = true;
                    stringChar = c;
                    token += c;
                } else if (c == stringChar) {
                    token += c;
                    segments.push_back({token, COLOR_STRING});
                    token.clear();
                    inString = false;
                } else {
                    token += c;
                }
                continue;
            }

            if (inString) {
                token += c;
                continue;
            }

            // Token delimiters
            if (std::isspace(c) || std::ispunct(c)) {
                if (!token.empty()) {
                    segments.push_back({token, GetTokenColor(token, false, false)});
                    token.clear();
                }
                // Output delimiter
                std::string delim(1, c);
                segments.push_back({delim, COLOR_DEFAULT});
            } else {
                token += c;
            }
        }

        // Output remaining token
        if (!token.empty()) {
            ImVec4 color = inString ? COLOR_STRING : GetTokenColor(token, false, false);
            segments.push_back({token, color});
        }

        return segments;
    }

    ImVec4 ScriptEditor::GetTokenColor(const std::string& token, bool inString, bool inComment) {
        if (inComment) return COLOR_COMMENT;
        if (inString) return COLOR_STRING;

        if (IsKeyword(token)) return COLOR_KEYWORD;
        if (IsBuiltin(token)) return COLOR_BUILTIN;

        // Check if number
        if (!token.empty()) {
            bool isNumber = true;
            bool hasDot = false;
            for (size_t i = 0; i < token.size(); i++) {
                char c = token[i];
                if (c == '.') {
                    if (hasDot) { isNumber = false; break; }
                    hasDot = true;
                } else if (c == '-' && i == 0) {
                    continue;
                } else if (!std::isdigit(c)) {
                    isNumber = false;
                    break;
                }
            }
            if (isNumber) return COLOR_NUMBER;
        }

        return COLOR_DEFAULT;
    }

    bool ScriptEditor::IsKeyword(const std::string& word) const {
        return std::find(s_Keywords.begin(), s_Keywords.end(), word) != s_Keywords.end();
    }

    bool ScriptEditor::IsBuiltin(const std::string& word) const {
        return std::find(s_Builtins.begin(), s_Builtins.end(), word) != s_Builtins.end();
    }

    std::string ScriptEditor::GetFullText() const {
        std::string result;
        for (size_t i = 0; i < m_Lines.size(); i++) {
            result += m_Lines[i];
            if (i < m_Lines.size() - 1) {
                result += '\n';
            }
        }
        return result;
    }

    void ScriptEditor::SetFullText(const std::string& text) {
        m_Lines.clear();
        std::istringstream stream(text);
        std::string line;
        while (std::getline(stream, line)) {
            // Remove carriage returns if present
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            m_Lines.push_back(line);
        }
        if (m_Lines.empty()) {
            m_Lines.push_back("");
        }
    }

    int ScriptEditor::GetLineCount() const {
        return static_cast<int>(m_Lines.size());
    }

    void ScriptEditor::LoadFromEntity(World& world) {
        if (!world.HasComponent<ScriptComponent>(m_EditingEntity)) return;

        auto& script = world.GetComponent<ScriptComponent>(m_EditingEntity);
        SetFullText(script.source);

        m_HasError = script.hasError;
        m_ErrorMessage = script.lastError;
        m_ErrorLine = script.errorLine;
    }

    void ScriptEditor::SaveToEntity(World& world) {
        if (!world.HasComponent<ScriptComponent>(m_EditingEntity)) return;

        auto& script = world.GetComponent<ScriptComponent>(m_EditingEntity);
        script.source = GetFullText();
        script.ast.clear();  // Invalidate cached AST
    }

}
