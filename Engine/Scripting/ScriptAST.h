#pragma once

#include "ScriptLexer.h"
#include <memory>
#include <vector>

namespace Xi {

    // Forward declarations
    struct ASTNode;
    struct ExprNode;
    struct StmtNode;

    using ASTNodePtr = std::shared_ptr<ASTNode>;
    using ExprNodePtr = std::shared_ptr<ExprNode>;
    using StmtNodePtr = std::shared_ptr<StmtNode>;

    // Base AST node
    struct ASTNode {
        int line = 0;
        virtual ~ASTNode() = default;
    };

    // Expression base
    struct ExprNode : ASTNode {};

    // Statement base
    struct StmtNode : ASTNode {};

    // ============ Expressions ============

    struct NumberExpr : ExprNode {
        double value;
        NumberExpr(double v) : value(v) {}
    };

    struct StringExpr : ExprNode {
        std::string value;
        StringExpr(const std::string& v) : value(v) {}
    };

    struct BoolExpr : ExprNode {
        bool value;
        BoolExpr(bool v) : value(v) {}
    };

    struct NilExpr : ExprNode {};

    struct IdentifierExpr : ExprNode {
        std::string name;
        IdentifierExpr(const std::string& n) : name(n) {}
    };

    struct BinaryExpr : ExprNode {
        ExprNodePtr left;
        TokenType op;
        ExprNodePtr right;
        BinaryExpr(ExprNodePtr l, TokenType o, ExprNodePtr r) : left(l), op(o), right(r) {}
    };

    struct UnaryExpr : ExprNode {
        TokenType op;
        ExprNodePtr operand;
        UnaryExpr(TokenType o, ExprNodePtr e) : op(o), operand(e) {}
    };

    struct CallExpr : ExprNode {
        ExprNodePtr callee;
        std::vector<ExprNodePtr> arguments;
        CallExpr(ExprNodePtr c, std::vector<ExprNodePtr> args) : callee(c), arguments(std::move(args)) {}
    };

    struct IndexExpr : ExprNode {
        ExprNodePtr object;
        ExprNodePtr index;
        IndexExpr(ExprNodePtr obj, ExprNodePtr idx) : object(obj), index(idx) {}
    };

    struct MemberExpr : ExprNode {
        ExprNodePtr object;
        std::string member;
        MemberExpr(ExprNodePtr obj, const std::string& m) : object(obj), member(m) {}
    };

    struct TableExpr : ExprNode {
        std::vector<std::pair<ExprNodePtr, ExprNodePtr>> entries; // key-value pairs
    };

    struct FunctionExpr : ExprNode {
        std::vector<std::string> params;
        std::vector<StmtNodePtr> body;
    };

    // ============ Statements ============

    struct ExprStmt : StmtNode {
        ExprNodePtr expr;
        ExprStmt(ExprNodePtr e) : expr(e) {}
    };

    struct LocalStmt : StmtNode {
        std::string name;
        ExprNodePtr initializer;
        LocalStmt(const std::string& n, ExprNodePtr init = nullptr) : name(n), initializer(init) {}
    };

    struct AssignStmt : StmtNode {
        ExprNodePtr target;
        ExprNodePtr value;
        AssignStmt(ExprNodePtr t, ExprNodePtr v) : target(t), value(v) {}
    };

    struct IfStmt : StmtNode {
        ExprNodePtr condition;
        std::vector<StmtNodePtr> thenBranch;
        std::vector<std::pair<ExprNodePtr, std::vector<StmtNodePtr>>> elseifBranches;
        std::vector<StmtNodePtr> elseBranch;
    };

    struct WhileStmt : StmtNode {
        ExprNodePtr condition;
        std::vector<StmtNodePtr> body;
    };

    struct RepeatStmt : StmtNode {
        std::vector<StmtNodePtr> body;
        ExprNodePtr condition;
    };

    struct ForStmt : StmtNode {
        std::string var;
        ExprNodePtr start;
        ExprNodePtr end;
        ExprNodePtr step;
        std::vector<StmtNodePtr> body;
    };

    struct ForInStmt : StmtNode {
        std::vector<std::string> vars;
        ExprNodePtr iterator;
        std::vector<StmtNodePtr> body;
    };

    struct FunctionStmt : StmtNode {
        std::string name;
        std::vector<std::string> params;
        std::vector<StmtNodePtr> body;
        bool isLocal = false;
    };

    struct ReturnStmt : StmtNode {
        std::vector<ExprNodePtr> values;
    };

    struct BreakStmt : StmtNode {};

    struct BlockStmt : StmtNode {
        std::vector<StmtNodePtr> statements;
    };

}
