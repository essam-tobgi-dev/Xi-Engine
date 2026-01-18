#pragma once

#include <cstdint>
#include <limits>

namespace Xi {

    using Entity = uint32_t;
    constexpr Entity INVALID_ENTITY = std::numeric_limits<Entity>::max();

    // Component type ID generation
    using ComponentTypeID = uint32_t;

    inline ComponentTypeID GetNextComponentTypeID() {
        static ComponentTypeID lastID = 0;
        return lastID++;
    }

    template<typename T>
    inline ComponentTypeID GetComponentTypeID() {
        static ComponentTypeID typeID = GetNextComponentTypeID();
        return typeID;
    }

    constexpr size_t MAX_COMPONENTS = 64;

}
