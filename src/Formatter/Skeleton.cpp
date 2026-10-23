#include "Skeleton.h"

#include <algorithm>
#include <cstddef>

namespace ObSL {
namespace {

bool is_bracket_open(TokenType t) {
  return t == TokenType::LEFT_PAREN || t == TokenType::LEFT_BRACKET ||
         t == TokenType::LEFT_BRACE;
}

bool is_bracket_close(TokenType t) {
  return t == TokenType::RIGHT_PAREN || t == TokenType::RIGHT_BRACKET ||
         t == TokenType::RIGHT_BRACE;
}

bool brackets_match(TokenType open, TokenType close) {
  switch (open) {
  case TokenType::LEFT_PAREN:
    return close == TokenType::RIGHT_PAREN;
  case TokenType::LEFT_BRACKET:
    return close == TokenType::RIGHT_BRACKET;
  case TokenType::LEFT_BRACE:
    return close == TokenType::RIGHT_BRACE;
  default:
    return false;
  }
}

bool is_statement_boundary(TokenType t) {
  switch (t) {
  case TokenType::SEMICOLON:
  case TokenType::RIGHT_BRACE:
  case TokenType::COLON:
  case TokenType::LEFT_BRACE:
    return true;
  default:
    return false;
  }
}

} // namespace

std::vector<FmtToken> annotate(const std::vector<Token> &tokens) {
  std::vector<FmtToken> out;
  out.reserve(tokens.size());
  for (const Token &t : tokens) {
    FmtToken f;
    f.token = t;
    if (t.type == TokenType::COMMENT)
      f.trivia = Trivia::LineComment;
    out.push_back(f);
  }

  const size_t n = out.size();

  // bracket pairing
  {
    std::vector<size_t> openers;
    size_t degraded_from = n;
    for (size_t i = 0; i < n; ++i) {
      const Token &t = out[i].token;
      if (t.type == TokenType::EOF_)
        break;
      if (is_bracket_open(t.type)) {
        openers.push_back(i);
      } else if (is_bracket_close(t.type)) {
        if (openers.empty() ||
            !brackets_match(out[openers.back()].token.type, t.type)) {
          degraded_from = std::min(degraded_from, i);
          openers.clear();
        } else {
          out[openers.back()].match = static_cast<int>(i);
          out[i].match = static_cast<int>(openers.back());
          openers.pop_back();
        }
      } else if (t.type == TokenType::UNKNOWN_ && degraded_from == n) {
        degraded_from = i;
      }
    }
    for (const size_t i : openers)
      degraded_from = std::min(degraded_from, i);
    for (size_t i = degraded_from; i < n; ++i)
      out[i].degraded = true;
  }

  // opener classification
  {
    const FmtToken *prev = nullptr;
    for (size_t i = 0; i < n; ++i) {
      FmtToken &cur = out[i];
      const TokenType t = cur.token.type;
      if (t == TokenType::COMMENT || t == TokenType::NEWLINE ||
          t == TokenType::EOF_)
        continue;
      if (is_bracket_open(t)) {
        switch (t) {
        case TokenType::LEFT_BRACE:
          cur.brace_kind =
              (prev != nullptr && prev->token.type == TokenType::IDENTIFIER)
                  ? BraceKind::StructBody
                  : BraceKind::BlockBody;
          break;
        case TokenType::LEFT_BRACKET:
          cur.brace_kind = (prev != nullptr &&
                            (prev->token.type == TokenType::IDENTIFIER ||
                             prev->token.type == TokenType::RIGHT_PAREN ||
                             prev->token.type == TokenType::RIGHT_BRACKET))
                               ? BraceKind::Index
                               : BraceKind::ArrayLit;
          break;
        case TokenType::LEFT_PAREN:
          cur.brace_kind = BraceKind::Parens;
          break;
        default:
          break;
        }
      }
      prev = &cur;
    }
  }

  // statement starts
  {
    int nesting = 0;
    const FmtToken *prev_code = nullptr;
    for (size_t i = 0; i < n; ++i) {
      FmtToken &cur = out[i];
      const TokenType t = cur.token.type;
      if (t == TokenType::COMMENT || t == TokenType::NEWLINE ||
          t == TokenType::EOF_)
        continue;
      const bool boundary =
          prev_code == nullptr ||
          (is_statement_boundary(prev_code->token.type) &&
           !(prev_code->token.type == TokenType::SEMICOLON && nesting > 0));
      if (boundary)
        cur.stmt_start = true;
      if (t == TokenType::LEFT_PAREN || t == TokenType::LEFT_BRACKET)
        ++nesting;
      else if (t == TokenType::RIGHT_PAREN || t == TokenType::RIGHT_BRACKET) {
        if (nesting > 0)
          --nesting;
      }
      prev_code = &cur;
    }
  }

  return out;
}

} // namespace ObSL
