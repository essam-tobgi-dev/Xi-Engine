#pragma once

#include <memory>

namespace Xi {

    class Mesh;
    class Material;

    struct MeshRenderer {
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<Material> material;
        bool castShadows = true;
        bool receiveShadows = true;
        bool visible = true;
    };

}
