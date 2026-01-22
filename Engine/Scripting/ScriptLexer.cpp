#include "ScriptLexer.h"
#include <unordered_map>
#include <cctype>

namespace Xi {

    static std::unordered_map<std::string, TokenType> s_Keywords = {
        {"and", TokenType::And},
        {"break", TokenType::Break},
        {"do", TokenType::Do},
        {"else", TokenType::Else},
        {"elseif", TokenType::ElseIf},
        {"end", TokenType::End},
        {"false", TokenType::False},
        {"for", TokenType::For},
        {"function", TokenType::Function},
        {"if", TokenType::If},
        {"in", TokenType::In},
        {"local", TokenType::Local},
        {"nil", TokenType::Nil},
        {"not", TokenType::Not},
        {"or", TokenType::Or},
        {"repeat", TokenType::Repeat},
        {"return", TokenType::Return},
        {"then", TokenType::Then},
        {"true", TokenType::True},
        {"until", TokenType::Until},
        {"while", TokenType::While}
    };

    ScriptLexer::ScriptLexer(const std::string& source)
        : m_Source(source) {}

    std::vector<Token> ScriptLexer::Tokenize() {
        std::vector<Token> tokens;

        while (!IsAtEnd()) {
            m_Start = m_Current;
            m_TokenColumn = m_Column;

            SkipWhitespace();
            if (IsAtEnd()) break;

            m_Start = m_Current;
            m_TokenColumn = m_Column;

            char c = Advance();

            // Single-character tokens
            switch (c) {
                case '(': tokens.push_back(MakeToken(TokenType::LeftParen)); continue;
                case ')': tokens.push_back(MakeToken(TokenType::RightParen)); continue;
                case '{': tokens.push_back(MakeToken(TokenType::LeftBrace)); continue;
                case '}': tokens.push_back(MakeToken(TokenType::RightBrace)); continue;
                case '[': tokens.push_back(MakeToken(TokenType::LeftBracket)); continue;
                case ']': tokens.push_back(MakeToken(TokenType::RightBracket)); continue;
                case ';': tokens.push_back(MakeToken(TokenType::Semicolon)); continue;
                case ':': tokens.push_back(MakeToken(TokenType::Colon)); continue;
                case ',': tokens.push_back(MakeToken(TokenType::Comma)); continue;
                case '+': tokens.push_back(MakeToken(TokenType::Plus)); continue;
                case '*': tokens.push_back(MakeToken(TokenType::Star)); continue;
                case '/': tokens.push_back(MakeToken(TokenType::Slash)); continue;
                case '%': tokens.push_back(MakeToken(TokenType::Percent)); continue;
                case '^': tokens.push_back(MakeToken(TokenType::Caret)); continue;
                case '#': tokens.push_back(MakeToken(TokenType::Hash)); continue;
            }

            // Two-character tokens
            if (c == '-') {
                if (Match('-')) {
                    SkipComment();
                    continue;
                }
                tokens.push_back(MakeToken(TokenType::Minus));
                continue;
            }

            if (c == '.') {
                if (Match('.')) {
                    tokens.push_back(MakeToken(TokenType::Concat));
                } else if (std::isdigit(Peek())) {
                    m_Current--;  // Back up to include the dot
                    m_Column--;
                    tokens.push_back(ScanNumber());
                } else {
                    tokens.push_back(MakeToken(TokenType::Dot));
                }
                continue;
            }

            if (c == '=') {
                if (Match('=')) {
                    tokens.push_back(MakeToken(TokenType::EqualEqual));
                } else {
                    tokens.push_back(MakeToken(TokenType::Equal));
                }
                continue;
            }

            if (c == '~') {
                if (Match('=')) {
                    tokens.push_back(MakeToken(TokenType::NotEqual));
                } else {
                    tokens.push_back(MakeError("Unexpected character '~'"));
                    return tokens;
                }
                continue;
            }

            if (c == '<') {
                if (Match('=')) {
                    tokens.push_back(MakeToken(TokenType::LessEqual));
                } else {
                    tokens.push_back(MakeToken(TokenType::Less));
                }
                continue;
            }

            if (c == '>') {
                if (Match('=')) {
                    tokens.push_back(MakeToken(TokenType::GreaterEqual));
                } else {
                    tokens.push_back(MakeToken(TokenType::Greater));
                }
                continue;
            }

            // Strings
            if (c == '"' || c == '\'') {
                tokens.push_back(ScanString(c));
                if (!m_Error.empty()) return tokens;
                continue;
            }

            // Numbers
            if (std::isdigit(c)) {
                m_Current--;  // Back up to include the first digit
                m_Column--;
                tokens.push_back(ScanNumber());
                continue;
            }

            // Identifiers and keywords
            if (std::isalpha(c) || c == '_') {
                m_Current--;  // Back up
                m_Column--;
                tokens.push_back(ScanIdentifier());
                continue;
            }

            tokens.push_back(MakeError("Unexpected character"));
            return tokens;
        }

        tokens.push_back(MakeToken(TokenType::EndOfFile));
        return tokens;
    }

