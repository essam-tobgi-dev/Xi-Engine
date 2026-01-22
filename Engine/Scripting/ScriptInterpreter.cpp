#include "ScriptInterpreter.h"
#include <cmath>
#include <algorithm>

namespace Xi {

    ScriptInterpreter::ScriptInterpreter() {
        m_CurrentEnv = &m_GlobalEnv;
    }

    ScriptInterpreter::~ScriptInterpreter() = default;

    void ScriptInterpreter::Execute(const std::vector<StmtNodePtr>& statements) {
        for (const auto& stmt : statements) {
            if (HasError() || m_Returning || m_Breaking) break;
            ExecuteStmt(stmt);
        }
    }

    void ScriptInterpreter::SetGlobal(const std::string& name, const ScriptValue& value) {
        m_GlobalEnv.variables[name] = value;
    }

    ScriptValue ScriptInterpreter::GetGlobal(const std::string& name) const {
        auto it = m_GlobalEnv.variables.find(name);
        if (it != m_GlobalEnv.variables.end()) {
            return it->second;
        }
        return ScriptValue();
    }

    bool ScriptInterpreter::HasGlobal(const std::string& name) const {
        return m_GlobalEnv.variables.find(name) != m_GlobalEnv.variables.end();
    }

    ScriptValue ScriptInterpreter::CallFunction(const std::string& name, const std::vector<ScriptValue>& args) {
        ScriptValue func = GetGlobal(name);

        if (func.GetType() == ScriptValue::Type::NativeFunction) {
            return func.AsNativeFunction()(args);
        }

        if (func.GetType() == ScriptValue::Type::Function) {
            int funcIndex = func.GetFunctionIndex();
            if (funcIndex < 0 || funcIndex >= static_cast<int>(m_Functions.size())) {
                RuntimeError("Invalid function reference", 0);
                return ScriptValue();
            }

            const StoredFunction& storedFunc = m_Functions[funcIndex];

            PushScope();

            // Bind parameters
            for (size_t i = 0; i < storedFunc.params.size(); i++) {
                if (i < args.size()) {
                    m_CurrentEnv->variables[storedFunc.params[i]] = args[i];
                } else {
                    m_CurrentEnv->variables[storedFunc.params[i]] = ScriptValue();
                }
            }

            // Execute body
            m_Returning = false;
            m_ReturnValues.clear();
            Execute(storedFunc.body);

            PopScope();

            m_Returning = false;
            if (!m_ReturnValues.empty()) {
                return m_ReturnValues[0];
            }
            return ScriptValue();
        }

        RuntimeError("Attempt to call non-function value '" + name + "'", 0);
        return ScriptValue();
    }

    bool ScriptInterpreter::HasFunction(const std::string& name) const {
        ScriptValue func = GetGlobal(name);
        return func.GetType() == ScriptValue::Type::Function ||
               func.GetType() == ScriptValue::Type::NativeFunction;
    }

    void ScriptInterpreter::Reset() {
        m_GlobalEnv.variables.clear();
        m_Functions.clear();
        m_CurrentEnv = &m_GlobalEnv;
        m_Returning = false;
        m_Breaking = false;
        m_ReturnValues.clear();
        ClearError();
    }

    void ScriptInterpreter::PushScope() {
        Environment* newEnv = new Environment();
        newEnv->parent = m_CurrentEnv;
        m_CurrentEnv = newEnv;
    }

    void ScriptInterpreter::PopScope() {
        if (m_CurrentEnv != &m_GlobalEnv) {
            Environment* old = m_CurrentEnv;
            m_CurrentEnv = m_CurrentEnv->parent;
            delete old;
        }
    }

    void ScriptInterpreter::SetVariable(const std::string& name, const ScriptValue& value) {
        // Search up the scope chain
        Environment* env = m_CurrentEnv;
        while (env != nullptr) {
            auto it = env->variables.find(name);
            if (it != env->variables.end()) {
                it->second = value;
                return;
            }
            env = env->parent;
        }
        // Not found, set in global
        m_GlobalEnv.variables[name] = value;
    }

    ScriptValue ScriptInterpreter::GetVariable(const std::string& name) const {
        Environment* env = m_CurrentEnv;
        while (env != nullptr) {
            auto it = env->variables.find(name);
            if (it != env->variables.end()) {
                return it->second;
            }
            env = env->parent;
        }
        return ScriptValue();
    }

