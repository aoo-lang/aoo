#pragma once
#include <span>
#include <string_view>

namespace Util {
    typedef uint8_t u8;
    using std::span, std::string_view;

    template <size_t length>
    [[nodiscard]] inline bool equals(const span<const u8> a, const char(& b)[length]) noexcept {
        if (a.size() != length - 1) return false;
        //They are all empty.
        if constexpr (length == 1) return true;
        return memcmp(a.data(), b, a.size()) == 0;
    }

    [[nodiscard]] inline bool equals(const span<const u8> a, const string_view b) noexcept {
        if (a.size() != b.size()) return false;
        //They are all empty.
        if (a.empty()) return true;
        return memcmp(a.data(), b.data(), a.size()) == 0;
    }
}