    Token ScriptLexer::MakeToken(TokenType type) {
        return Token{type, m_Source.substr(m_Start, m_Current - m_Start), m_Line, m_TokenColumn};
    }

    Token ScriptLexer::MakeToken(TokenType type, const std::string& value) {
        return Token{type, value, m_Line, m_TokenColumn};
    }

    Token ScriptLexer::MakeError(const std::string& message) {
        m_Error = message + " at line " + std::to_string(m_Line);
        m_ErrorLine = m_Line;
        return Token{TokenType::Error, message, m_Line, m_TokenColumn};
    }

    char ScriptLexer::Peek() const {
        if (IsAtEnd()) return '\0';
        return m_Source[m_Current];
    }

    char ScriptLexer::PeekNext() const {
        if (m_Current + 1 >= m_Source.size()) return '\0';
        return m_Source[m_Current + 1];
    }

    char ScriptLexer::Advance() {
        char c = m_Source[m_Current++];
        if (c == '\n') {
            m_Line++;
            m_Column = 1;
        } else {
            m_Column++;
        }
        return c;
    }

    bool ScriptLexer::Match(char expected) {
        if (IsAtEnd()) return false;
        if (m_Source[m_Current] != expected) return false;
        Advance();
        return true;
    }

    bool ScriptLexer::IsAtEnd() const {
        return m_Current >= m_Source.size();
    }

    void ScriptLexer::SkipWhitespace() {
        while (!IsAtEnd()) {
            char c = Peek();
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                Advance();
            } else {
                break;
            }
        }
    }

    void ScriptLexer::SkipComment() {
        // Skip to end of line for single-line comments
        // For multi-line comments: --[[ ... ]]
        if (Peek() == '[' && PeekNext() == '[') {
            Advance(); // [
            Advance(); // [
            while (!IsAtEnd()) {
                if (Peek() == ']' && PeekNext() == ']') {
                    Advance(); // ]
                    Advance(); // ]
                    return;
                }
                Advance();
            }
        } else {
            // Single line comment
            while (!IsAtEnd() && Peek() != '\n') {
                Advance();
            }
        }
    }

    Token ScriptLexer::ScanNumber() {
        m_Start = m_Current;
        m_TokenColumn = m_Column;

        while (std::isdigit(Peek())) Advance();

        // Look for decimal part
        if (Peek() == '.' && std::isdigit(PeekNext())) {
            Advance(); // consume '.'
            while (std::isdigit(Peek())) Advance();
        }

        // Look for exponent
        if (Peek() == 'e' || Peek() == 'E') {
            Advance();
            if (Peek() == '+' || Peek() == '-') Advance();
            while (std::isdigit(Peek())) Advance();
        }

        return MakeToken(TokenType::Number);
    }

    Token ScriptLexer::ScanString(char quote) {
        std::string value;

        while (!IsAtEnd() && Peek() != quote) {
            if (Peek() == '\n') {
                return MakeError("Unterminated string");
            }
            if (Peek() == '\\') {
                Advance();
                if (IsAtEnd()) break;
                char escaped = Advance();
                switch (escaped) {
                    case 'n': value += '\n'; break;
                    case 't': value += '\t'; break;
                    case 'r': value += '\r'; break;
                    case '\\': value += '\\'; break;
                    case '"': value += '"'; break;
                    case '\'': value += '\''; break;
                    default: value += escaped; break;
                }
            } else {
                value += Advance();
            }
        }

        if (IsAtEnd()) {
            return MakeError("Unterminated string");
        }

        Advance(); // closing quote
        return MakeToken(TokenType::String, value);
    }

    Token ScriptLexer::ScanIdentifier() {
        m_Start = m_Current;
        m_TokenColumn = m_Column;

        while (std::isalnum(Peek()) || Peek() == '_') {
            Advance();
        }

        std::string text = m_Source.substr(m_Start, m_Current - m_Start);
        TokenType type = CheckKeyword(text);
        return MakeToken(type, text);
    }

    TokenType ScriptLexer::CheckKeyword(const std::string& text) {
        auto it = s_Keywords.find(text);
        if (it != s_Keywords.end()) {
            return it->second;
        }
        return TokenType::Identifier;
    }

}
