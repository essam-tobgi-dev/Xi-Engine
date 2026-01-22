#include "ScriptParser.h"
#include <stdexcept>

namespace Xi {

    ScriptParser::ScriptParser(const std::vector<Token>& tokens)
        : m_Tokens(tokens) {}

    std::vector<StmtNodePtr> ScriptParser::Parse() {
        std::vector<StmtNodePtr> statements;

        while (!IsAtEnd() && m_Error.empty()) {
            auto stmt = ParseStatement();
            if (stmt) {
                statements.push_back(stmt);
            }
        }

        return statements;
    }

    StmtNodePtr ScriptParser::ParseStatement() {
        if (Match(TokenType::Local)) {
            if (Check(TokenType::Function)) {
                return ParseFunctionStatement(true);
            }
            return ParseLocalStatement();
        }
        if (Match(TokenType::Function)) {
            return ParseFunctionStatement(false);
        }
        if (Match(TokenType::If)) {
            return ParseIfStatement();
        }
        if (Match(TokenType::While)) {
            return ParseWhileStatement();
        }
        if (Match(TokenType::Repeat)) {
            return ParseRepeatStatement();
        }
        if (Match(TokenType::For)) {
            return ParseForStatement();
        }
        if (Match(TokenType::Return)) {
            return ParseReturnStatement();
        }
        if (Match(TokenType::Break)) {
            return std::make_shared<BreakStmt>();
        }
        if (Match(TokenType::Do)) {
            auto block = std::make_shared<BlockStmt>();
            block->statements = ParseBlock();
            Consume(TokenType::End, "Expected 'end' after block");
            return block;
        }

        return ParseExpressionStatement();
    }

    StmtNodePtr ScriptParser::ParseLocalStatement() {
        Token name = Consume(TokenType::Identifier, "Expected variable name");
        ExprNodePtr init = nullptr;

        if (Match(TokenType::Equal)) {
            init = ParseExpression();
        }

        auto stmt = std::make_shared<LocalStmt>(name.value, init);
        stmt->line = name.line;
        return stmt;
    }

    StmtNodePtr ScriptParser::ParseFunctionStatement(bool isLocal) {
        Match(TokenType::Function);  // consume 'function' if not yet consumed

        Token name = Consume(TokenType::Identifier, "Expected function name");
        Consume(TokenType::LeftParen, "Expected '(' after function name");

        std::vector<std::string> params;
        if (!Check(TokenType::RightParen)) {
            do {
                Token param = Consume(TokenType::Identifier, "Expected parameter name");
                params.push_back(param.value);
            } while (Match(TokenType::Comma));
        }
        Consume(TokenType::RightParen, "Expected ')' after parameters");

        auto body = ParseBlock();
        Consume(TokenType::End, "Expected 'end' after function body");

        auto stmt = std::make_shared<FunctionStmt>();
        stmt->name = name.value;
        stmt->params = params;
        stmt->body = body;
        stmt->isLocal = isLocal;
        stmt->line = name.line;
        return stmt;
    }

    StmtNodePtr ScriptParser::ParseIfStatement() {
        auto stmt = std::make_shared<IfStmt>();
        stmt->line = Previous().line;

        stmt->condition = ParseExpression();
        Consume(TokenType::Then, "Expected 'then' after if condition");
        stmt->thenBranch = ParseBlock();

        while (Match(TokenType::ElseIf)) {
            auto cond = ParseExpression();
            Consume(TokenType::Then, "Expected 'then' after elseif condition");
            auto branch = ParseBlock();
            stmt->elseifBranches.push_back({cond, branch});
        }

        if (Match(TokenType::Else)) {
            stmt->elseBranch = ParseBlock();
        }

        Consume(TokenType::End, "Expected 'end' after if statement");
        return stmt;
    }

    StmtNodePtr ScriptParser::ParseWhileStatement() {
        auto stmt = std::make_shared<WhileStmt>();
        stmt->line = Previous().line;

        stmt->condition = ParseExpression();
        Consume(TokenType::Do, "Expected 'do' after while condition");
        stmt->body = ParseBlock();
        Consume(TokenType::End, "Expected 'end' after while body");

        return stmt;
    }

    StmtNodePtr ScriptParser::ParseRepeatStatement() {
        auto stmt = std::make_shared<RepeatStmt>();
        stmt->line = Previous().line;

        stmt->body = ParseBlock();
        Consume(TokenType::Until, "Expected 'until' after repeat body");
        stmt->condition = ParseExpression();

        return stmt;
    }

