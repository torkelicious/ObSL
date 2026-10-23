#pragma once

#include <ObSL/Tokens.h>
#include <cstdint>
#include <string_view>
#include <vector>

namespace ObSL {

// LexMode::Runtime          comments & newlines are skipped
// LexMode::Formatting       comments & newlines are emitted as COMMENT / NEWLINE

enum class LexMode : uint8_t {
  Runtime,
  Formatting,
};

class Lexer {
public:
  explicit Lexer(std::string_view source, LexMode mode = LexMode::Runtime);

  std::vector<Token> tokenize();

private:
  std::string_view source;
  LexMode mode;
  size_t current = 0;
  int line = 1;
  int column = 1;

  [[nodiscard]] char peek() const;

  [[nodiscard]] char peek_next() const;

  char advance();

  [[nodiscard]] bool is_at_end() const;

  void skip_whitespace(std::vector<Token> &trivia);

  Token read_string();

  Token read_num();

  Token read_identifier_or_keyword();

  Token read_operator_or_symbol();
};
} // namespace ObSL
