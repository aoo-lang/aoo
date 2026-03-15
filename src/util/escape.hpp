#pragma once

namespace Util {
    [[nodiscard]] inline char unescapeChar(char c) noexcept {
        switch (c) {
            case 'a': return '\a';
            case 'b': return '\b';
            case 'f': return '\f';
            case 'n': return '\n';
            case 'r': return '\r';
            case 't': return '\t';
            case 'v': return '\v';
            case '\\': return '\\';
            case '\'': return '\'';
            case '"': return '"';
            case '?': return '\?';
            case 'e': case 'E': return '\x1B';
            default: return c;
        }
    }
}