    bool ScriptInterpreter::HasVariable(const std::string& name) const {
        Environment* env = m_CurrentEnv;
        while (env != nullptr) {
            if (env->variables.find(name) != env->variables.end()) {
                return true;
            }
            env = env->parent;
        }
        return false;
    }

    void ScriptInterpreter::ExecuteStmt(const StmtNodePtr& stmt) {
        if (HasError() || m_Returning || m_Breaking) return;

        if (auto* local = dynamic_cast<LocalStmt*>(stmt.get())) {
            ExecuteLocal(local);
        } else if (auto* assign = dynamic_cast<AssignStmt*>(stmt.get())) {
            ExecuteAssign(assign);
        } else if (auto* ifStmt = dynamic_cast<IfStmt*>(stmt.get())) {
            ExecuteIf(ifStmt);
        } else if (auto* whileStmt = dynamic_cast<WhileStmt*>(stmt.get())) {
            ExecuteWhile(whileStmt);
        } else if (auto* repeatStmt = dynamic_cast<RepeatStmt*>(stmt.get())) {
            ExecuteRepeat(repeatStmt);
        } else if (auto* forStmt = dynamic_cast<ForStmt*>(stmt.get())) {
            ExecuteFor(forStmt);
        } else if (auto* forIn = dynamic_cast<ForInStmt*>(stmt.get())) {
            ExecuteForIn(forIn);
        } else if (auto* funcStmt = dynamic_cast<FunctionStmt*>(stmt.get())) {
            ExecuteFunction(funcStmt);
        } else if (auto* retStmt = dynamic_cast<ReturnStmt*>(stmt.get())) {
            ExecuteReturn(retStmt);
        } else if (auto* blockStmt = dynamic_cast<BlockStmt*>(stmt.get())) {
            ExecuteBlock(blockStmt);
        } else if (auto* breakStmt = dynamic_cast<BreakStmt*>(stmt.get())) {
            (void)breakStmt;
            m_Breaking = true;
        } else if (auto* exprStmt = dynamic_cast<ExprStmt*>(stmt.get())) {
            Evaluate(exprStmt->expr);
        }
    }

    void ScriptInterpreter::ExecuteLocal(const LocalStmt* stmt) {
        ScriptValue value;
        if (stmt->initializer) {
            value = Evaluate(stmt->initializer);
        }
        m_CurrentEnv->variables[stmt->name] = value;
    }

    void ScriptInterpreter::ExecuteAssign(const AssignStmt* stmt) {
        ScriptValue value = Evaluate(stmt->value);
        AssignToTarget(stmt->target, value);
    }

    void ScriptInterpreter::ExecuteIf(const IfStmt* stmt) {
        ScriptValue condition = Evaluate(stmt->condition);
        if (condition.IsTrue()) {
            PushScope();
            Execute(stmt->thenBranch);
            PopScope();
            return;
        }

        for (const auto& elseif : stmt->elseifBranches) {
            ScriptValue elifCond = Evaluate(elseif.first);
            if (elifCond.IsTrue()) {
                PushScope();
                Execute(elseif.second);
                PopScope();
                return;
            }
        }

        if (!stmt->elseBranch.empty()) {
            PushScope();
            Execute(stmt->elseBranch);
            PopScope();
        }
    }

    void ScriptInterpreter::ExecuteWhile(const WhileStmt* stmt) {
        int iterations = 0;
        while (Evaluate(stmt->condition).IsTrue()) {
            if (++iterations > MAX_ITERATIONS) {
                RuntimeError("Maximum iterations exceeded in while loop", stmt->line);
                return;
            }
            if (HasError() || m_Returning) return;

            PushScope();
            Execute(stmt->body);
            PopScope();

            if (m_Breaking) {
                m_Breaking = false;
                break;
            }
        }
    }

    void ScriptInterpreter::ExecuteRepeat(const RepeatStmt* stmt) {
        int iterations = 0;
        do {
            if (++iterations > MAX_ITERATIONS) {
                RuntimeError("Maximum iterations exceeded in repeat loop", stmt->line);
                return;
            }
            if (HasError() || m_Returning) return;

            PushScope();
            Execute(stmt->body);
            bool shouldBreak = m_Breaking;
            m_Breaking = false;
            ScriptValue condition = Evaluate(stmt->condition);
            PopScope();

            if (shouldBreak || condition.IsTrue()) break;
        } while (true);
    }

