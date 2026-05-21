#pragma once
#include <vector>

#include "tokens.hpp"

namespace AOO::Lexer::internal {
    typedef uint64_t u64;
    using std::vector;

    inline u64 cursor{0};
    inline vector<Token> tokens;
}