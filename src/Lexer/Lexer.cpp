#include <ObSL/Lexer.h>
#include <cctype>
#include <unordered_map>
#include <format>
#include <ObSL/Natives.h>

namespace ObSL {
    Lexer::Lexer(const std::string_view source) : source(source) {
    }

    std::vector<Token> Lexer::tokenize() {
        std::vector<Token> tokens;

        if (source.length() >= 3 && static_cast<unsigned char>(source[0]) == 0xEF &&
            static_cast<unsigned char>(source[1]) == 0xBB && static_cast<unsigned char>(source[2]) == 0xBF) {
            current += 3;
        }

        while (!is_at_end()) {
            skip_whitespace();
            if (is_at_end())
                break;
            if (const char c = peek(); std::isdigit(static_cast<unsigned char>(c))) {
                tokens.push_back(read_num());
            } else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                tokens.push_back(read_identifier_or_keyword());
            } else if (c == '"' || c == '\'') {
                tokens.push_back(read_string());
            } else {
                tokens.push_back(read_operator_or_symbol());
            }
        }
        tokens.push_back(Token{
            .type = TokenType::EOF_, .lexeme = "", .line = static_cast<uint16_t>(line), .column = static_cast<uint16_t>(column),
            .start_pos = static_cast<uint32_t>(current), .end_pos = static_cast<uint32_t>(current)
        });
        return tokens;
    }


    char Lexer::peek() const { return is_at_end() ? '\0' : source[current]; }

    char Lexer::peek_next() const { return current + 1 >= source.size() ? '\0' : source[current + 1]; }

    char Lexer::advance() {
        const char chr = peek();
        ++current;
        ++column;
        return chr;
    }

    bool Lexer::is_at_end() const { return current >= source.size(); }

    // skips comments too
    void Lexer::skip_whitespace() {
        while (!is_at_end()) {
            if (const char c = peek(); c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f') {
                advance();
            } else if (c == '\n') {
                advance();
                ++line;
                column = 1;
            } else if (c == '/' && peek_next() == '/') {
                advance();
                advance();
                while (peek() != '\n' && !is_at_end()) {
                    advance();
                }
            } else if (static_cast<unsigned char>(c) == 0xC2 && static_cast<unsigned char>(peek_next()) == 0xA0) {
                advance();
                advance();
            } else {
                break;
            }
        }
    }


    Token Lexer::read_num() {
        const size_t number_start = current;
        const size_t start_col = column;
        while (std::isdigit(static_cast<unsigned char>(peek()))) {
            advance();
        }
        // look for a fractional part.
        if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peek_next()))) {
            advance(); // Consume the "."
            while (std::isdigit(static_cast<unsigned char>(peek()))) {
                advance();
            }
        }
        const auto lexeme = source.substr(number_start, current - number_start);
        return Token{
            .type = TokenType::NUMBER,
            .lexeme = lexeme,
            .line = static_cast<uint16_t>(line),
            .column = static_cast<uint16_t>(start_col),
            .start_pos = static_cast<uint32_t>(number_start),
            .end_pos = static_cast<uint32_t>(current)
        };
    }

    Token Lexer::read_identifier_or_keyword() {
        const size_t id_start = current;
        const size_t start_col = column;

        while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') {
            advance();
        }

        const auto text = source.substr(id_start, current - id_start);

        static const std::unordered_map<std::string_view, TokenType> keywords = {
            {"and", TokenType::AND}, {"break", TokenType::BREAK}, {"case", TokenType::CASE},
            {"catch", TokenType::CATCH}, {"default", TokenType::DEFAULT}, {"switch", TokenType::SWITCH},
            {"struct", TokenType::STRUCT}, {"else", TokenType::ELSE}, {"false", TokenType::FALSE_},
            {"fn", TokenType::FN}, {"for", TokenType::FOR}, {"foreach", TokenType::FOREACH},
            {"if", TokenType::IF}, {"in", TokenType::IN_}, {"is", TokenType::IS},
            {"null", TokenType::NULL_}, {"or", TokenType::OR}, {"print", TokenType::PRINT},
            {"println", TokenType::PRINTLN}, {"return", TokenType::RETURN}, {"true", TokenType::TRUE_},
            {"try", TokenType::TRY}, {"using", TokenType::USING}, {"var", TokenType::VAR},
            {"while", TokenType::WHILE},
        };

        const auto it = keywords.find(text);
        const TokenType type = it != keywords.end() ? it->second : TokenType::IDENTIFIER;

        return Token{
            .type = type,
            .lexeme = text,
            .line = static_cast<uint16_t>(line),
            .column = static_cast<uint16_t>(start_col),
            .start_pos = static_cast<uint32_t>(id_start),
            .end_pos = static_cast<uint32_t>(current)
        };
    }

    Token Lexer::read_string() {
        const char quote_type = peek();
        const auto start_pos = static_cast<uint32_t>(current);
        const auto start_col = static_cast<uint16_t>(column);
        advance();
        while (!is_at_end() && peek() != quote_type) {
            if (peek() == '\n') {
                advance();
                line++;
                column = 1;
                continue;
            }
            if (peek() == '\\') {
                advance();
                if (!is_at_end()) {
                    if (peek() == '\n') {
                        advance();
                        line++;
                        column = 1;
                    } else {
                        advance();
                    }
                    continue;
                }
            }
            advance();
        }
        if (is_at_end()) {
            throw RuntimeError(Token{
                                   .type = TokenType::UNKNOWN_, .lexeme = "", .line = static_cast<uint16_t>(line),
                                   .column = static_cast<uint16_t>(start_col), .start_pos = static_cast<uint32_t>(current)
                               },
                               "Unterminated string.");
        }
        advance();
        return Token{
            .type = TokenType::STRING, .lexeme = std::string_view(&source[start_pos], current - start_pos),
            .line = static_cast<uint16_t>(line), .column = static_cast<uint16_t>(start_col), .start_pos = static_cast<uint32_t>(current)
        };
    }

    Token Lexer::read_operator_or_symbol() {
        const uint16_t start_col = column;
        const uint32_t start_pos = current;
        switch (advance()) {
            case '+':
                if (peek() == '+') {
                    advance();
                    return Token{
                        .type = TokenType::PLUS_PLUS,
                        .lexeme = "++",
                        .line = static_cast<uint16_t>(line),
                        .column = static_cast<uint16_t>(start_col),
                        .start_pos = static_cast<uint32_t>(start_pos),
                        .end_pos = static_cast<uint32_t>(current)
                    };
                }
                if (peek() == '=') {
                    advance();
                    return Token{
                        .type = TokenType::PLUS_EQUAL,
                        .lexeme = "+=",
                        .line = static_cast<uint16_t>(line),
                        .column = static_cast<uint16_t>(start_col),
                        .start_pos = static_cast<uint32_t>(start_pos),
                        .end_pos = static_cast<uint32_t>(current)
                    };
                }
                return Token{
                    .type = TokenType::PLUS,
                    .lexeme = "+",
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
            case '-':
                if (peek() == '-') {
                    advance();
                    return Token{
                        .type = TokenType::MINUS_MINUS, .lexeme = "--",
                        .line = static_cast<uint16_t>(line), .column = static_cast<uint16_t>(start_col),
                        .start_pos = static_cast<uint32_t>(start_pos), .end_pos = static_cast<uint32_t>(current)
                    };
                }
                if (peek() == '=') {
                    advance();
                    return Token{
                        .type = TokenType::MINUS_EQUAL, .lexeme = "-=",
                        .line = static_cast<uint16_t>(line), .column = static_cast<uint16_t>(start_col),
                        .start_pos = static_cast<uint32_t>(start_pos), .end_pos = static_cast<uint32_t>(current)
                    };
                }
                return Token{
                    .type = TokenType::MINUS,
                    .lexeme = "-",
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
            case '*':
                if (peek() == '=') {
                    advance();
                    return Token{
                        .type = TokenType::STAR_EQUAL,
                        .lexeme = "*=",
                        .line = static_cast<uint16_t>(line),
                        .column = static_cast<uint16_t>(start_col),
                        .start_pos = static_cast<uint32_t>(start_pos),
                        .end_pos = static_cast<uint32_t>(current)
                    };
                }
                return Token{
                    .type = TokenType::STAR,
                    .lexeme = "*",
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
            case '/':
                if (peek() == '=') {
                    advance();
                    return Token{
                        .type = TokenType::SLASH_EQUAL, .lexeme = "/=",
                        .line = static_cast<uint16_t>(line), .column = static_cast<uint16_t>(start_col),
                        .start_pos = static_cast<uint32_t>(start_pos), .end_pos = static_cast<uint32_t>(current)
                    };
                }
                return Token{
                    .type = TokenType::SLASH,
                    .lexeme = "/",
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
            case '%':
                if (peek() == '=') {
                    advance();
                    return Token{
                        .type = TokenType::PERCENT_EQUAL, .lexeme = "%=",
                        .line = static_cast<uint16_t>(line), .column = static_cast<uint16_t>(start_col),
                        .start_pos = static_cast<uint32_t>(start_pos), .end_pos = static_cast<uint32_t>(current)
                    };
                }
                return Token{
                    .type = TokenType::PERCENT,
                    .lexeme = "%",
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
            case '=':
                if (peek() == '=') {
                    advance();
                    return Token{
                        .type = TokenType::EQUAL_EQUAL, .lexeme = "==",
                        .line = static_cast<uint16_t>(line), .column = static_cast<uint16_t>(start_col),
                        .start_pos = static_cast<uint32_t>(start_pos), .end_pos = static_cast<uint32_t>(current)
                    };
                }
                return Token{
                    .type = TokenType::ASSIGN,
                    .lexeme = "=",
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
            case '!':
                if (peek() == '=') {
                    advance();
                    return Token{
                        .type = TokenType::BANG_EQUAL,
                        .lexeme = "!=",
                        .line = static_cast<uint16_t>(line),
                        .column = static_cast<uint16_t>(start_col),
                        .start_pos = static_cast<uint32_t>(start_pos),
                        .end_pos = static_cast<uint32_t>(current)
                    };
                }
                return Token{
                    .type = TokenType::BANG,
                    .lexeme = "!",
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
            case '<':
                if (peek() == '=') {
                    advance();
                    return Token{
                        .type = TokenType::LESS_EQUAL,
                        .lexeme = "<=",
                        .line = static_cast<uint16_t>(line),
                        .column = static_cast<uint16_t>(start_col),
                        .start_pos = static_cast<uint32_t>(start_pos),
                        .end_pos = static_cast<uint32_t>(current)
                    };
                }
                if (peek() == '<') {
                    advance();
                    return Token{
                        .type = TokenType::LESS_LESS,
                        .lexeme = "<<",
                        .line = static_cast<uint16_t>(line),
                        .column = static_cast<uint16_t>(start_col),
                        .start_pos = static_cast<uint32_t>(start_pos),
                        .end_pos = static_cast<uint32_t>(current)
                    };
                }
                return Token{
                    .type = TokenType::LESS,
                    .lexeme = "<",
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
            case '>':
                if (peek() == '=') {
                    advance();
                    return Token{
                        .type = TokenType::GREATER_EQUAL, .lexeme = ">=",
                        .line = static_cast<uint16_t>(line), .column = static_cast<uint16_t>(start_col),
                        .start_pos = static_cast<uint32_t>(start_pos), .end_pos = static_cast<uint32_t>(current)
                    };
                }
                if (peek() == '>') {
                    advance();
                    return Token{
                        .type = TokenType::GREATER_GREATER, .lexeme = ">>",
                        .line = static_cast<uint16_t>(line), .column = static_cast<uint16_t>(start_col),
                        .start_pos = static_cast<uint32_t>(start_pos), .end_pos = static_cast<uint32_t>(current)
                    };
                }
                return Token{
                    .type = TokenType::GREATER,
                    .lexeme = ">",
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
            case '&':
                if (peek() == '&') {
                    advance();
                    return Token{
                        .type = TokenType::AND,
                        .lexeme = "&&",
                        .line = static_cast<uint16_t>(line),
                        .column = static_cast<uint16_t>(start_col),
                        .start_pos = static_cast<uint32_t>(start_pos),
                        .end_pos = static_cast<uint32_t>(current)
                    };
                }
                return Token{
                    .type = TokenType::AMPERSAND,
                    .lexeme = "&",
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
            case '|':
                if (peek() == '|') {
                    advance();
                    return Token{
                        .type = TokenType::OR,
                        .lexeme = "||",
                        .line = static_cast<uint16_t>(line),
                        .column = static_cast<uint16_t>(start_col),
                        .start_pos = static_cast<uint32_t>(start_pos),
                        .end_pos = static_cast<uint32_t>(current)
                    };
                }
                return Token{
                    .type = TokenType::PIPE,
                    .lexeme = "|",
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
            case '^':
                return Token{
                    .type = TokenType::CARET,
                    .lexeme = "^",
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
            case '~':
                return Token{
                    .type = TokenType::TILDE,
                    .lexeme = "~",
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
            case '(':
                return Token{
                    .type = TokenType::LEFT_PAREN,
                    .lexeme = "(",
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
            case ')':
                return Token{
                    .type = TokenType::RIGHT_PAREN, .lexeme = ")",
                    .line = static_cast<uint16_t>(line), .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos), .end_pos = static_cast<uint32_t>(current)
                };
            case '{':
                return Token{
                    .type = TokenType::LEFT_BRACE,
                    .lexeme = "{",
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
            case '}':
                return Token{
                    .type = TokenType::RIGHT_BRACE, .lexeme = "}",
                    .line = static_cast<uint16_t>(line), .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos), .end_pos = static_cast<uint32_t>(current)
                };
            case '[':
                return Token{
                    .type = TokenType::LEFT_BRACKET, .lexeme = "[",
                    .line = static_cast<uint16_t>(line), .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos), .end_pos = static_cast<uint32_t>(current)
                };
            case ']':
                return Token{
                    .type = TokenType::RIGHT_BRACKET, .lexeme = "]",
                    .line = static_cast<uint16_t>(line), .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos), .end_pos = static_cast<uint32_t>(current)
                };
            case ';':
                return Token{
                    .type = TokenType::SEMICOLON,
                    .lexeme = ";",
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
            case ',':
                return Token{
                    .type = TokenType::COMMA,
                    .lexeme = ",",
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
            case '.':
                return Token{
                    .type = TokenType::DOT,
                    .lexeme = ".",
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
            case ':':
                return Token{
                    .type = TokenType::COLON,
                    .lexeme = ":",
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
            default:
                return Token{
                    .type = TokenType::UNKNOWN_,
                    .lexeme = std::string_view(&source[start_pos], 1),
                    .line = static_cast<uint16_t>(line),
                    .column = static_cast<uint16_t>(start_col),
                    .start_pos = static_cast<uint32_t>(start_pos),
                    .end_pos = static_cast<uint32_t>(current)
                };
        }
    }
} // namespace ObSL
