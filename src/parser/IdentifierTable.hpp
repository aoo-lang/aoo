#pragma once
#include <span>
#include <string>
#include <boost/unordered/unordered_flat_map.hpp>

namespace AO::Parser {
    typedef uint8_t u8;
    typedef uint64_t u64;
    using std::string, std::span, boost::unordered_flat_map;

    enum struct IdentifierType : u8 {
        Unknown, Variable, Function, Type, Trait, Enum, Module, Label
    };

    struct IdentifierData {
        IdentifierType type;
        u64 firstOccurrence;
    };

    namespace detail {
        inline unordered_flat_map<string, IdentifierData> identifierTable;
    }

    [[nodiscard]] inline bool addIdentifier(const span<const u8> identifier, IdentifierType type, u64 occurrence) noexcept {
        const string identifierStr{identifier.begin(), identifier.end()};
        const auto p = detail::identifierTable.find(identifierStr);
        if (p == detail::identifierTable.end()) {
            detail::identifierTable.emplace(std::move(identifierStr), IdentifierData{type, occurrence});
            return true;
        }
        else return false;
    }

    [[nodiscard]] inline IdentifierType getIdentifierType(const span<const u8> identifier) noexcept {
        const string identifierStr{identifier.begin(), identifier.end()};
        const auto p = detail::identifierTable.find(identifierStr);
        if (p == detail::identifierTable.end()) return IdentifierType::Unknown;
        else return p->second.type;
    }
}