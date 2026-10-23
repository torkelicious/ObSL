#include "Formatter.h"

#include <ObSL/Lexer.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include "Skeleton.h"

namespace ObSL {
namespace {

constexpr int kIndentWidth = 4;
constexpr int kMaxBlankLines = 1;
constexpr int kMaxLineLength = 100;
constexpr int kContIndent = 4;

bool value_like(TokenType t) {
  switch (t) {
  case TokenType::NUMBER:
  case TokenType::STRING:
  case TokenType::IDENTIFIER:
  case TokenType::RIGHT_PAREN:
  case TokenType::RIGHT_BRACKET:
  case TokenType::PLUS_PLUS:
  case TokenType::MINUS_MINUS:
    return true;
  default:
    return false;
  }
}

// when not a space belongs between prev and cur
bool glues(const FmtToken &prev, const FmtToken &cur,
           const bool minus_unary = false) {
  const TokenType p = prev.token.type;
  const TokenType c = cur.token.type;

  switch (c) {
  case TokenType::COMMA:
  case TokenType::SEMICOLON:
  case TokenType::COLON:
  case TokenType::DOT:
  case TokenType::RIGHT_PAREN:
  case TokenType::RIGHT_BRACKET:
  case TokenType::PLUS_PLUS:
  case TokenType::MINUS_MINUS:
    return true;
  case TokenType::PLUS:
  case TokenType::MINUS:
    return p == TokenType::LEFT_PAREN || p == TokenType::LEFT_BRACKET;
  case TokenType::LEFT_PAREN:
    return p == TokenType::IDENTIFIER || p == TokenType::RIGHT_PAREN ||
           p == TokenType::RIGHT_BRACKET;
  case TokenType::LEFT_BRACKET:
    return p == TokenType::IDENTIFIER || p == TokenType::RIGHT_PAREN ||
           p == TokenType::RIGHT_BRACKET;
  default:
    break;
  }

  switch (p) {
  case TokenType::LEFT_PAREN:
  case TokenType::LEFT_BRACKET:
  case TokenType::DOT:
  case TokenType::BANG:
  case TokenType::TILDE:
    return true;
  case TokenType::PLUS:
  case TokenType::MINUS:
    return minus_unary;
  default:
    return false;
  }
}

std::string indent_of(int units) {
  units = std::max(units, 0);
  return std::string(static_cast<size_t>(units) * kIndentWidth, ' ');
}

// binary operator precedence
int binop_prec(TokenType t) {
  switch (t) {
  case TokenType::OR:
    return 1;
  case TokenType::AND:
    return 2;
  case TokenType::EQUAL_EQUAL:
  case TokenType::BANG_EQUAL:
  case TokenType::GREATER:
  case TokenType::GREATER_EQUAL:
  case TokenType::LESS:
  case TokenType::LESS_EQUAL:
  case TokenType::IS:
    return 3;
  case TokenType::PIPE:
    return 4;
  case TokenType::CARET:
    return 5;
  case TokenType::AMPERSAND:
    return 6;
  case TokenType::LESS_LESS:
  case TokenType::GREATER_GREATER:
    return 7;
  case TokenType::PLUS:
  case TokenType::MINUS:
    return 8;
  case TokenType::STAR:
  case TokenType::SLASH:
  case TokenType::PERCENT:
    return 9;
  default:
    return 0;
  }
}

struct InlineResult {
  bool ok = false;
  std::string text;
};

InlineResult inline_render(const std::vector<FmtToken> &skel,
                           const size_t first, const size_t last,
                           const int start_col, const bool multi = false) {
  InlineResult r;
  std::string text;
  const FmtToken *prev_code = nullptr;
  bool prev_un = false;
  int stmt_count = 1;

  for (size_t j = first; j <= last; ++j) {
    const FmtToken &tok = skel[j];
    const TokenType tt = tok.token.type;

    if (tt == TokenType::NEWLINE)
      continue;

    if (tt == TokenType::COMMENT || tt == TokenType::UNKNOWN_ ||
        tt == TokenType::CASE || tt == TokenType::DEFAULT ||
        tt == TokenType::RIGHT_BRACE || tt == TokenType::EOF_)
      return r; // refuse to inline

    if (prev_code != nullptr && tok.stmt_start) {
      if (prev_code->token.type != TokenType::SEMICOLON &&
          prev_code->token.type != TokenType::RIGHT_BRACE)
        return r; // joining here would change meaning
      if (!multi && ++stmt_count > 1)
        return r; // one statement per block
    }

    std::string sep;
    if (prev_code != nullptr)
      sep = glues(*prev_code, tok, prev_un) ? "" : " ";

    if (tt == TokenType::LEFT_BRACE) {
      if (tok.degraded || tok.match < 0)
        return r;
      const size_t close_i = static_cast<size_t>(tok.match);
      if (close_i <= j || close_i > last)
        return r;

      const int open_col =
          start_col + static_cast<int>(text.size() + sep.size());
      bool nest_empty = true;
      for (size_t k = j + 1; k < close_i; ++k)
        if (skel[k].token.type != TokenType::NEWLINE) {
          nest_empty = false;
          break;
        }
      if (nest_empty) {
        text += sep;
        text += "{}";
      } else {
        // struct bodies may put several trivial members on one line
        const InlineResult nest =
            inline_render(skel, j + 1, close_i - 1, open_col + 2,
                          tok.brace_kind == BraceKind::StructBody);
        if (!nest.ok ||
            open_col + static_cast<int>(nest.text.size()) + 4 > kMaxLineLength)
          return r;
        text += sep;
        text += "{ ";
        text += nest.text;
        text += " }";
      }
      prev_code = &skel[close_i];
      prev_un = false;
      j = close_i;
      continue;
    }

    text += sep;
    text += tok.token.lexeme;
    prev_un = (tt == TokenType::MINUS || tt == TokenType::PLUS) &&
              !value_like(prev_code != nullptr ? prev_code->token.type
                                               : TokenType::EOF_);
    prev_code = &tok;
  }

  r.ok = true;
  r.text = std::move(text);
  return r;
}

struct WrapResult {
  bool ok = false;
  std::string text;
  int last_width = 0;
};

WrapResult wrap_flat(const std::vector<FmtToken> &skel, const size_t begin,
                     const size_t end, const int base_col) {
  const InlineResult flat = inline_render(skel, begin, end - 1, base_col);
  if (!flat.ok ||
      base_col + static_cast<int>(flat.text.size()) > kMaxLineLength)
    return {};
  WrapResult w;
  w.ok = true;
  w.text = flat.text;
  w.last_width = base_col + static_cast<int>(flat.text.size());
  return w;
}

bool is_oplike(TokenType t) {
  if (binop_prec(t) > 0)
    return true;
  switch (t) {
  case TokenType::ASSIGN:
  case TokenType::PLUS_EQUAL:
  case TokenType::MINUS_EQUAL:
  case TokenType::STAR_EQUAL:
  case TokenType::SLASH_EQUAL:
  case TokenType::PERCENT_EQUAL:
    return true;
  default:
    return false;
  }
}

WrapResult try_bracket_list(const std::vector<FmtToken> &skel,
                            const size_t begin, const size_t end,
                            const int base_col, const bool want_array) {
  // find the first depth 0 bracket of the wanted kind with a valid closer
  int rel = 0;
  size_t open_i = end, close_i = end;
  char open_ch = 0, close_ch = 0;
  for (size_t k = begin; k < end; ++k) {
    const TokenType kt = skel[k].token.type;
    if (kt == TokenType::NEWLINE)
      continue;
    if (kt == TokenType::COMMENT || kt == TokenType::UNKNOWN_ ||
        kt == TokenType::LEFT_BRACE)
      return {};
    if (kt == TokenType::LEFT_PAREN || kt == TokenType::LEFT_BRACKET) {
      const bool is_array = kt == TokenType::LEFT_BRACKET;
      if (rel == 0 && is_array == want_array &&
          skel[k].brace_kind ==
              (is_array ? BraceKind::ArrayLit : BraceKind::Parens) &&
          skel[k].match >= 0) {
        open_i = k;
        close_i = static_cast<size_t>(skel[k].match);
        if (close_i >= end)
          return {};
        open_ch = is_array ? '[' : '(';
        close_ch = is_array ? ']' : ')';
        break;
      }
      ++rel;
    } else if (kt == TokenType::RIGHT_PAREN || kt == TokenType::RIGHT_BRACKET) {
      if (rel > 0)
        --rel;
    }
  }
  if (open_i >= end)
    return {};

  std::vector<std::pair<size_t, size_t>> segs;
  {
    int d = 0;
    size_t s = open_i + 1;
    for (size_t k = open_i + 1; k < close_i; ++k) {
      const TokenType kt = skel[k].token.type;
      if (kt == TokenType::NEWLINE)
        continue;
      if (kt == TokenType::LEFT_PAREN || kt == TokenType::LEFT_BRACKET) {
        ++d;
      } else if (kt == TokenType::RIGHT_PAREN ||
                 kt == TokenType::RIGHT_BRACKET) {
        if (d > 0)
          --d;
      } else if (kt == TokenType::COMMA && d == 0) {
        segs.emplace_back(s, k);
        s = k + 1;
      }
    }
    segs.emplace_back(s, close_i);
  }
  if (segs.size() < 2)
    return {}; // dont wrap single arg calls

  std::string text;
  int w = base_col;

  if (open_i > begin) {
    const InlineResult head = inline_render(skel, begin, open_i - 1, base_col);
    if (!head.ok)
      return {};
    const char sep = (open_i == begin)
                         ? 0
                         : (glues(skel[open_i - 1], skel[open_i]) ? 0 : ' ');
    if (sep == ' ')
      text += ' ';
    text += head.text;
    w = base_col + static_cast<int>(head.text.size()) + (sep == ' ' ? 1 : 0);
  }
  text += open_ch;
  ++w;
  if (w > kMaxLineLength)
    return {};

  for (size_t s = 0; s < segs.size(); ++s) {
    const auto &seg = segs[s];
    const InlineResult arg =
        inline_render(skel, seg.first, seg.second - 1, base_col + kContIndent);
    if (!arg.ok)
      return {};
    const int aw =
        base_col + kContIndent + static_cast<int>(arg.text.size()) + 1;
    if (aw > kMaxLineLength)
      return {};
    text += '\n';
    text += indent_of(base_col / kIndentWidth + 1);
    text += arg.text;
    if (s + 1 < segs.size())
      text += ',';
    w = aw;
  }

  text += '\n';
  text += indent_of(base_col / kIndentWidth);
  text += close_ch;
  w = base_col + 1;
  if (w > kMaxLineLength)
    return {};

  if (close_i + 1 < end) {
    size_t tstart = close_i + 1;
    while (tstart < end && skel[tstart].token.type == TokenType::NEWLINE)
      ++tstart;
    if (tstart < end) {
      const bool tail_op = is_oplike(skel[tstart].token.type);
      const bool glues_tail = glues(skel[close_i], skel[tstart]);
      const int head_extra =
          (glues_tail ? 0 : 1) +
          (tail_op ? static_cast<int>(skel[tstart].token.lexeme.size()) + 1
                   : 0);
      if (!glues_tail)
        text += ' ';
      if (tail_op) {
        text += skel[tstart].token.lexeme;
        text += ' ';
      }
      const int tstart_col = base_col + 1 + head_extra;
      const InlineResult tail = inline_render(
          skel, tail_op ? tstart + 1 : tstart, end - 1, tstart_col);
      const int tl = tstart_col + static_cast<int>(tail.text.size());
      if (!tail.ok || tl > kMaxLineLength)
        return {};
      text += tail.text;
      w = tl;
    }
  }

  WrapResult wr;
  wr.ok = true;
  wr.text = text;
  wr.last_width = w;
  return wr;
}

WrapResult try_binops(const std::vector<FmtToken> &skel, const size_t begin,
                      const size_t end, const int base_col) {
  std::string text;
  std::string line;
  int width = base_col;
  const FmtToken *prev = nullptr;
  bool prev_un = false;
  size_t last_cand = end;
  size_t cand_line_len = 0;
  bool any_cand = false;
  bool fresh_line = false;

  size_t k = begin;
  while (k < end) {
    const FmtToken &tok = skel[k];
    const TokenType tt = tok.token.type;
    if (tt == TokenType::NEWLINE) {
      ++k;
      continue;
    }
    if (tt == TokenType::COMMENT || tt == TokenType::UNKNOWN_)
      return {};

    const std::string sep =
        (!fresh_line && prev != nullptr && !glues(*prev, tok, prev_un)) ? " "
                                                                        : "";
    const int addw = static_cast<int>(sep.size() + tok.token.lexeme.size());
    const bool is_cand =
        binop_prec(tt) > 0 &&
        (tt != TokenType::MINUS ||
         value_like(prev != nullptr ? prev->token.type : TokenType::COMMA));

    if (width + addw > kMaxLineLength && last_cand != end) {
      line.resize(cand_line_len);
      text += line;
      text += '\n';
      const int cont = base_col + kContIndent;
      line = indent_of(base_col / kIndentWidth + 1);
      width = cont;
      prev = &skel[last_cand];
      prev_un = false;
      fresh_line = true;
      k = last_cand + 1;
      last_cand = end;
      continue;
    }
    if (width + addw > kMaxLineLength)
      return {}; // give up

    line += sep;
    line += tok.token.lexeme;
    width += addw;
    fresh_line = false;
    if (is_cand) {
      last_cand = k;
      cand_line_len = line.size();
      any_cand = true;
    }
    prev_un = (tt == TokenType::MINUS || tt == TokenType::PLUS) &&
              prev != nullptr && !value_like(prev->token.type);
    prev = &tok;
    ++k;
  }

  if (!any_cand)
    return {};
  text += line;

  WrapResult w;
  w.ok = true;
  w.text = text;
  w.last_width = width;
  return w;
}

WrapResult wrap_statement(const std::vector<FmtToken> &skel, const size_t begin,
                          const size_t end, const int base_col) {
  if (end <= begin)
    return {};
  WrapResult w = wrap_flat(skel, begin, end, base_col); // fits
  if (w.ok)
    return w;
  w = try_bracket_list(skel, begin, end, base_col, false); // one arg per line
  if (w.ok)
    return w;
  w = try_bracket_list(skel, begin, end, base_col, true); // one slot per line
  if (w.ok)
    return w;
  return try_binops(skel, begin, end, base_col); // break at ops
}

} // namespace

std::string format_source(const std::string_view source) {
  Lexer lexer(source, LexMode::Formatting);
  const std::vector<Token> tokens = lexer.tokenize();
  const std::vector<FmtToken> skel = annotate(tokens);

  const size_t n = skel.size();
  std::vector<size_t> stmt_end(n, 0);
  std::vector<bool> stmt_wrappable(n, false);
  for (size_t idx = 0; idx < n; ++idx) {
    const FmtToken &f = skel[idx];
    const TokenType ft = f.token.type;
    if (ft == TokenType::NEWLINE || ft == TokenType::COMMENT ||
        ft == TokenType::EOF_)
      continue;
    if (!f.stmt_start || f.degraded)
      continue;
    if (ft == TokenType::LEFT_BRACE || ft == TokenType::RIGHT_BRACE ||
        ft == TokenType::ELSE || ft == TokenType::CATCH)
      continue;
    size_t k = idx;
    bool wrappable = true;
    while (k < n) {
      const TokenType kt = skel[k].token.type;
      if (kt == TokenType::EOF_)
        break;
      if (kt == TokenType::COMMENT || kt == TokenType::UNKNOWN_)
        break; // junk
      if (kt == TokenType::NEWLINE) {
        ++k;
        continue;
      }
      if (kt == TokenType::LEFT_BRACE) {
        wrappable = false; // block statements layout
        break;
      }
      if (k > idx && skel[k].stmt_start)
        break;
      ++k;
    }
    stmt_end[idx] = k;
    stmt_wrappable[idx] = wrappable;
  }

  std::string out;
  out.reserve(source.size() + source.size() / 8 + 16);

  int depth = 0;          // curly brac nesting (blocks and struct bodies)
  int paren = 0;          // ( / [ nesting
  int case_depth = 0;     // stmt lines following a case label
  int pending_nl = 0;     // NEWLINE not yet into the output
  bool head_open = false; // else/catch head is being consumed
  int col = 0;            // width of the current out line
  bool line_open = false;
  const FmtToken *last_code = nullptr;
  bool minus_unary = false;

  auto to_line_start = [&](const bool force, const bool allow_blanks) {
    if (pending_nl > 0 || force) {
      if (line_open) {
        out += '\n';
        int blanks = 0;
        if (pending_nl > 0 && allow_blanks)
          blanks = std::min(std::max(pending_nl - 1, 0), kMaxBlankLines);
        pending_nl = 0;
        for (int b = 0; b < blanks; ++b)
          out += '\n';
      } else {
        pending_nl = 0;
      }
      line_open = false;
    }
    if (!line_open) {
      const std::string indent =
          indent_of(depth + case_depth + (paren > 0 ? 1 : 0));
      out += indent;
      col = static_cast<int>(indent.size());
    }
  };

  for (size_t i = 0; i < n; ++i) {
    const FmtToken &cur = skel[i];
    const TokenType t = cur.token.type;

    if (t == TokenType::NEWLINE) {
      ++pending_nl;
      continue;
    }

    if (t == TokenType::COMMENT) {
      head_open = false;
      if (pending_nl > 0) {
        to_line_start(false, true);
      } else if (line_open) {
        const int gap = cur.token.lexeme.starts_with("/*") ? 1 : 2;
        for (int g = 0; g < gap; ++g) out += ' ';
        col += gap;
      }
      out += cur.token.lexeme;
      col += static_cast<int>(cur.token.lexeme.size());
      line_open = true;
      continue;
    }

    if (t == TokenType::EOF_) {
      if (line_open) {
        out += '\n';
        col = 0;
      }
      pending_nl = 0;
      break;
    }

    // code token
    if (t == TokenType::CASE || t == TokenType::DEFAULT)
      case_depth = 0;

    // '} else {' / '} catch (e) {': onto the line of the closer
    const bool cuddle = (t == TokenType::ELSE || t == TokenType::CATCH) &&
                        line_open && last_code != nullptr &&
                        last_code->token.type == TokenType::RIGHT_BRACE &&
                        paren == 0;

    if (t == TokenType::RIGHT_BRACE) {
      pending_nl = 0; // no blank lines before closing brace
      if (line_open) {
        out += '\n';
        line_open = false;
        col = 0;
      }
      if (depth > 0)
        --depth;
      case_depth = 0;
      head_open = false;
      const std::string indent = indent_of(depth);
      out += indent;
      col = static_cast<int>(indent.size());
      out += cur.token.lexeme;
      ++col;
      line_open = true;
      last_code = &cur;
      minus_unary = false;
      continue;
    }

    if (cur.stmt_start && !cuddle) {
      if (head_open && t == TokenType::IF) {
      } else {
        head_open = false;
        to_line_start(true, paren == 0);

        // simple statements: lay out within the budget (flat or wrapped)
        if (stmt_wrappable[i] && !cur.degraded && stmt_end[i] > i + 1) {
          const size_t end = stmt_end[i];
          const WrapResult w = wrap_statement(skel, i, end, col);
          if (w.ok) {
            out += w.text;
            col = w.last_width;
            line_open = true;
            // last code token of the span for subsequent spacing state
            size_t last_i = end;
            while (last_i > i + 1 &&
                   skel[last_i - 1].token.type == TokenType::NEWLINE)
              --last_i;
            last_code = &skel[last_i - 1];
            minus_unary = false;
            i = last_i - 1;
            continue;
          }
        }
      }
    } else if (!cuddle && pending_nl > 0) {
      to_line_start(false, false);
    } else if (cuddle) {
      pending_nl = 0;   
      head_open = true; 
    }

    // spacing before the token
    if (line_open && last_code != nullptr) {
      bool glue = glues(*last_code, cur);
      if (last_code->token.type == TokenType::MINUS)
        glue = minus_unary;
      if (!glue) {
        out += ' ';
        ++col;
      }
    }

    // inline brace bodies
    if (t == TokenType::LEFT_BRACE && cur.match > static_cast<int>(i) &&
        !cur.degraded) {
      const size_t close_i = static_cast<size_t>(cur.match);

      bool blank_only = true;
      for (size_t j = i + 1; j < close_i; ++j)
        if (skel[j].token.type != TokenType::NEWLINE) {
          blank_only = false;
          break;
        }
      if (blank_only) {
        out += "{}";
        col += 2;
        line_open = true;
        last_code = &skel[close_i];
        minus_unary = false;
        head_open = false;
        i = close_i;
        continue;
      }

      size_t nx = close_i + 1;
      while (nx < n && (skel[nx].token.type == TokenType::NEWLINE ||
                        skel[nx].token.type == TokenType::COMMENT))
        ++nx;
      const bool chain_follows =
          nx < n && (skel[nx].token.type == TokenType::ELSE ||
                     skel[nx].token.type == TokenType::CATCH);

      const InlineResult body =
          inline_render(skel, i + 1, close_i - 1, col + 2,
                        cur.brace_kind == BraceKind::StructBody);
      if (body.ok && !head_open && !chain_follows &&
          col + static_cast<int>(body.text.size()) + 4 <= kMaxLineLength) {
        out += "{ " + body.text + " }";
        col += static_cast<int>(body.text.size()) + 4;
        line_open = true;
        last_code = &skel[close_i];
        minus_unary = false;
        i = close_i;
        continue;
      }
    }

    out += cur.token.lexeme;
    col += static_cast<int>(cur.token.lexeme.size());

    if (t == TokenType::SEMICOLON)
      head_open = false;
    if (t == TokenType::LEFT_BRACE) {
      ++depth;
      if (head_open) { 
        head_open = false;
      }
    } else if (t == TokenType::LEFT_PAREN || t == TokenType::LEFT_BRACKET) {
      ++paren;
    } else if (t == TokenType::RIGHT_PAREN || t == TokenType::RIGHT_BRACKET) {
      if (paren > 0)
        --paren;
    } else if (t == TokenType::COLON) {
      ++case_depth;
      size_t body_start = i + 1;
      bool label_comment = false;
      while (body_start < n &&
             (skel[body_start].token.type == TokenType::NEWLINE ||
              skel[body_start].token.type == TokenType::COMMENT)) {
        if (skel[body_start].token.type == TokenType::COMMENT)
          label_comment = true;
        ++body_start;
      }
      size_t body_end = body_start;
      if (body_start < n && skel[body_start].token.type != TokenType::COMMENT &&
          skel[body_start].token.type != TokenType::EOF_) {
        body_end = body_start + 1;
        while (body_end < n && skel[body_end].token.type != TokenType::EOF_ &&
               !skel[body_end].stmt_start)
          ++body_end; // the body is the single statement starting here
      }
      if (!label_comment && body_end > body_start) {
        size_t last_i = body_end;
        while (last_i > body_start &&
               skel[last_i - 1].token.type == TokenType::NEWLINE)
          --last_i;
        const FmtToken &last_in_body = skel[last_i - 1];
        const TokenType lt = last_in_body.token.type;
        if (lt == TokenType::SEMICOLON || lt == TokenType::RIGHT_BRACE) {
          const InlineResult label =
              inline_render(skel, body_start, body_end - 1, col + 2);
          if (label.ok &&
              col + static_cast<int>(label.text.size()) + 1 <= kMaxLineLength) {
            out += ' ';
            out += label.text;
            col += static_cast<int>(label.text.size()) + 1;
            line_open = true;
            last_code = &last_in_body;
            minus_unary = false;
            i = body_end - 1;
            continue;
          }
        }
      }
      minus_unary = false;
      line_open = true;
      last_code = &cur;
      continue;
    }

    minus_unary = t == TokenType::MINUS && last_code != nullptr &&
                  !value_like(last_code->token.type);
    line_open = true;
    last_code = &cur;
  }

  return out;
}

} // namespace ObSL
