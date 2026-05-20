#pragma once
#include <cstdint>
#include <string_view>
#include <vector>

namespace AOO::Parser {
    typedef uint8_t u8;
    typedef uint32_t u32;
    using std::string_view, std::vector;

    enum struct ErrorKind : u8 {
        UnexpectedToken,
        ExpectedExpression,
        ExpectedType,
        ExpectedIdentifier,
        ExpectedSemicolon,
        ExpectedRightParen,
        ExpectedRightBrace,
        ExpectedRightBracket,
        ExpectedRightAngle,
        UnclosedGenericArgs,
        ComparisonChained,
        RangeChained,
        BadStructLitInCondition,
        BangBangPrefix,
        IncDecPrefix,
        UnknownDeclaration,
        TruncatedInput,
        InternalBug
    };

    struct ParseError {
        ErrorKind kind;
        u32 tokenIndex;
        string_view detail;
    };

    [[nodiscard]] inline const char* tostring_ErrorKind(ErrorKind kind) noexcept {
        using enum ErrorKind;
        switch (kind) {
            case UnexpectedToken: return "UnexpectedToken";
            case ExpectedExpression: return "ExpectedExpression";
            case ExpectedType: return "ExpectedType";
            case ExpectedIdentifier: return "ExpectedIdentifier";
            case ExpectedSemicolon: return "ExpectedSemicolon";
            case ExpectedRightParen: return "ExpectedRightParen";
            case ExpectedRightBrace: return "ExpectedRightBrace";
            case ExpectedRightBracket: return "ExpectedRightBracket";
            case ExpectedRightAngle: return "ExpectedRightAngle";
            case UnclosedGenericArgs: return "UnclosedGenericArgs";
            case ComparisonChained: return "ComparisonChained";
            case RangeChained: return "RangeChained";
            case BadStructLitInCondition: return "BadStructLitInCondition";
            case BangBangPrefix: return "BangBangPrefix";
            case IncDecPrefix: return "IncDecPrefix";
            case UnknownDeclaration: return "UnknownDeclaration";
            case TruncatedInput: return "TruncatedInput";
            case InternalBug: return "InternalBug";
        }
    }
}