    void ScriptInterpreter::ExecuteFor(const ForStmt* stmt) {
        ScriptValue startVal = Evaluate(stmt->start);
        ScriptValue endVal = Evaluate(stmt->end);
        ScriptValue stepVal = stmt->step ? Evaluate(stmt->step) : ScriptValue(1.0);

        if (!startVal.IsNumber() || !endVal.IsNumber() || !stepVal.IsNumber()) {
            RuntimeError("For loop bounds must be numbers", stmt->line);
            return;
        }

        double current = startVal.AsNumber();
        double end = endVal.AsNumber();
        double step = stepVal.AsNumber();

        if (step == 0) {
            RuntimeError("For loop step cannot be zero", stmt->line);
            return;
        }

        int iterations = 0;
        while ((step > 0 && current <= end) || (step < 0 && current >= end)) {
            if (++iterations > MAX_ITERATIONS) {
                RuntimeError("Maximum iterations exceeded in for loop", stmt->line);
                return;
            }
            if (HasError() || m_Returning) return;

            PushScope();
            m_CurrentEnv->variables[stmt->var] = ScriptValue(current);
            Execute(stmt->body);
            PopScope();

            if (m_Breaking) {
                m_Breaking = false;
                break;
            }

            current += step;
        }
    }

    void ScriptInterpreter::ExecuteForIn(const ForInStmt* stmt) {
        ScriptValue iterator = Evaluate(stmt->iterator);

        if (iterator.IsTable()) {
            const auto& table = iterator.GetTableData();
            for (const auto& [key, value] : table) {
                if (HasError() || m_Returning) return;

                PushScope();
                if (stmt->vars.size() >= 1) {
                    m_CurrentEnv->variables[stmt->vars[0]] = ScriptValue(key);
                }
                if (stmt->vars.size() >= 2) {
                    m_CurrentEnv->variables[stmt->vars[1]] = value;
                }
                Execute(stmt->body);
                PopScope();

                if (m_Breaking) {
                    m_Breaking = false;
                    break;
                }
            }
        }
    }

    void ScriptInterpreter::ExecuteFunction(const FunctionStmt* stmt) {
        StoredFunction func;
        func.params = stmt->params;
        func.body = stmt->body;

        int funcIndex = static_cast<int>(m_Functions.size());
        m_Functions.push_back(func);

        ScriptValue funcValue;
        funcValue.SetFunctionIndex(funcIndex);

        if (stmt->isLocal) {
            m_CurrentEnv->variables[stmt->name] = funcValue;
        } else {
            m_GlobalEnv.variables[stmt->name] = funcValue;
        }
    }

    void ScriptInterpreter::ExecuteReturn(const ReturnStmt* stmt) {
        m_ReturnValues.clear();
        for (const auto& expr : stmt->values) {
            m_ReturnValues.push_back(Evaluate(expr));
        }
        m_Returning = true;
    }

    void ScriptInterpreter::ExecuteBlock(const BlockStmt* stmt) {
        PushScope();
        Execute(stmt->statements);
        PopScope();
    }

    ScriptValue ScriptInterpreter::Evaluate(const ExprNodePtr& expr) {
        if (HasError()) return ScriptValue();

        if (auto* num = dynamic_cast<NumberExpr*>(expr.get())) {
            return EvalNumber(num);
        }
        if (auto* str = dynamic_cast<StringExpr*>(expr.get())) {
            return EvalString(str);
        }
        if (auto* b = dynamic_cast<BoolExpr*>(expr.get())) {
            return EvalBool(b);
        }
        if (auto* n = dynamic_cast<NilExpr*>(expr.get())) {
            return EvalNil(n);
        }
        if (auto* id = dynamic_cast<IdentifierExpr*>(expr.get())) {
            return EvalIdentifier(id);
        }
        if (auto* bin = dynamic_cast<BinaryExpr*>(expr.get())) {
            return EvalBinary(bin);
        }
        if (auto* un = dynamic_cast<UnaryExpr*>(expr.get())) {
            return EvalUnary(un);
        }
        if (auto* call = dynamic_cast<CallExpr*>(expr.get())) {
            return EvalCall(call);
        }
        if (auto* idx = dynamic_cast<IndexExpr*>(expr.get())) {
            return EvalIndex(idx);
        }
        if (auto* mem = dynamic_cast<MemberExpr*>(expr.get())) {
            return EvalMember(mem);
        }
        if (auto* tbl = dynamic_cast<TableExpr*>(expr.get())) {
            return EvalTable(tbl);
        }
        if (auto* func = dynamic_cast<FunctionExpr*>(expr.get())) {
            return EvalFunctionExpr(func);
        }

        return ScriptValue();
    }

