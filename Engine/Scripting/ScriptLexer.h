#pragma once

#include <string>
#include <vector>

namespace Xi {

    enum class TokenType {
        // Literals
        Number,
        String,
        Identifier,
        True,
        False,
        Nil,

        // Keywords
        And,
        Break,
        Do,
        Else,
        ElseIf,
        End,
        For,
        Function,
        If,
        In,
        Local,
        Not,
        Or,
        Repeat,
        Return,
        Then,
        Until,
        While,

        // Operators
        Plus,           // +
        Minus,          // -
        Star,           // *
        Slash,          // /
        Percent,        // %
        Caret,          // ^
        Hash,           // #
        Equal,          // =
        EqualEqual,     // ==
        NotEqual,       // ~=
        Less,           // <
        LessEqual,      // <=
        Greater,        // >
        GreaterEqual,   // >=
        Concat,         // ..

        // Delimiters
        LeftParen,      // (
        RightParen,     // )
        LeftBrace,      // {
        RightBrace,     // }
        LeftBracket,    // [
        RightBracket,   // ]
        Semicolon,      // ;
        Colon,          // :
        Comma,          // ,
        Dot,            // .

        // Special
        EndOfFile,
        Error
    };

    struct Token {
        TokenType type;
        std::string value;
        int line;
        int column;
    };

    class ScriptLexer {
    public:
        ScriptLexer(const std::string& source);

        std::vector<Token> Tokenize();
        const std::string& GetError() const { return m_Error; }
        int GetErrorLine() const { return m_ErrorLine; }

    private:
        Token MakeToken(TokenType type);
        Token MakeToken(TokenType type, const std::string& value);
        Token MakeError(const std::string& message);

        char Peek() const;
        char PeekNext() const;
        char Advance();
        bool Match(char expected);
        bool IsAtEnd() const;

        void SkipWhitespace();
        void SkipComment();

        Token ScanNumber();
        Token ScanString(char quote);
        Token ScanIdentifier();

        TokenType CheckKeyword(const std::string& text);

        std::string m_Source;
        size_t m_Start = 0;
        size_t m_Current = 0;
        int m_Line = 1;
        int m_Column = 1;
        int m_TokenColumn = 1;

        std::string m_Error;
        int m_ErrorLine = -1;
    };

}
