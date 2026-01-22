#pragma once

#include "ScriptValue.h"
#include "ScriptAST.h"
#include <unordered_map>
#include <vector>
#include <functional>

namespace Xi {

    class ScriptInterpreter {
    public:
        ScriptInterpreter();
        ~ScriptInterpreter();

        // Execute a list of statements
        void Execute(const std::vector<StmtNodePtr>& statements);

        // Global environment access
        void SetGlobal(const std::string& name, const ScriptValue& value);
        ScriptValue GetGlobal(const std::string& name) const;
        bool HasGlobal(const std::string& name) const;

        // Call a function by name
        ScriptValue CallFunction(const std::string& name, const std::vector<ScriptValue>& args = {});
        bool HasFunction(const std::string& name) const;

        // Error handling
        bool HasError() const { return !m_Error.empty(); }
        const std::string& GetError() const { return m_Error; }
        int GetErrorLine() const { return m_ErrorLine; }
        void ClearError() { m_Error.clear(); m_ErrorLine = -1; }

        // Reset state
        void Reset();

    private:
        // Environment (scope) management
        struct Environment {
            std::unordered_map<std::string, ScriptValue> variables;
            Environment* parent = nullptr;
        };

        void PushScope();
        void PopScope();
        void SetVariable(const std::string& name, const ScriptValue& value);
        ScriptValue GetVariable(const std::string& name) const;
        bool HasVariable(const std::string& name) const;

        // Statement execution
        void ExecuteStmt(const StmtNodePtr& stmt);
        void ExecuteLocal(const LocalStmt* stmt);
        void ExecuteAssign(const AssignStmt* stmt);
        void ExecuteIf(const IfStmt* stmt);
        void ExecuteWhile(const WhileStmt* stmt);
        void ExecuteRepeat(const RepeatStmt* stmt);
        void ExecuteFor(const ForStmt* stmt);
        void ExecuteForIn(const ForInStmt* stmt);
        void ExecuteFunction(const FunctionStmt* stmt);
        void ExecuteReturn(const ReturnStmt* stmt);
        void ExecuteBlock(const BlockStmt* stmt);

        // Expression evaluation
        ScriptValue Evaluate(const ExprNodePtr& expr);
        ScriptValue EvalNumber(const NumberExpr* expr);
        ScriptValue EvalString(const StringExpr* expr);
        ScriptValue EvalBool(const BoolExpr* expr);
        ScriptValue EvalNil(const NilExpr* expr);
        ScriptValue EvalIdentifier(const IdentifierExpr* expr);
        ScriptValue EvalBinary(const BinaryExpr* expr);
        ScriptValue EvalUnary(const UnaryExpr* expr);
        ScriptValue EvalCall(const CallExpr* expr);
        ScriptValue EvalIndex(const IndexExpr* expr);
        ScriptValue EvalMember(const MemberExpr* expr);
        ScriptValue EvalTable(const TableExpr* expr);
        ScriptValue EvalFunctionExpr(const FunctionExpr* expr);

        // Assignment target evaluation
        void AssignToTarget(const ExprNodePtr& target, const ScriptValue& value);

        void RuntimeError(const std::string& message, int line);

        Environment m_GlobalEnv;
        Environment* m_CurrentEnv = nullptr;

        // For storing function definitions
        struct StoredFunction {
            std::vector<std::string> params;
            std::vector<StmtNodePtr> body;
        };
        std::vector<StoredFunction> m_Functions;

        // Control flow
        bool m_Returning = false;
        bool m_Breaking = false;
        std::vector<ScriptValue> m_ReturnValues;

        std::string m_Error;
        int m_ErrorLine = -1;

        // Max iterations to prevent infinite loops
        static constexpr int MAX_ITERATIONS = 1000000;
    };

}
