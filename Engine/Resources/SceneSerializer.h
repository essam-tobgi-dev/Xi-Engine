#pragma once

#include <string>

namespace Xi {

    class World;

    class SceneSerializer {
    public:
        SceneSerializer(World& world);

        bool Save(const std::string& filepath);
        bool Load(const std::string& filepath);

    private:
        World& m_World;
    };

}
