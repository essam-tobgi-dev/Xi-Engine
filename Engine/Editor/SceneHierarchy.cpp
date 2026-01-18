#include "SceneHierarchy.h"
#include "../ECS/World.h"

#include <imgui.h>

namespace Xi {

    SceneHierarchy::SceneHierarchy() = default;
    SceneHierarchy::~SceneHierarchy() = default;

    void SceneHierarchy::Draw(World& world) {
        ImGui::Begin("Scene Hierarchy");

        // Add entity button
        if (ImGui::Button("+ Add Entity")) {
            world.CreateEntity("New Entity");
        }

        ImGui::Separator();

        // Draw root entities
        for (Entity entity : world.GetRootEntities()) {
            DrawEntityNode(world, entity);
        }

        // Deselect on empty space click
        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
            if (!ImGui::IsAnyItemHovered()) {
                m_SelectedEntity = INVALID_ENTITY;
            }
        }

        // Right-click context menu
        if (ImGui::BeginPopupContextWindow("HierarchyContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::MenuItem("Create Empty Entity")) {
                world.CreateEntity("New Entity");
            }
            if (ImGui::MenuItem("Create Cube")) {
                world.CreateEntity("Cube");
            }
            if (ImGui::MenuItem("Create Sphere")) {
                world.CreateEntity("Sphere");
            }
            if (ImGui::MenuItem("Create Light")) {
                world.CreateEntity("Light");
            }
            if (ImGui::MenuItem("Create Camera")) {
                world.CreateEntity("Camera");
            }
            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void SceneHierarchy::DrawEntityNode(World& world, Entity entity) {
        const std::string& name = world.GetEntityName(entity);
        const auto& children = world.GetChildren(entity);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

        if (children.empty()) {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }

        if (m_SelectedEntity == entity) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        bool active = world.IsEntityActive(entity);
        if (!active) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        }

        bool opened = ImGui::TreeNodeEx((void*)(intptr_t)entity, flags, "%s", name.c_str());

        if (!active) {
            ImGui::PopStyleColor();
        }

        // Selection
        if (ImGui::IsItemClicked()) {
            m_SelectedEntity = entity;
        }

        // Drag and drop for hierarchy
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
            ImGui::SetDragDropPayload("ENTITY", &entity, sizeof(Entity));
            ImGui::Text("%s", name.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY")) {
                Entity droppedEntity = *(const Entity*)payload->Data;
                if (droppedEntity != entity) {
                    world.SetParent(droppedEntity, entity);
                }
            }
            ImGui::EndDragDropTarget();
        }

        // Context menu
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete")) {
                if (m_SelectedEntity == entity) {
                    m_SelectedEntity = INVALID_ENTITY;
                }
                world.DestroyEntity(entity);
                ImGui::EndPopup();
                if (opened && !children.empty()) {
                    ImGui::TreePop();
                }
                return;
            }
            if (ImGui::MenuItem("Duplicate")) {
                // TODO: Implement entity duplication
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Create Child")) {
                Entity child = world.CreateEntity("Child");
                world.SetParent(child, entity);
            }
            if (ImGui::MenuItem("Unparent")) {
                world.SetParent(entity, INVALID_ENTITY);
            }
            ImGui::EndPopup();
        }

        // Draw children
        if (opened && !children.empty()) {
            for (Entity child : children) {
                DrawEntityNode(world, child);
            }
            ImGui::TreePop();
        }
    }

}
