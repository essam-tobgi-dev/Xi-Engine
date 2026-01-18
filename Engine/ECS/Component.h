#pragma once

#include "Entity.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <bitset>

namespace Xi {

    using ComponentMask = std::bitset<MAX_COMPONENTS>;

    // Base class for component pools
    class ComponentPoolBase {
    public:
        virtual ~ComponentPoolBase() = default;
        virtual void RemoveEntity(Entity entity) = 0;
        virtual bool HasEntity(Entity entity) const = 0;
        virtual void Clear() = 0;
    };

    // Templated component pool for storing components of a specific type
    template<typename T>
    class ComponentPool : public ComponentPoolBase {
    public:
        T& Add(Entity entity) {
            if (m_EntityToIndex.find(entity) != m_EntityToIndex.end()) {
                return m_Components[m_EntityToIndex[entity]];
            }

            size_t index = m_Components.size();
            m_Components.emplace_back();
            m_Entities.push_back(entity);
            m_EntityToIndex[entity] = index;
            return m_Components.back();
        }

        T& Get(Entity entity) {
            return m_Components[m_EntityToIndex.at(entity)];
        }

        const T& Get(Entity entity) const {
            return m_Components[m_EntityToIndex.at(entity)];
        }

        bool HasEntity(Entity entity) const override {
            return m_EntityToIndex.find(entity) != m_EntityToIndex.end();
        }

        void RemoveEntity(Entity entity) override {
            auto it = m_EntityToIndex.find(entity);
            if (it == m_EntityToIndex.end()) return;

            size_t indexToRemove = it->second;
            size_t lastIndex = m_Components.size() - 1;

            if (indexToRemove != lastIndex) {
                // Swap with last element
                m_Components[indexToRemove] = std::move(m_Components[lastIndex]);
                m_Entities[indexToRemove] = m_Entities[lastIndex];
                m_EntityToIndex[m_Entities[indexToRemove]] = indexToRemove;
            }

            m_Components.pop_back();
            m_Entities.pop_back();
            m_EntityToIndex.erase(entity);
        }

        void Clear() override {
            m_Components.clear();
            m_Entities.clear();
            m_EntityToIndex.clear();
        }

        // Iteration support
        std::vector<T>& GetComponents() { return m_Components; }
        const std::vector<T>& GetComponents() const { return m_Components; }
        std::vector<Entity>& GetEntities() { return m_Entities; }
        const std::vector<Entity>& GetEntities() const { return m_Entities; }

        size_t Size() const { return m_Components.size(); }

    private:
        std::vector<T> m_Components;
        std::vector<Entity> m_Entities;
        std::unordered_map<Entity, size_t> m_EntityToIndex;
    };

}
