#pragma once

#include "../ECS/Entity.h"

namespace Xi {

    class World;
    class ScriptEditor;

    class Inspector {
    public:
        Inspector();
        ~Inspector();

        void Draw(World& world, Entity entity, ScriptEditor* scriptEditor = nullptr);

    private:
        void DrawTransform(World& world, Entity entity);
        void DrawMeshRenderer(World& world, Entity entity);
        void DrawSpriteRenderer(World& world, Entity entity);
        void DrawCamera(World& world, Entity entity);
        void DrawLight(World& world, Entity entity);
        void DrawCollider(World& world, Entity entity);
        void DrawRigidBody(World& world, Entity entity);
        void DrawAudioSource(World& world, Entity entity);
        void DrawScript(World& world, Entity entity, ScriptEditor* scriptEditor);

        void DrawAddComponentMenu(World& world, Entity entity);
    };

}