    ScriptValue ScriptInterpreter::EvalNumber(const NumberExpr* expr) {
        return ScriptValue(expr->value);
    }

    ScriptValue ScriptInterpreter::EvalString(const StringExpr* expr) {
        return ScriptValue(expr->value);
    }

    ScriptValue ScriptInterpreter::EvalBool(const BoolExpr* expr) {
        return ScriptValue(expr->value);
    }

    ScriptValue ScriptInterpreter::EvalNil(const NilExpr*) {
        return ScriptValue();
    }

    ScriptValue ScriptInterpreter::EvalIdentifier(const IdentifierExpr* expr) {
        return GetVariable(expr->name);
    }

    ScriptValue ScriptInterpreter::EvalBinary(const BinaryExpr* expr) {
        // Short-circuit evaluation for and/or
        if (expr->op == TokenType::And) {
            ScriptValue left = Evaluate(expr->left);
            if (!left.IsTrue()) return left;
            return Evaluate(expr->right);
        }
        if (expr->op == TokenType::Or) {
            ScriptValue left = Evaluate(expr->left);
            if (left.IsTrue()) return left;
            return Evaluate(expr->right);
        }

        ScriptValue left = Evaluate(expr->left);
        ScriptValue right = Evaluate(expr->right);

        switch (expr->op) {
            case TokenType::Plus: return left + right;
            case TokenType::Minus: return left - right;
            case TokenType::Star: return left * right;
            case TokenType::Slash: return left / right;
            case TokenType::Percent:
                if (left.IsNumber() && right.IsNumber()) {
                    return ScriptValue(std::fmod(left.AsNumber(), right.AsNumber()));
                }
                return ScriptValue();
            case TokenType::Caret:
                if (left.IsNumber() && right.IsNumber()) {
                    return ScriptValue(std::pow(left.AsNumber(), right.AsNumber()));
                }
                return ScriptValue();
            case TokenType::Concat:
                return ScriptValue(left.ToString() + right.ToString());
            case TokenType::EqualEqual: return ScriptValue(left == right);
            case TokenType::NotEqual: return ScriptValue(left != right);
            case TokenType::Less: return ScriptValue(left < right);
            case TokenType::LessEqual: return ScriptValue(left <= right);
            case TokenType::Greater: return ScriptValue(left > right);
            case TokenType::GreaterEqual: return ScriptValue(left >= right);
            default:
                RuntimeError("Unknown binary operator", expr->line);
                return ScriptValue();
        }
    }

    ScriptValue ScriptInterpreter::EvalUnary(const UnaryExpr* expr) {
        ScriptValue operand = Evaluate(expr->operand);

        switch (expr->op) {
            case TokenType::Minus:
                return -operand;
            case TokenType::Not:
                return ScriptValue(!operand.IsTrue());
            case TokenType::Hash:
                if (operand.IsString()) {
                    return ScriptValue(static_cast<double>(operand.AsString().size()));
                }
                if (operand.IsTable()) {
                    return ScriptValue(static_cast<double>(operand.GetTableData().size()));
                }
                return ScriptValue(0.0);
            default:
                return ScriptValue();
        }
    }

    ScriptValue ScriptInterpreter::EvalCall(const CallExpr* expr) {
        ScriptValue callee = Evaluate(expr->callee);

        std::vector<ScriptValue> args;
        for (const auto& arg : expr->arguments) {
            args.push_back(Evaluate(arg));
        }

        if (callee.GetType() == ScriptValue::Type::NativeFunction) {
            try {
                return callee.AsNativeFunction()(args);
            } catch (const std::exception& e) {
                RuntimeError(e.what(), expr->line);
                return ScriptValue();
            }
        }

        if (callee.GetType() == ScriptValue::Type::Function) {
            int funcIndex = callee.GetFunctionIndex();
            if (funcIndex < 0 || funcIndex >= static_cast<int>(m_Functions.size())) {
                RuntimeError("Invalid function reference", expr->line);
                return ScriptValue();
            }

            const StoredFunction& storedFunc = m_Functions[funcIndex];

            PushScope();

            // Bind parameters
            for (size_t i = 0; i < storedFunc.params.size(); i++) {
                if (i < args.size()) {
                    m_CurrentEnv->variables[storedFunc.params[i]] = args[i];
                } else {
                    m_CurrentEnv->variables[storedFunc.params[i]] = ScriptValue();
                }
            }

            // Execute body
            bool wasReturning = m_Returning;
            m_Returning = false;
            m_ReturnValues.clear();
            Execute(storedFunc.body);

            PopScope();

            ScriptValue result;
            if (!m_ReturnValues.empty()) {
                result = m_ReturnValues[0];
            }
            m_Returning = wasReturning;
            m_ReturnValues.clear();

            return result;
        }

        RuntimeError("Attempt to call non-function value", expr->line);
        return ScriptValue();
    }

