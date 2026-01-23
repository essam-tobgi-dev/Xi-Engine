#include "ScriptEditor.h"
#include "../ECS/World.h"
#include "../ECS/Components/Script.h"
#include "../Scripting/ScriptSystem.h"
#include "../Scripting/ScriptEngine.h"
#include "../Core/Log.h"

namespace Xi {

    ScriptEditor::ScriptEditor() {
        // Set up Lua language definition
        m_Editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
        m_Editor.SetPalette(TextEditor::GetDarkPalette());
        m_Editor.SetShowWhitespaces(false);
    }

    ScriptEditor::~ScriptEditor() = default;

    void ScriptEditor::SetEditingEntity(Entity entity) {
        if (m_EditingEntity != entity) {
            m_EditingEntity = entity;
            // Clear error state when switching entities
            m_HasError = false;
            m_ErrorMessage.clear();
            m_ErrorLine = -1;
            m_Editor.SetErrorMarkers({});
        }
    }

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
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 0.55f, 0.50f, 1.0f));
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

        // Render the text editor
        m_Editor.Render("##ScriptEditor", codeAreaSize, true);

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
                    m_Editor.SetErrorMarkers({});
                    XI_LOG_INFO("Script compiled successfully");
                } else {
                    auto& script = world.GetComponent<ScriptComponent>(m_EditingEntity);
                    m_HasError = true;
                    m_ErrorMessage = script.lastError;
                    m_ErrorLine = script.errorLine;

                    // Set error marker in editor
                    TextEditor::ErrorMarkers markers;
                    if (m_ErrorLine > 0) {
                        markers[m_ErrorLine] = m_ErrorMessage;
                    }
                    m_Editor.SetErrorMarkers(markers);
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

                        TextEditor::ErrorMarkers markers;
                        if (m_ErrorLine > 0) {
                            markers[m_ErrorLine] = m_ErrorMessage;
                        }
                        m_Editor.SetErrorMarkers(markers);
                    } else {
                        m_HasError = false;
                        m_Editor.SetErrorMarkers({});
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
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "| %d lines", m_Editor.GetTotalLines());
    }

    void ScriptEditor::DrawErrorPanel() {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.25f, 0.10f, 0.10f, 1.0f));
        ImGui::BeginChild("ErrorPanel", ImVec2(0, 75), true);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
        ImGui::TextWrapped("Error: %s", m_ErrorMessage.c_str());
        ImGui::PopStyleColor();

        if (m_ErrorLine > 0) {
            ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.6f, 1.0f), "Line: %d", m_ErrorLine);

            // Click to go to error line
            if (ImGui::IsItemClicked()) {
                m_Editor.SetCursorPosition(TextEditor::Coordinates(m_ErrorLine - 1, 0));
            }
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    void ScriptEditor::LoadFromEntity(World& world) {
        if (!world.HasComponent<ScriptComponent>(m_EditingEntity)) return;

        auto& script = world.GetComponent<ScriptComponent>(m_EditingEntity);
        m_Editor.SetText(script.source);

        m_HasError = script.hasError;
        m_ErrorMessage = script.lastError;
        m_ErrorLine = script.errorLine;

        // Set error markers if there's an error
        TextEditor::ErrorMarkers markers;
        if (m_HasError && m_ErrorLine > 0) {
            markers[m_ErrorLine] = m_ErrorMessage;
        }
        m_Editor.SetErrorMarkers(markers);
    }

    void ScriptEditor::SaveToEntity(World& world) {
        if (!world.HasComponent<ScriptComponent>(m_EditingEntity)) return;

        auto& script = world.GetComponent<ScriptComponent>(m_EditingEntity);
        script.source = m_Editor.GetText();
        script.ast.clear();  // Invalidate cached AST
    }

}