    StmtNodePtr ScriptParser::ParseForStatement() {
        Token var = Consume(TokenType::Identifier, "Expected variable name");

        if (Match(TokenType::Equal)) {
            // Numeric for: for i = start, end, step do
            auto stmt = std::make_shared<ForStmt>();
            stmt->var = var.value;
            stmt->line = var.line;

            stmt->start = ParseExpression();
            Consume(TokenType::Comma, "Expected ',' after for start value");
            stmt->end = ParseExpression();

            if (Match(TokenType::Comma)) {
                stmt->step = ParseExpression();
            }

            Consume(TokenType::Do, "Expected 'do' in for statement");
            stmt->body = ParseBlock();
            Consume(TokenType::End, "Expected 'end' after for body");

            return stmt;
        } else if (Match(TokenType::Comma) || Check(TokenType::In)) {
            // For-in: for k, v in pairs(t) do
            auto stmt = std::make_shared<ForInStmt>();
            stmt->line = var.line;
            stmt->vars.push_back(var.value);

            while (Previous().type == TokenType::Comma) {
                Token nextVar = Consume(TokenType::Identifier, "Expected variable name");
                stmt->vars.push_back(nextVar.value);
                if (!Match(TokenType::Comma)) break;
            }

            Consume(TokenType::In, "Expected 'in' in for statement");
            stmt->iterator = ParseExpression();
            Consume(TokenType::Do, "Expected 'do' in for statement");
            stmt->body = ParseBlock();
            Consume(TokenType::End, "Expected 'end' after for body");

            return stmt;
        }

        Error("Invalid for statement");
        return nullptr;
    }

    StmtNodePtr ScriptParser::ParseReturnStatement() {
        auto stmt = std::make_shared<ReturnStmt>();
        stmt->line = Previous().line;

        if (!Check(TokenType::End) && !Check(TokenType::Else) &&
            !Check(TokenType::ElseIf) && !Check(TokenType::Until) &&
            !IsAtEnd()) {
            do {
                stmt->values.push_back(ParseExpression());
            } while (Match(TokenType::Comma));
        }

        return stmt;
    }

    StmtNodePtr ScriptParser::ParseExpressionStatement() {
        ExprNodePtr expr = ParseExpression();

        // Check for assignment
        if (Match(TokenType::Equal)) {
            ExprNodePtr value = ParseExpression();
            auto stmt = std::make_shared<AssignStmt>(expr, value);
            stmt->line = expr->line;
            return stmt;
        }

        auto stmt = std::make_shared<ExprStmt>(expr);
        stmt->line = expr->line;
        return stmt;
    }

    std::vector<StmtNodePtr> ScriptParser::ParseBlock() {
        std::vector<StmtNodePtr> statements;

        while (!Check(TokenType::End) && !Check(TokenType::Else) &&
               !Check(TokenType::ElseIf) && !Check(TokenType::Until) &&
               !IsAtEnd() && m_Error.empty()) {
            auto stmt = ParseStatement();
            if (stmt) {
                statements.push_back(stmt);
            }
        }

        return statements;
    }

    ExprNodePtr ScriptParser::ParseExpression() {
        return ParseOr();
    }

    ExprNodePtr ScriptParser::ParseOr() {
        auto left = ParseAnd();

        while (Match(TokenType::Or)) {
            int line = Previous().line;
            auto right = ParseAnd();
            auto expr = std::make_shared<BinaryExpr>(left, TokenType::Or, right);
            expr->line = line;
            left = expr;
        }

        return left;
    }

    ExprNodePtr ScriptParser::ParseAnd() {
        auto left = ParseComparison();

        while (Match(TokenType::And)) {
            int line = Previous().line;
            auto right = ParseComparison();
            auto expr = std::make_shared<BinaryExpr>(left, TokenType::And, right);
            expr->line = line;
            left = expr;
        }

        return left;
    }

    ExprNodePtr ScriptParser::ParseComparison() {
        auto left = ParseConcat();

        while (Match({TokenType::Less, TokenType::LessEqual, TokenType::Greater,
                      TokenType::GreaterEqual, TokenType::EqualEqual, TokenType::NotEqual})) {
            TokenType op = Previous().type;
            int line = Previous().line;
            auto right = ParseConcat();
            auto expr = std::make_shared<BinaryExpr>(left, op, right);
            expr->line = line;
            left = expr;
        }

        return left;
    }

