#pragma once

#include "../Entity.h"
#include "../../Scripting/ScriptInterpreter.h"
#include "../../Scripting/ScriptAST.h"
#include <string>
#include <memory>
#include <vector>

namespace Xi {

    struct ScriptComponent {
        // Script source code
        std::string source;

        // Optional file path (for file-based scripts)
        std::string filepath;

        // Compiled AST (cached)
        std::vector<StmtNodePtr> ast;

        // Runtime interpreter instance
        std::unique_ptr<ScriptInterpreter> interpreter;

        // Script state
        bool initialized = false;
        bool hasError = false;
        std::string lastError;
        int errorLine = -1;

        // Owner entity (set at runtime)
        Entity owner = INVALID_ENTITY;

        // Reset runtime state (for hot reload or stop)
        void Reset() {
            interpreter.reset();
            initialized = false;
            hasError = false;
            lastError.clear();
            errorLine = -1;
        }

        // Clear everything including source
        void Clear() {
            source.clear();
            filepath.clear();
            ast.clear();
            Reset();
        }
    };

}
