#pragma once
#include <iostream>

#include "../currentFile.hpp"
#include "tokens.hpp"

namespace AOO::Lexer {
    typedef uint8_t u8;
    typedef uint64_t u64;
    using std::cerr;
    using enum TokenType;

    enum State : u8 {
        //We don't need start state because Lexer will only call this on 0-9.
        //Start,
        DetectBase,
        IntegerLoop,
        FractionLoop,
        ExponentLoop,
        TypeSuffix,
        Finished,
        Error
    };

    enum Base : u8 {
        Binary, Octal, Decimal, Hexadecimal,
    };

    enum struct TypeSize : u8 {
        None, U8, U16, U32, U64, I8, I16, I32, I64, F32, F64
    };

    [[nodiscard]] inline bool isValidDigitForBase(char c, Base base) noexcept {
        switch (base) {
            case Binary: return c == '0' || c == '1';
            case Octal: return c >= '0' && c <= '7';
            case Decimal: return c >= '0' && c <= '9';
            case Hexadecimal: return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
        }
    }

    [[nodiscard]] inline bool isAlphabeticDigit(char c) noexcept {
        return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }

    [[nodiscard]] inline Token convertStateToToken(bool isFloatingPoint, Base base, TypeSize typeSize, const span<const u8> literal) noexcept {
        if (isFloatingPoint) {
            if (base == Decimal) switch (typeSize) {
                case TypeSize::None: return {.type = GN_DECIMAL_FLOAT, .payload = literal};
                case TypeSize::F32: return {.type = GN_DECIMAL_FLOAT_F32, .payload = literal};
                case TypeSize::F64: return {.type = GN_DECIMAL_FLOAT_F64, .payload = literal};
                default: return {.type = MISC_ERROR, .payload = literal};
            }
            else if (base == Hexadecimal) switch (typeSize) {
                case TypeSize::None: return {.type = GN_HEX_FLOAT, .payload = literal};
                case TypeSize::F32: return {.type = GN_HEX_FLOAT_F32, .payload = literal};
                case TypeSize::F64: return {.type = GN_HEX_FLOAT_F64, .payload = literal};
                default: return {.type = MISC_ERROR, .payload = literal};
            }
            //Only Decimal and Hexadecimal can be floating-point, and floating point number literals cannot have integer typesize suffixes. Funny enough, this is the only place where this rule is enforced.
            else return {.type = MISC_ERROR, .payload = literal};
        }
        else switch (base) {
            case Binary:
                switch (typeSize) {
                    case TypeSize::None: return {.type = GN_BINARY_INT, .payload = literal};
                    case TypeSize::U8: return {.type = GN_BINARY_INT_U8, .payload = literal};
                    case TypeSize::U16: return {.type = GN_BINARY_INT_U16, .payload = literal};
                    case TypeSize::U32: return {.type = GN_BINARY_INT_U32, .payload = literal};
                    case TypeSize::U64: return {.type = GN_BINARY_INT_U64, .payload = literal};
                    case TypeSize::I8: return {.type = GN_BINARY_INT_I8, .payload = literal};
                    case TypeSize::I16: return {.type = GN_BINARY_INT_I16, .payload = literal};
                    case TypeSize::I32: return {.type = GN_BINARY_INT_I32, .payload = literal};
                    case TypeSize::I64: return {.type = GN_BINARY_INT_I64, .payload = literal};
                    default: return {.type = MISC_ERROR, .payload = literal};
                }
                break;
            case Octal:
                switch (typeSize) {
                    case TypeSize::None: return {.type = GN_OCTAL_INT, .payload = literal};
                    case TypeSize::U8: return {.type = GN_OCTAL_INT_U8, .payload = literal};
                    case TypeSize::U16: return {.type = GN_OCTAL_INT_U16, .payload = literal};
                    case TypeSize::U32: return {.type = GN_OCTAL_INT_U32, .payload = literal};
                    case TypeSize::U64: return {.type = GN_OCTAL_INT_U64, .payload = literal};
                    case TypeSize::I8: return {.type = GN_OCTAL_INT_I8, .payload = literal};
                    case TypeSize::I16: return {.type = GN_OCTAL_INT_I16, .payload = literal};
                    case TypeSize::I32: return {.type = GN_OCTAL_INT_I32, .payload = literal};
                    case TypeSize::I64: return {.type = GN_OCTAL_INT_I64, .payload = literal};
                    default: return {.type = MISC_ERROR, .payload = literal};
                }
                break;
            case Decimal:
                switch (typeSize) {
                    case TypeSize::None: return {.type = GN_DECIMAL_INT, .payload = literal};
                    case TypeSize::U8: return {.type = GN_DECIMAL_INT_U8, .payload = literal};
                    case TypeSize::U16: return {.type = GN_DECIMAL_INT_U16, .payload = literal};
                    case TypeSize::U32: return {.type = GN_DECIMAL_INT_U32, .payload = literal};
                    case TypeSize::U64: return {.type = GN_DECIMAL_INT_U64, .payload = literal};
                    case TypeSize::I8: return {.type = GN_DECIMAL_INT_I8, .payload = literal};
                    case TypeSize::I16: return {.type = GN_DECIMAL_INT_I16, .payload = literal};
                    case TypeSize::I32: return {.type = GN_DECIMAL_INT_I32, .payload = literal};
                    case TypeSize::I64: return {.type = GN_DECIMAL_INT_I64, .payload = literal};
                    default: return {.type = MISC_ERROR, .payload = literal};
                }
                break;
            case Hexadecimal:
                switch (typeSize) {
                    case TypeSize::None: return {.type = GN_HEX_INT, .payload = literal};
                    case TypeSize::U8: return {.type = GN_HEX_INT_U8, .payload = literal};
                    case TypeSize::U16: return {.type = GN_HEX_INT_U16, .payload = literal};
                    case TypeSize::U32: return {.type = GN_HEX_INT_U32, .payload = literal};
                    case TypeSize::U64: return {.type = GN_HEX_INT_U64, .payload = literal};
                    case TypeSize::I8: return {.type = GN_HEX_INT_I8, .payload = literal};
                    case TypeSize::I16: return {.type = GN_HEX_INT_I16, .payload = literal};
                    case TypeSize::I32: return {.type = GN_HEX_INT_I32, .payload = literal};
                    case TypeSize::I64: return {.type = GN_HEX_INT_I64, .payload = literal};
                    default: return {.type = MISC_ERROR, .payload = literal};
                }
                break;
        }
    }

