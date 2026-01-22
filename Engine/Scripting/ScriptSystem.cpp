#include "ScriptSystem.h"
#include "ScriptEngine.h"
#include "../ECS/World.h"
#include "../ECS/Components/Script.h"
#include "../Core/Log.h"

namespace Xi {

    ScriptSystem::ScriptSystem(ScriptEngine* engine)
        : m_Engine(engine) {}

    ScriptSystem::~ScriptSystem() = default;

    void ScriptSystem::Update(World& world, float dt) {
        if (!m_IsPlaying || !m_Engine) return;

        // Call OnUpdate for all scripts
        world.ForEach<ScriptComponent>([this, &world, dt](Entity entity, ScriptComponent& script) {
            if (script.initialized && !script.hasError) {
                CallOnUpdate(world, entity, dt);
            }
        });
    }

    void ScriptSystem::StartScripts(World& world) {
        if (!m_Engine) return;

        m_IsPlaying = true;
        XI_LOG_INFO("Script System: Starting scripts");

        // Initialize and call OnStart for all scripts
        world.ForEach<ScriptComponent>([this, &world](Entity entity, ScriptComponent& script) {
            if (!script.source.empty()) {
                InitializeScript(world, entity);
                if (script.initialized && !script.hasError) {
                    CallOnStart(world, entity);
                }
            }
        });
    }

    void ScriptSystem::StopScripts(World& world) {
        if (!m_Engine) return;

        XI_LOG_INFO("Script System: Stopping scripts");

        // Call OnDestroy and reset all scripts
        world.ForEach<ScriptComponent>([this, &world](Entity entity, ScriptComponent& script) {
            if (script.initialized && !script.hasError) {
                CallOnDestroy(world, entity);
            }
            script.Reset();
        });

        m_IsPlaying = false;
    }

    void ScriptSystem::ReloadScript(World& world, Entity entity) {
        if (!m_Engine || !world.HasComponent<ScriptComponent>(entity)) return;

        auto& script = world.GetComponent<ScriptComponent>(entity);

        // Call OnDestroy if was running
        if (m_IsPlaying && script.initialized && !script.hasError) {
            CallOnDestroy(world, entity);
        }

        // Reset and reinitialize
        script.Reset();
        InitializeScript(world, entity);

        // Call OnStart if playing
        if (m_IsPlaying && script.initialized && !script.hasError) {
            CallOnStart(world, entity);
        }
    }

    bool ScriptSystem::CompileScript(World& world, Entity entity) {
        if (!m_Engine || !world.HasComponent<ScriptComponent>(entity)) return false;

        auto& script = world.GetComponent<ScriptComponent>(entity);

        // Compile the script
        script.ast = m_Engine->Compile(script.source);

        if (m_Engine->HasError()) {
            script.hasError = true;
            script.lastError = m_Engine->GetError();
            script.errorLine = m_Engine->GetErrorLine();
            return false;
        }

        script.hasError = false;
        script.lastError.clear();
        script.errorLine = -1;
        return true;
    }

    void ScriptSystem::InitializeScript(World& world, Entity entity) {
        if (!m_Engine || !world.HasComponent<ScriptComponent>(entity)) return;

        auto& script = world.GetComponent<ScriptComponent>(entity);
        script.owner = entity;

        // Compile if not already compiled
        if (script.ast.empty() && !script.source.empty()) {
            script.ast = m_Engine->Compile(script.source);

            if (m_Engine->HasError()) {
                script.hasError = true;
                script.lastError = m_Engine->GetError();
                script.errorLine = m_Engine->GetErrorLine();
                XI_LOG_ERROR("Script compile error on " + world.GetEntityName(entity) + ": " + script.lastError);
                return;
            }
        }

        if (script.ast.empty()) {
            return;
        }

        // Create interpreter and register APIs
        script.interpreter = m_Engine->CreateInterpreter();
        m_Engine->RegisterStandardLibrary(*script.interpreter);
        m_Engine->RegisterEngineAPI(*script.interpreter, entity);

        // Execute the script to define functions
        script.interpreter->Execute(script.ast);

        if (script.interpreter->HasError()) {
            script.hasError = true;
            script.lastError = script.interpreter->GetError();
            script.errorLine = script.interpreter->GetErrorLine();
            XI_LOG_ERROR("Script init error on " + world.GetEntityName(entity) + ": " + script.lastError);
            return;
        }

        script.initialized = true;
        script.hasError = false;
        script.lastError.clear();
        script.errorLine = -1;
    }

    void ScriptSystem::CallOnStart(World& world, Entity entity) {
        if (!world.HasComponent<ScriptComponent>(entity)) return;

        auto& script = world.GetComponent<ScriptComponent>(entity);
        if (!script.interpreter || !script.initialized) return;

        if (script.interpreter->HasFunction("OnStart")) {
            script.interpreter->CallFunction("OnStart");

            if (script.interpreter->HasError()) {
                script.hasError = true;
                script.lastError = script.interpreter->GetError();
                script.errorLine = script.interpreter->GetErrorLine();
                XI_LOG_ERROR("OnStart error on " + world.GetEntityName(entity) + ": " + script.lastError);
            }
        }
    }

    void ScriptSystem::CallOnUpdate(World& world, Entity entity, float dt) {
        if (!world.HasComponent<ScriptComponent>(entity)) return;

        auto& script = world.GetComponent<ScriptComponent>(entity);
        if (!script.interpreter || !script.initialized || script.hasError) return;

        if (script.interpreter->HasFunction("OnUpdate")) {
            script.interpreter->CallFunction("OnUpdate", {ScriptValue(static_cast<double>(dt))});

            if (script.interpreter->HasError()) {
                script.hasError = true;
                script.lastError = script.interpreter->GetError();
                script.errorLine = script.interpreter->GetErrorLine();
                XI_LOG_ERROR("OnUpdate error on " + world.GetEntityName(entity) + ": " + script.lastError);
            }
        }
    }

    void ScriptSystem::CallOnDestroy(World& world, Entity entity) {
        if (!world.HasComponent<ScriptComponent>(entity)) return;

        auto& script = world.GetComponent<ScriptComponent>(entity);
        if (!script.interpreter || !script.initialized) return;

        if (script.interpreter->HasFunction("OnDestroy")) {
            script.interpreter->CallFunction("OnDestroy");
            // Don't log errors on destroy, just let it fail silently
        }
    }

}