    ScriptValue ScriptInterpreter::EvalIndex(const IndexExpr* expr) {
        ScriptValue object = Evaluate(expr->object);
        ScriptValue index = Evaluate(expr->index);

        if (object.IsTable()) {
            if (index.IsString()) {
                return object.GetTable(index.AsString());
            } else if (index.IsNumber()) {
                return object.GetTable(std::to_string(static_cast<int>(index.AsNumber())));
            }
        }

        return ScriptValue();
    }

    ScriptValue ScriptInterpreter::EvalMember(const MemberExpr* expr) {
        ScriptValue object = Evaluate(expr->object);

        if (object.IsTable()) {
            return object.GetTable(expr->member);
        }

        if (object.IsVec3()) {
            if (expr->member == "x") return ScriptValue(static_cast<double>(object.AsVec3().x));
            if (expr->member == "y") return ScriptValue(static_cast<double>(object.AsVec3().y));
            if (expr->member == "z") return ScriptValue(static_cast<double>(object.AsVec3().z));
        }

        return ScriptValue();
    }

    ScriptValue ScriptInterpreter::EvalTable(const TableExpr* expr) {
        ScriptValue table = ScriptValue::CreateTable();

        for (const auto& entry : expr->entries) {
            ScriptValue key = Evaluate(entry.first);
            ScriptValue value = Evaluate(entry.second);

            if (key.IsNumber()) {
                table.SetTable(std::to_string(static_cast<int>(key.AsNumber())), value);
            } else {
                table.SetTable(key.ToString(), value);
            }
        }

        return table;
    }

    ScriptValue ScriptInterpreter::EvalFunctionExpr(const FunctionExpr* expr) {
        StoredFunction func;
        func.params = expr->params;
        func.body = expr->body;

        int funcIndex = static_cast<int>(m_Functions.size());
        m_Functions.push_back(func);

        ScriptValue funcValue;
        funcValue.SetFunctionIndex(funcIndex);
        return funcValue;
    }

    void ScriptInterpreter::AssignToTarget(const ExprNodePtr& target, const ScriptValue& value) {
        if (auto* id = dynamic_cast<IdentifierExpr*>(target.get())) {
            SetVariable(id->name, value);
        } else if (auto* idx = dynamic_cast<IndexExpr*>(target.get())) {
            ScriptValue object = Evaluate(idx->object);
            ScriptValue index = Evaluate(idx->index);
            if (object.IsTable()) {
                if (index.IsString()) {
                    object.GetTableData()[index.AsString()] = value;
                } else if (index.IsNumber()) {
                    object.GetTableData()[std::to_string(static_cast<int>(index.AsNumber()))] = value;
                }
                // Update the variable
                if (auto* objId = dynamic_cast<IdentifierExpr*>(idx->object.get())) {
                    SetVariable(objId->name, object);
                }
            }
        } else if (auto* mem = dynamic_cast<MemberExpr*>(target.get())) {
            ScriptValue object = Evaluate(mem->object);
            if (object.IsTable()) {
                object.GetTableData()[mem->member] = value;
                // Update the variable
                if (auto* objId = dynamic_cast<IdentifierExpr*>(mem->object.get())) {
                    SetVariable(objId->name, object);
                }
            } else if (object.IsVec3()) {
                glm::vec3& vec = const_cast<glm::vec3&>(object.AsVec3());
                if (value.IsNumber()) {
                    float f = static_cast<float>(value.AsNumber());
                    if (mem->member == "x") vec.x = f;
                    else if (mem->member == "y") vec.y = f;
                    else if (mem->member == "z") vec.z = f;
                    // Update the variable
                    if (auto* objId = dynamic_cast<IdentifierExpr*>(mem->object.get())) {
                        SetVariable(objId->name, ScriptValue(vec));
                    }
                }
            }
        } else {
            RuntimeError("Invalid assignment target", target->line);
        }
    }

    void ScriptInterpreter::RuntimeError(const std::string& message, int line) {
        if (m_Error.empty()) {
            m_Error = message + " at line " + std::to_string(line);
            m_ErrorLine = line;
        }
    }

}
