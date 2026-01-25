#pragma once

#include "Entity.h"
#include "Component.h"
#include "System.h"

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <functional>
#include <algorithm>

namespace Xi {

    class Renderer;
    class PhysicsWorld;

    struct EntityInfo {
        std::string name;
        bool active = true;
        Entity parent = INVALID_ENTITY;
        std::vector<Entity> children;
    };

    class World {
    public:
        World();
        ~World();

        // Entity management
        Entity CreateEntity(const std::string& name = "Entity");
        void DestroyEntity(Entity entity);
        bool IsEntityValid(Entity entity) const;

        // Entity info
        const std::string& GetEntityName(Entity entity) const;
        void SetEntityName(Entity entity, const std::string& name);
        bool IsEntityActive(Entity entity) const;
        void SetEntityActive(Entity entity, bool active);

        // Hierarchy
        void SetParent(Entity child, Entity parent);
        Entity GetParent(Entity entity) const;
        const std::vector<Entity>& GetChildren(Entity entity) const;
        std::vector<Entity> GetRootEntities() const;

        // Component management
        template<typename T>
        T& AddComponent(Entity entity) {
            ComponentTypeID typeID = GetComponentTypeID<T>();
            EnsureComponentPool<T>(typeID);

            auto* pool = static_cast<ComponentPool<T>*>(m_ComponentPools[typeID].get());
            T& component = pool->Add(entity);

            m_EntityMasks[entity].set(typeID);
            return component;
        }

        template<typename T>
        void RemoveComponent(Entity entity) {
            ComponentTypeID typeID = GetComponentTypeID<T>();
            if (typeID >= m_ComponentPools.size() || !m_ComponentPools[typeID]) return;

            auto* pool = static_cast<ComponentPool<T>*>(m_ComponentPools[typeID].get());
            pool->RemoveEntity(entity);
            m_EntityMasks[entity].reset(typeID);
        }

        template<typename T>
        T& GetComponent(Entity entity) {
            ComponentTypeID typeID = GetComponentTypeID<T>();
            auto* pool = static_cast<ComponentPool<T>*>(m_ComponentPools[typeID].get());
            return pool->Get(entity);
        }

        template<typename T>
        const T& GetComponent(Entity entity) const {
            ComponentTypeID typeID = GetComponentTypeID<T>();
            auto* pool = static_cast<const ComponentPool<T>*>(m_ComponentPools[typeID].get());
            return pool->Get(entity);
        }

        template<typename T>
        bool HasComponent(Entity entity) const {
            ComponentTypeID typeID = GetComponentTypeID<T>();
            if (typeID >= m_ComponentPools.size() || !m_ComponentPools[typeID]) return false;

            auto it = m_EntityMasks.find(entity);
            if (it == m_EntityMasks.end()) return false;

            return it->second.test(typeID);
        }

        template<typename T>
        ComponentPool<T>* GetComponentPool() {
            ComponentTypeID typeID = GetComponentTypeID<T>();
            if (typeID >= m_ComponentPools.size() || !m_ComponentPools[typeID]) return nullptr;
            return static_cast<ComponentPool<T>*>(m_ComponentPools[typeID].get());
        }

        // Iterate entities with specific components
        template<typename... Components>
        void ForEach(std::function<void(Entity, Components&...)> func) {
            // Get the first component pool to iterate
            using FirstComponent = std::tuple_element_t<0, std::tuple<Components...>>;
            auto* pool = GetComponentPool<FirstComponent>();
            if (!pool) return;

            for (Entity entity : pool->GetEntities()) {
                if ((HasComponent<Components>(entity) && ...)) {
                    func(entity, GetComponent<Components>(entity)...);
                }
            }
        }

        // System management
        template<typename T, typename... Args>
        T* AddSystem(Args&&... args) {
            auto system = std::make_unique<T>(std::forward<Args>(args)...);
            T* ptr = system.get();
            m_Systems.push_back(std::move(system));
            return ptr;
        }

        void RegisterDefaultSystems(Renderer& renderer, PhysicsWorld& physics);

        // Update all systems
        void Update(float dt);
        void Render(Renderer& renderer);

        // Get all entities
        const std::unordered_map<Entity, EntityInfo>& GetEntities() const { return m_EntityInfo; }
        size_t GetEntityCount() const { return m_EntityInfo.size(); }

        void Clear();

    private:
        template<typename T>
        void EnsureComponentPool(ComponentTypeID typeID) {
            if (typeID >= m_ComponentPools.size()) {
                m_ComponentPools.resize(typeID + 1);
            }
            if (!m_ComponentPools[typeID]) {
                m_ComponentPools[typeID] = std::make_unique<ComponentPool<T>>();
            }
        }

        Entity m_NextEntityID = 0;
        std::unordered_map<Entity, EntityInfo> m_EntityInfo;
        std::unordered_map<Entity, ComponentMask> m_EntityMasks;
        std::vector<std::unique_ptr<ComponentPoolBase>> m_ComponentPools;
        std::vector<std::unique_ptr<System>> m_Systems;
        std::vector<Entity> m_EntitiesToDestroy;

        static std::vector<Entity> s_EmptyChildren;
    };

}
