#include "World.h"
#include "../Core/Log.h"

namespace Xi {

    std::vector<Entity> World::s_EmptyChildren;

    World::World() {
        XI_LOG_INFO("ECS World created");
    }

    World::~World() {
        Clear();
    }

    Entity World::CreateEntity(const std::string& name) {
        Entity entity = m_NextEntityID++;

        EntityInfo info;
        info.name = name;
        info.active = true;
        info.parent = INVALID_ENTITY;

        m_EntityInfo[entity] = info;
        m_EntityMasks[entity] = ComponentMask();

        return entity;
    }

    void World::DestroyEntity(Entity entity) {
        if (!IsEntityValid(entity)) return;

        // Queue for destruction to avoid iterator invalidation
        m_EntitiesToDestroy.push_back(entity);
    }

    bool World::IsEntityValid(Entity entity) const {
        return m_EntityInfo.find(entity) != m_EntityInfo.end();
    }

    const std::string& World::GetEntityName(Entity entity) const {
        static std::string empty;
        auto it = m_EntityInfo.find(entity);
        return it != m_EntityInfo.end() ? it->second.name : empty;
    }

    void World::SetEntityName(Entity entity, const std::string& name) {
        auto it = m_EntityInfo.find(entity);
        if (it != m_EntityInfo.end()) {
            it->second.name = name;
        }
    }

    bool World::IsEntityActive(Entity entity) const {
        auto it = m_EntityInfo.find(entity);
        return it != m_EntityInfo.end() && it->second.active;
    }

    void World::SetEntityActive(Entity entity, bool active) {
        auto it = m_EntityInfo.find(entity);
        if (it != m_EntityInfo.end()) {
            it->second.active = active;
        }
    }

    void World::SetParent(Entity child, Entity parent) {
        if (!IsEntityValid(child)) return;

        auto& childInfo = m_EntityInfo[child];

        // Remove from old parent
        if (childInfo.parent != INVALID_ENTITY && IsEntityValid(childInfo.parent)) {
            auto& oldParentInfo = m_EntityInfo[childInfo.parent];
            auto it = std::find(oldParentInfo.children.begin(), oldParentInfo.children.end(), child);
            if (it != oldParentInfo.children.end()) {
                oldParentInfo.children.erase(it);
            }
        }

        // Set new parent
        childInfo.parent = parent;

        // Add to new parent's children
        if (parent != INVALID_ENTITY && IsEntityValid(parent)) {
            m_EntityInfo[parent].children.push_back(child);
        }
    }

    Entity World::GetParent(Entity entity) const {
        auto it = m_EntityInfo.find(entity);
        return it != m_EntityInfo.end() ? it->second.parent : INVALID_ENTITY;
    }

    const std::vector<Entity>& World::GetChildren(Entity entity) const {
        auto it = m_EntityInfo.find(entity);
        return it != m_EntityInfo.end() ? it->second.children : s_EmptyChildren;
    }

    std::vector<Entity> World::GetRootEntities() const {
        std::vector<Entity> roots;
        for (const auto& [entity, info] : m_EntityInfo) {
            if (info.parent == INVALID_ENTITY) {
                roots.push_back(entity);
            }
        }
        return roots;
    }

    void World::RegisterDefaultSystems(Renderer& renderer, PhysicsWorld& physics) {
        (void)renderer;
        (void)physics;
        // Systems will be added after component definitions
    }

    void World::Update(float dt) {
        // Process pending destructions
        for (Entity entity : m_EntitiesToDestroy) {
            // Destroy children first
            auto it = m_EntityInfo.find(entity);
            if (it != m_EntityInfo.end()) {
                for (Entity child : it->second.children) {
                    DestroyEntity(child);
                }

                // Remove from parent
                if (it->second.parent != INVALID_ENTITY) {
                    SetParent(entity, INVALID_ENTITY);
                }

                // Remove all components
                for (auto& pool : m_ComponentPools) {
                    if (pool) {
                        pool->RemoveEntity(entity);
                    }
                }

                m_EntityMasks.erase(entity);
                m_EntityInfo.erase(entity);
            }
        }
        m_EntitiesToDestroy.clear();

        // Update systems
        for (auto& system : m_Systems) {
            if (system->IsEnabled()) {
                system->Update(*this, dt);
            }
        }
    }

    void World::Render(Renderer& renderer) {
        for (auto& system : m_Systems) {
            if (system->IsEnabled()) {
                system->Render(*this, renderer);
            }
        }
    }

    void World::Clear() {
        for (auto& pool : m_ComponentPools) {
            if (pool) {
                pool->Clear();
            }
        }
        m_EntityInfo.clear();
        m_EntityMasks.clear();
        m_EntitiesToDestroy.clear();
        m_NextEntityID = 0;
    }

}
