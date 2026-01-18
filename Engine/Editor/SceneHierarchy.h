#pragma once

#include "../ECS/Entity.h"

namespace Xi {

    class World;

    class SceneHierarchy {
    public:
        SceneHierarchy();
        ~SceneHierarchy();

        void Draw(World& world);

        Entity GetSelectedEntity() const { return m_SelectedEntity; }
        void SetSelectedEntity(Entity entity) { m_SelectedEntity = entity; }

    private:
        void DrawEntityNode(World& world, Entity entity);

        Entity m_SelectedEntity = INVALID_ENTITY;
    };

}
