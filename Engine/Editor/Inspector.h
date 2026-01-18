#pragma once

#include "../ECS/Entity.h"

namespace Xi {

    class World;

    class Inspector {
    public:
        Inspector();
        ~Inspector();

        void Draw(World& world, Entity entity);

    private:
        void DrawTransform(World& world, Entity entity);
        void DrawMeshRenderer(World& world, Entity entity);
        void DrawSpriteRenderer(World& world, Entity entity);
        void DrawCamera(World& world, Entity entity);
        void DrawLight(World& world, Entity entity);
        void DrawCollider(World& world, Entity entity);
        void DrawRigidBody(World& world, Entity entity);
        void DrawAudioSource(World& world, Entity entity);

        void DrawAddComponentMenu(World& world, Entity entity);
    };

}
