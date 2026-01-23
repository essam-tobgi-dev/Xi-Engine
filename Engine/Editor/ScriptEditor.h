#pragma once

#include "../ECS/Entity.h"
#include <string>
#include <TextEditor.h>

namespace Xi {

    class World;
    class ScriptSystem;
    class ScriptEngine;

    class ScriptEditor {
    public:
        ScriptEditor();
        ~ScriptEditor();

        void Draw(World& world, ScriptSystem* scriptSystem, ScriptEngine* engine);

        void SetEditingEntity(Entity entity);
        Entity GetEditingEntity() const { return m_EditingEntity; }

        void LoadFromEntity(World& world);
        void SaveToEntity(World& world);

    private:
        void DrawToolbar(World& world, ScriptSystem* scriptSystem, ScriptEngine* engine);
        void DrawErrorPanel();

        Entity m_EditingEntity = INVALID_ENTITY;

        // ImGuiColorTextEdit editor
        TextEditor m_Editor;

        // Error state
        bool m_HasError = false;
        std::string m_ErrorMessage;
        int m_ErrorLine = -1;
    };

}