    [[nodiscard]] inline Token getNumberLiteral(u64& cursor) noexcept {
        const u64 origin = cursor;

        State state{DetectBase};
        Base base;
        TypeSize typeSize{TypeSize::None};
        bool isFloatingPoint = false;
        u8 lastChar = 0;

        while (cursor < fileContent.size()) {
            bool internalStop_DoNotUseInAnyOtherScenario_ThatsWhyItHasSuchAScaryName_LOL = false;
            switch (state) {
                case DetectBase: {
                    switch (fileContent[cursor]) {
                        case '0':
                            if (cursor + 1 < fileContent.size()) {
                                switch (fileContent[cursor + 1]) {
                                    case 'x': case 'X':
                                        base = Hexadecimal;
                                        lastChar = fileContent[cursor + 1];
                                        cursor += 2;
                                        break;
                                    case 'b': case 'B':
                                        base = Binary;
                                        lastChar = fileContent[cursor + 1];
                                        cursor += 2;
                                        break;
                                    case 'o': case 'O':
                                        base = Octal;
                                        lastChar = fileContent[cursor + 1];
                                        cursor += 2;
                                        break;
                                    default:
                                        //Eat the valid 0.
                                        lastChar = '0';
                                        cursor++;
                                        base = Decimal;
                                        break;
                                }
                            }
                            else { //Literal 0 at the end of the file.
                                lastChar = '0';
                                cursor++;
                                return {.type = GN_DECIMAL_INT, .payload = span(fileContent.data() + origin, 1)};
                            }
                            break;
                        case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                            base = Decimal;
                            //Eat the valid digit.
                            lastChar = fileContent[cursor];
                            cursor++;
                            break;
                        //default: //Invalid digit. This should never happen because Lexer will only call this function on 0-9.
                    }
                    state = IntegerLoop;
                    break;
                }
                case IntegerLoop: case FractionLoop: case ExponentLoop: {
                    const char c = fileContent[cursor];
                    switch (c) {
                        //Digit *OR* decimal floating-point.
                        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case 'a': case 'A': case 'b': case 'B': case 'c': case 'C': case 'd': case 'D': case 'e': case 'E': case 'f': case 'F':
                            switch (base) {
                                case Hexadecimal:
                                    if (state == ExponentLoop && isAlphabeticDigit(c)) state = Error; //Exponent part must be in decimal, not hexadecimal.
                                    lastChar = c;
                                    cursor++;
                                    break;
                                case Decimal:
                                    if (isAlphabeticDigit(c)) {
                                        //We need decimal floating-point literals to use "f32"/"f64" suffixes instead of hexadecimal's "_f32"/"_f64".
                                        if (c == 'f') {
                                            //Only 0-9 can be a valid digit before the type suffix.
                                            if (isValidDigitForBase(lastChar, Decimal)) state = TypeSuffix;
                                            else state = Error;
                                            //There is no `cursor++` because we need it to be checked in the `TypeSuffix` state as well.
                                            //No `lastChar = c` as well.
                                        }
                                        else if (c == 'e' || c == 'E') {
                                            if (state == ExponentLoop || !isValidDigitForBase(lastChar, Decimal)) state = Error; //Multiple exponent indicators / direct separator before exponent indicator.
                                            else {
                                                isFloatingPoint = true;
                                                state = ExponentLoop;
                                            }
                                            lastChar = c;
                                            cursor++;
                                        }
                                        else {
                                            state = Error;
                                            lastChar = c;
                                            cursor++;
                                        }
                                    }
                                    else {
                                        //Eat the valid digit.
                                        lastChar = c;
                                        cursor++;
                                    }
                                    break;
                                case Octal:
                                    if (!isValidDigitForBase(c, Octal)) state = Error;
                                    lastChar = c;
                                    cursor++;
                                    break;
                                case Binary:
                                    if (!isValidDigitForBase(c, Binary)) state = Error;
                                    lastChar = c;
                                    cursor++;
                                    break;
                            }
                            break;
                        case '.': //Fractional part.
                            if (state == FractionLoop || state == ExponentLoop) state = Error; //Multiple dots / dot after exponent indicator.
                            else if (base == Decimal) {
                                if (!isValidDigitForBase(lastChar, Decimal)) state = Error;
                                else {
                                    isFloatingPoint = true;
                                    state = FractionLoop;
                                }
                            }
                            else if (base == Hexadecimal) {
                                if (!isValidDigitForBase(lastChar, Hexadecimal)) state = Error;
                                else {
                                    isFloatingPoint = true;
                                    state = FractionLoop;
                                }
                            }
                            else state = Error;
                            lastChar = c;
                            cursor++;
                            break;
                        case 'p': case 'P': //Hexadecimal floating-point.
                            if (state == ExponentLoop) state = Error; //Multiple exponent indicators.
                            else if (base == Hexadecimal) {
                                if (!isValidDigitForBase(lastChar, Hexadecimal)) state = Error;
                                else {
                                    isFloatingPoint = true;
                                    state = ExponentLoop;
                                }
                            }
                            else state = Error;
                            lastChar = c;
                            cursor++;
                            break;
                        case 'u': case 'i': case '_': //Type suffixes.
                            //Use "f32"/"f64" for decimal floating-point literals and "_f32"/"_f64" for hexadecimal floating-point literals. This is necessary and mandatory.
                            if (base == Decimal && c == '_') state = Error;
                            else if (!isValidDigitForBase(lastChar, base)) state = Error;
                            else state = TypeSuffix;
                            //There is no `cursor++` because we need it to be checked in the TypeSuffix state as well.
                            //No `lastChar = c` as well.
                            break;
                        case '+': case '-': //Exponent sign.
                            if (state == ExponentLoop) {
                                if (lastChar != 'e' && lastChar != 'E' && lastChar != 'p' && lastChar != 'P') state = Error;
                                //Else: Do nothing. WE DON'T NEED TO PARSE IT!
                                //else
                                lastChar = c;
                                cursor++;
                                break;
                            }
                            else [[fallthrough]]; //Equivalent to `default:`.
                        //Separator or other characters. Check if the previous character is a digit.
                        //For separator, the next character being a digit will be checked in the respective cases in the next iteration.
                        //The only difference between separator and other characters is that the state will be finished for other characters.
                        case '\'': default:
                            if (isValidDigitForBase(lastChar, base)) {
                                //Eat the separator.
                                if (c == '\'') {
                                    lastChar = c;
                                    cursor++;
                                }
                                //`default:`: Other character following a valid digit means the literal is finished.
                                //Don't eat this character because it might be the start of the next token.
                                else state = Finished;
                            }
                            //Don't eat this character because it might be the start of the next token.
                            else state = Error;
                            break;
                    }
                    break;
                }
                case TypeSuffix:
                    switch (fileContent[cursor]) {
                        case 'u': case 'i':
                            if (cursor + 1 < fileContent.size()) {
                                if (fileContent[cursor + 1] == '8') {
                                    state = Finished;
                                    typeSize = (fileContent[cursor] == 'u') ? TypeSize::U8 : TypeSize::I8;
                                    lastChar = '8';
                                    cursor += 2;
                                }
                                else if (cursor + 2 < fileContent.size()) {
                                    if (fileContent[cursor + 1] == '1' && fileContent[cursor + 2] == '6') {
                                        state = Finished;
                                        typeSize = (fileContent[cursor] == 'u') ? TypeSize::U16 : TypeSize::I16;
                                        lastChar = '6';
                                        cursor += 3;
                                    }
                                    else if (fileContent[cursor + 1] == '3' && fileContent[cursor + 2] == '2') {
                                        state = Finished;
                                        typeSize = (fileContent[cursor] == 'u') ? TypeSize::U32 : TypeSize::I32;
                                        lastChar = '2';
                                        cursor += 3;
                                    }
                                    else if (fileContent[cursor + 1] == '6' && fileContent[cursor + 2] == '4') {
                                        state = Finished;
                                        typeSize = (fileContent[cursor] == 'u') ? TypeSize::U64 : TypeSize::I64;
                                        lastChar = '4';
                                        cursor += 3;
                                    }
                                    else {
                                        //Include the two invalid characters after 'u'/'i' in the error.
                                        lastChar = fileContent[cursor + 2];
                                        cursor += 3;
                                        state = Error;
                                    }
                                }
                                else {
                                    //Include the invalid character after 'u'/'i' in the error.
                                    lastChar = fileContent[cursor + 1];
                                    cursor += 2;
                                    state = Error;
                                }
                            }
                            else {
                                //Include 'u'/'i' at the end of the file in the error.
                                lastChar = fileContent[cursor];
                                cursor++;
                                state = Error;
                            }
                            break;
                        case '_':
                            if (cursor + 3 >= fileContent.size()) {
                                lastChar = fileContent[cursor];
                                cursor++;
                                state = Error;
                                break;
                            }
                            else if (fileContent[cursor + 1] != 'f') {
                                lastChar = fileContent[cursor + 1];
                                cursor += 2;
                                state = Error;
                                break;
                            }
                            else {
                                //Eat the '_' character.
                                cursor++;
                                [[fallthrough]];
                            }
                        case 'f':
                            if (cursor + 2 < fileContent.size()) {
                                if (fileContent[cursor + 1] == '3' && fileContent[cursor + 2] == '2') {
                                    typeSize = TypeSize::F32;
                                    state = Finished;
                                    lastChar = '2';
                                }
                                else if (fileContent[cursor + 1] == '6' && fileContent[cursor + 2] == '4') {
                                    typeSize = TypeSize::F64;
                                    state = Finished;
                                    lastChar = '4';
                                }
                                else {
                                    state = Error;
                                    //Include the invalid characters in the error.
                                    lastChar = fileContent[cursor + 2];
                                }
                                cursor += 3;
                            }
                            else {
                                state = Error;
                                lastChar = 'f';
                                //Include 'f'.
                                cursor++;
                            }
                            break;
                        default: //We should never get here.
                            lastChar = fileContent[cursor];
                            cursor++;
                            state = Error;
                            break;
                    }
                    break;
                //Put cursor after the literal before going to here!
                case Finished: case Error:
                    internalStop_DoNotUseInAnyOtherScenario_ThatsWhyItHasSuchAScaryName_LOL = true;
                    break;
            }
            if (internalStop_DoNotUseInAnyOtherScenario_ThatsWhyItHasSuchAScaryName_LOL) break;
            //PLEASE INCREMENT `cursor` MANUALLY IN EACH CASE.
            //cursor++;
            //PLEASE UPDATE `lastChar` MANUALLY IN EACH CASE AS WELL IF NECESSARY.
            //lastChar = fileContent[cursor];
        }

        const auto literalRange = span(fileContent.data() + origin, cursor - origin);

        switch (state) {
            case Finished: return convertStateToToken(isFloatingPoint, base, typeSize, literalRange);
            case Error: return {.type = MISC_ERROR, .payload = literalRange};

            //Below scenario: file ended. THIS is the hardest part: Figure out if the literal is valid or not without looking back arbitrarily (because we need to comply with streaming).

            case DetectBase: //`DetectBase` is a transient state, so the only way to end the file in this state is the cursor hit the end without even eating the first character. So it's a straight up EOF. But what I really want to know is, how did we get here?
                cerr << "How did we get here?\n";
                return {.type = MISC_EOF};

            case IntegerLoop: //The only way to end the file in this state is the cursor hit the end right after eating a valid digit, so it's definitely a valid integer literal.
                return convertStateToToken(isFloatingPoint, base, typeSize, literalRange);

            case FractionLoop: //Check if the fractional part is valid -> the only way to be an invalid fractional part then end the file is that the last character is the dot.
                if (lastChar == '.') return {.type = MISC_ERROR, .payload = literalRange}; //Dot cannot be the last character.
                else return convertStateToToken(isFloatingPoint, base, typeSize, literalRange);

            case ExponentLoop: //Check if the exponent is valid number -> the only way to be an invalid exponent then end the file is that the last character is the exponent indicator.
                if (lastChar == 'e' || lastChar == 'E' || lastChar == 'p' || lastChar == 'P') return {.type = MISC_ERROR, .payload = literalRange}; //Exponent indicator cannot be the last character.
                else return convertStateToToken(isFloatingPoint, base, typeSize, literalRange);

            case TypeSuffix: //`TypeSuffix` is a transient state, so the only way to end the file in this state is the cursor hit the end right after 'u'/'i'/'_', so it's definitely an error.
                return {.type = MISC_ERROR, .payload = literalRange};
        }
    }
}