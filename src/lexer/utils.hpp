#pragma once
#include <span>

#include "../currentFile.hpp"
#include "tokens.hpp"

namespace AOO::Lexer {
    typedef uint64_t u64;
    using std::span;
    using enum TokenType;
    using enum StringType;

    [[nodiscard]] inline Token greedyUntilAndErrorOut(u64& cursor, u8 delimiter, u64 origin) noexcept {
        while (cursor < fileContent.size() && fileContent[cursor] != delimiter) cursor++;
        //If we stopped because we found the delimiter, consume it. We cannot flip the condition because fileContent[cursor] might be out of bounds and crash the lexer.
        if (cursor < fileContent.size()) cursor++;
        return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, cursor - origin)};
    }
}