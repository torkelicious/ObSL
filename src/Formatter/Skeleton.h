#pragma once

#include <ObSL/Tokens.h>

#include <cstdint>
#include <vector>

namespace ObSL {

    enum class BraceKind : uint8_t {
        None,
        BlockBody,
        StructBody,
        Parens,
        ArrayLit,
        Index,
    };

    enum class Trivia : uint8_t {
        None,
        LineComment,
    };

    struct FmtToken {
        Token token;
        Trivia trivia = Trivia::None;
        bool stmt_start = false;
        bool degraded = false;
        BraceKind brace_kind = BraceKind::None;
        int match = -1;
    };

    std::vector<FmtToken> annotate(const std::vector<Token> &tokens);

} // namespace ObSL
