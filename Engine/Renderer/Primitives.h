#pragma once

#include <memory>

namespace Xi {

    class Mesh;

    class Primitives {
    public:
        static std::shared_ptr<Mesh> CreateCube();
        static std::shared_ptr<Mesh> CreateSphere(int segments = 32, int rings = 16);
        static std::shared_ptr<Mesh> CreatePlane(float size = 10.0f);
        static std::shared_ptr<Mesh> CreateQuad();
        static std::shared_ptr<Mesh> CreateCylinder(int segments = 32);
        static std::shared_ptr<Mesh> CreateCone(int segments = 32);
    };

}
