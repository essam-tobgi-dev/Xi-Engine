#pragma once

namespace Xi {

    class World;
    class Renderer;

    class System {
    public:
        virtual ~System() = default;

        virtual void Update(World& world, float dt) { (void)world; (void)dt; }
        virtual void Render(World& world, Renderer& renderer) { (void)world; (void)renderer; }

        void SetEnabled(bool enabled) { m_Enabled = enabled; }
        bool IsEnabled() const { return m_Enabled; }

    protected:
        bool m_Enabled = true;
    };

}
