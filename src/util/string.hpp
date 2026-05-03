#pragma once
#include <cstdint>

namespace Util {
    typedef uint8_t u8;
    
    [[nodiscard]] inline bool isWhitespace(char c) noexcept {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }

    [[nodiscard]] inline bool isValidIdentifierStart(char c) noexcept {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }

    [[nodiscard]] inline bool isValidIdentifierPart(char c) noexcept {
        return isValidIdentifierStart(c) || (c >= '0' && c <= '9');
    }

    [[nodiscard]] inline bool isHexDigit(char c) noexcept {
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }

    [[nodiscard]] inline u8 getHexValue(char c) noexcept {
        if (c >= '0' && c <= '9') return c - '0';
        else if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        else return 0; //Should not happen if we only call this on valid hex digits.
    }

    [[nodiscard]] inline bool isOctalDigit(char c) noexcept {
        return c >= '0' && c <= '7';
    }
}