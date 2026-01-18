#pragma once

#include <memory>
#include <glm/glm.hpp>

namespace Xi {

    class Texture;
    class Material;

    struct SpriteRenderer {
        std::shared_ptr<Texture> texture;
        std::shared_ptr<Material> material;
        glm::vec4 color = glm::vec4(1.0f);
        glm::vec2 tiling = glm::vec2(1.0f);
        glm::vec2 offset = glm::vec2(0.0f);
        int sortingOrder = 0;
        bool flipX = false;
        bool flipY = false;
        bool visible = true;
    };

}
