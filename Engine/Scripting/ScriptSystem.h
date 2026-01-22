#pragma once

#include "../ECS/System.h"
#include "../ECS/Entity.h"
#include <memory>

namespace Xi {

    class ScriptEngine;
    class World;

    class ScriptSystem : public System {
    public:
        ScriptSystem(ScriptEngine* engine);
        ~ScriptSystem();

        // System interface
        void Update(World& world, float dt) override;
        void Render(World& world, Renderer& renderer) override {}

        // Play mode control
        void StartScripts(World& world);
        void StopScripts(World& world);
        bool IsPlaying() const { return m_IsPlaying; }

        // Hot reload a single script
        void ReloadScript(World& world, Entity entity);

        // Compile a script (for validation)
        bool CompileScript(World& world, Entity entity);

    private:
        void InitializeScript(World& world, Entity entity);
        void CallOnStart(World& world, Entity entity);
        void CallOnUpdate(World& world, Entity entity, float dt);
        void CallOnDestroy(World& world, Entity entity);

        ScriptEngine* m_Engine = nullptr;
        bool m_IsPlaying = false;
    };

}
