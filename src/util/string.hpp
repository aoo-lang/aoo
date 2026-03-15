#pragma once

namespace Util {
    
    [[nodiscard]] inline bool isWhitespace(char c) noexcept {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }

    [[nodiscard]] inline bool isValidIdentifierStart(char c) noexcept {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }

    [[nodiscard]] inline bool isValidIdentifierPart(char c) noexcept {
        return isValidIdentifierStart(c) || (c >= '0' && c <= '9');
    }
}