    ExprNodePtr ScriptParser::ParseConcat() {
        auto left = ParseAddSub();

        while (Match(TokenType::Concat)) {
            int line = Previous().line;
            auto right = ParseAddSub();
            auto expr = std::make_shared<BinaryExpr>(left, TokenType::Concat, right);
            expr->line = line;
            left = expr;
        }

        return left;
    }

    ExprNodePtr ScriptParser::ParseAddSub() {
        auto left = ParseMulDiv();

        while (Match({TokenType::Plus, TokenType::Minus})) {
            TokenType op = Previous().type;
            int line = Previous().line;
            auto right = ParseMulDiv();
            auto expr = std::make_shared<BinaryExpr>(left, op, right);
            expr->line = line;
            left = expr;
        }

        return left;
    }

    ExprNodePtr ScriptParser::ParseMulDiv() {
        auto left = ParseUnary();

        while (Match({TokenType::Star, TokenType::Slash, TokenType::Percent})) {
            TokenType op = Previous().type;
            int line = Previous().line;
            auto right = ParseUnary();
            auto expr = std::make_shared<BinaryExpr>(left, op, right);
            expr->line = line;
            left = expr;
        }

        return left;
    }

    ExprNodePtr ScriptParser::ParseUnary() {
        if (Match({TokenType::Minus, TokenType::Not, TokenType::Hash})) {
            TokenType op = Previous().type;
            int line = Previous().line;
            auto operand = ParseUnary();
            auto expr = std::make_shared<UnaryExpr>(op, operand);
            expr->line = line;
            return expr;
        }

        return ParsePower();
    }

    ExprNodePtr ScriptParser::ParsePower() {
        auto left = ParsePostfix();

        if (Match(TokenType::Caret)) {
            int line = Previous().line;
            auto right = ParseUnary();  // Right-associative
            auto expr = std::make_shared<BinaryExpr>(left, TokenType::Caret, right);
            expr->line = line;
            return expr;
        }

        return left;
    }

    ExprNodePtr ScriptParser::ParsePostfix() {
        auto expr = ParsePrimary();

        while (true) {
            if (Match(TokenType::LeftParen)) {
                // Function call
                std::vector<ExprNodePtr> args;
                if (!Check(TokenType::RightParen)) {
                    do {
                        args.push_back(ParseExpression());
                    } while (Match(TokenType::Comma));
                }
                Consume(TokenType::RightParen, "Expected ')' after arguments");
                auto call = std::make_shared<CallExpr>(expr, args);
                call->line = expr->line;
                expr = call;
            } else if (Match(TokenType::LeftBracket)) {
                // Index access
                auto index = ParseExpression();
                Consume(TokenType::RightBracket, "Expected ']' after index");
                auto indexExpr = std::make_shared<IndexExpr>(expr, index);
                indexExpr->line = expr->line;
                expr = indexExpr;
            } else if (Match(TokenType::Dot)) {
                // Member access
                Token name = Consume(TokenType::Identifier, "Expected member name");
                auto member = std::make_shared<MemberExpr>(expr, name.value);
                member->line = expr->line;
                expr = member;
            } else if (Match(TokenType::Colon)) {
                // Method call (obj:method(args) -> obj.method(obj, args))
                Token name = Consume(TokenType::Identifier, "Expected method name");
                Consume(TokenType::LeftParen, "Expected '(' after method name");

                std::vector<ExprNodePtr> args;
                args.push_back(expr);  // self as first argument
                if (!Check(TokenType::RightParen)) {
                    do {
                        args.push_back(ParseExpression());
                    } while (Match(TokenType::Comma));
                }
                Consume(TokenType::RightParen, "Expected ')' after arguments");

                auto member = std::make_shared<MemberExpr>(expr, name.value);
                auto call = std::make_shared<CallExpr>(member, args);
                call->line = expr->line;
                expr = call;
            } else {
                break;
            }
        }

        return expr;
    }

