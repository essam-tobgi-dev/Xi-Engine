#pragma once

#include "ScriptLexer.h"
#include "ScriptAST.h"
#include <vector>

namespace Xi {

    class ScriptParser {
    public:
        ScriptParser(const std::vector<Token>& tokens);

        std::vector<StmtNodePtr> Parse();

        bool HasError() const { return !m_Error.empty(); }
        const std::string& GetError() const { return m_Error; }
        int GetErrorLine() const { return m_ErrorLine; }

    private:
        // Statement parsing
        StmtNodePtr ParseStatement();
        StmtNodePtr ParseLocalStatement();
        StmtNodePtr ParseFunctionStatement(bool isLocal);
        StmtNodePtr ParseIfStatement();
        StmtNodePtr ParseWhileStatement();
        StmtNodePtr ParseRepeatStatement();
        StmtNodePtr ParseForStatement();
        StmtNodePtr ParseReturnStatement();
        StmtNodePtr ParseExpressionStatement();
        std::vector<StmtNodePtr> ParseBlock();

        // Expression parsing (precedence climbing)
        ExprNodePtr ParseExpression();
        ExprNodePtr ParseOr();
        ExprNodePtr ParseAnd();
        ExprNodePtr ParseComparison();
        ExprNodePtr ParseConcat();
        ExprNodePtr ParseAddSub();
        ExprNodePtr ParseMulDiv();
        ExprNodePtr ParseUnary();
        ExprNodePtr ParsePower();
        ExprNodePtr ParsePostfix();
        ExprNodePtr ParsePrimary();
        ExprNodePtr ParseTableConstructor();
        ExprNodePtr ParseFunctionExpression();

        // Helpers
        Token Peek() const;
        Token Previous() const;
        Token Advance();
        bool Check(TokenType type) const;
        bool Match(TokenType type);
        bool Match(std::initializer_list<TokenType> types);
        Token Consume(TokenType type, const std::string& message);
        void Error(const std::string& message);
        bool IsAtEnd() const;
        void Synchronize();

        std::vector<Token> m_Tokens;
        size_t m_Current = 0;
        std::string m_Error;
        int m_ErrorLine = -1;
    };

}
