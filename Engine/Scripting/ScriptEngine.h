#pragma once

#include "ScriptInterpreter.h"
#include "ScriptLexer.h"
#include "ScriptParser.h"
#include "../ECS/Entity.h"
#include <memory>
#include <string>

namespace Xi {

    class World;

    class ScriptEngine {
    public:
        ScriptEngine();
        ~ScriptEngine();

        // Initialize with world reference for entity operations
        void Init(World* world);
        void Shutdown();

        // Create a new interpreter instance for a script
        std::unique_ptr<ScriptInterpreter> CreateInterpreter();

        // Compile script source code and return AST
        // Returns empty vector on error, check GetError()
        std::vector<StmtNodePtr> Compile(const std::string& source);

        // Validate script without executing
        bool Validate(const std::string& source);

        // Register standard library functions on an interpreter
        void RegisterStandardLibrary(ScriptInterpreter& interp);

        // Register engine API (Input, Time, Log, Vec3, etc.)
        void RegisterEngineAPI(ScriptInterpreter& interp, Entity entity);

        // Error handling
        bool HasError() const { return !m_Error.empty(); }
        const std::string& GetError() const { return m_Error; }
        int GetErrorLine() const { return m_ErrorLine; }
        void ClearError() { m_Error.clear(); m_ErrorLine = -1; }

        World* GetWorld() const { return m_World; }

    private:
        void RegisterMathLibrary(ScriptInterpreter& interp);
        void RegisterStringLibrary(ScriptInterpreter& interp);
        void RegisterTableLibrary(ScriptInterpreter& interp);
        void RegisterInputAPI(ScriptInterpreter& interp);
        void RegisterTimeAPI(ScriptInterpreter& interp);
        void RegisterLogAPI(ScriptInterpreter& interp);
        void RegisterVec3API(ScriptInterpreter& interp);
        void RegisterWorldAPI(ScriptInterpreter& interp);
        void RegisterEntityAPI(ScriptInterpreter& interp, Entity entity);

        World* m_World = nullptr;
        std::string m_Error;
        int m_ErrorLine = -1;
    };

}