    ExprNodePtr ScriptParser::ParsePrimary() {
        if (Match(TokenType::Number)) {
            auto expr = std::make_shared<NumberExpr>(std::stod(Previous().value));
            expr->line = Previous().line;
            return expr;
        }

        if (Match(TokenType::String)) {
            auto expr = std::make_shared<StringExpr>(Previous().value);
            expr->line = Previous().line;
            return expr;
        }

        if (Match(TokenType::True)) {
            auto expr = std::make_shared<BoolExpr>(true);
            expr->line = Previous().line;
            return expr;
        }

        if (Match(TokenType::False)) {
            auto expr = std::make_shared<BoolExpr>(false);
            expr->line = Previous().line;
            return expr;
        }

        if (Match(TokenType::Nil)) {
            auto expr = std::make_shared<NilExpr>();
            expr->line = Previous().line;
            return expr;
        }

        if (Match(TokenType::Identifier)) {
            auto expr = std::make_shared<IdentifierExpr>(Previous().value);
            expr->line = Previous().line;
            return expr;
        }

        if (Match(TokenType::LeftParen)) {
            auto expr = ParseExpression();
            Consume(TokenType::RightParen, "Expected ')' after expression");
            return expr;
        }

        if (Match(TokenType::LeftBrace)) {
            return ParseTableConstructor();
        }

        if (Match(TokenType::Function)) {
            return ParseFunctionExpression();
        }

        Error("Expected expression");
        return std::make_shared<NilExpr>();
    }

    ExprNodePtr ScriptParser::ParseTableConstructor() {
        auto table = std::make_shared<TableExpr>();
        table->line = Previous().line;

        int arrayIndex = 1;
        if (!Check(TokenType::RightBrace)) {
            do {
                if (Check(TokenType::RightBrace)) break;

                ExprNodePtr key;
                ExprNodePtr value;

                if (Match(TokenType::LeftBracket)) {
                    key = ParseExpression();
                    Consume(TokenType::RightBracket, "Expected ']' after key");
                    Consume(TokenType::Equal, "Expected '=' after key");
                    value = ParseExpression();
                } else if (Check(TokenType::Identifier) &&
                           m_Current + 1 < m_Tokens.size() &&
                           m_Tokens[m_Current + 1].type == TokenType::Equal) {
                    Token name = Advance();
                    Consume(TokenType::Equal, "Expected '='");
                    key = std::make_shared<StringExpr>(name.value);
                    value = ParseExpression();
                } else {
                    key = std::make_shared<NumberExpr>(arrayIndex++);
                    value = ParseExpression();
                }

                table->entries.push_back({key, value});
            } while (Match(TokenType::Comma) || Match(TokenType::Semicolon));
        }

        Consume(TokenType::RightBrace, "Expected '}' after table");
        return table;
    }

    ExprNodePtr ScriptParser::ParseFunctionExpression() {
        auto func = std::make_shared<FunctionExpr>();
        func->line = Previous().line;

        Consume(TokenType::LeftParen, "Expected '(' after 'function'");
        if (!Check(TokenType::RightParen)) {
            do {
                Token param = Consume(TokenType::Identifier, "Expected parameter name");
                func->params.push_back(param.value);
            } while (Match(TokenType::Comma));
        }
        Consume(TokenType::RightParen, "Expected ')' after parameters");

        func->body = ParseBlock();
        Consume(TokenType::End, "Expected 'end' after function body");

        return func;
    }

    Token ScriptParser::Peek() const {
        return m_Tokens[m_Current];
    }

    Token ScriptParser::Previous() const {
        return m_Tokens[m_Current - 1];
    }

    Token ScriptParser::Advance() {
        if (!IsAtEnd()) m_Current++;
        return Previous();
    }

    bool ScriptParser::Check(TokenType type) const {
        if (IsAtEnd()) return false;
        return Peek().type == type;
    }

    bool ScriptParser::Match(TokenType type) {
        if (Check(type)) {
            Advance();
            return true;
        }
        return false;
    }

    bool ScriptParser::Match(std::initializer_list<TokenType> types) {
        for (TokenType type : types) {
            if (Check(type)) {
                Advance();
                return true;
            }
        }
        return false;
    }

    Token ScriptParser::Consume(TokenType type, const std::string& message) {
        if (Check(type)) return Advance();
        Error(message);
        return Peek();
    }

    void ScriptParser::Error(const std::string& message) {
        if (m_Error.empty()) {
            Token token = Peek();
            m_Error = message + " at line " + std::to_string(token.line);
            m_ErrorLine = token.line;
        }
    }

    bool ScriptParser::IsAtEnd() const {
        return Peek().type == TokenType::EndOfFile;
    }

    void ScriptParser::Synchronize() {
        Advance();

        while (!IsAtEnd()) {
            switch (Peek().type) {
                case TokenType::Function:
                case TokenType::Local:
                case TokenType::If:
                case TokenType::While:
                case TokenType::For:
                case TokenType::Return:
                    return;
                default:
                    Advance();
            }
        }
    }

}
