#pragma once
#include <corecrt_math.h>
#include <limits>

#include "../currentFile.hpp"
#include "tokens.hpp"

namespace AOO::Lexer {
    typedef uint8_t u8;
    typedef uint64_t u64;
    using std::numeric_limits;
    using enum TokenType;

    //This may produce floats.
    [[nodiscard]] inline Token decimal(u64& cursor, u64 origin) noexcept {
        u64 intValue = 0;
        double floatValue = 0.0;
        enum struct StopState : u8 {
            Continue, Error, ErrorGreedy, Finish
        } stopState = StopState::Continue;
        struct FloatState {
            bool gotDecimalPoint{false}, gotExponent{false};
        } floatState;
        u8 lastChar;
        while (cursor < fileContent.size()) {
            const u8 currentChar = fileContent[cursor];
            switch (currentChar) {
                case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                    if (intValue > (numeric_limits<u64>::max() / 10)) {
                        //todo: Overflow. We need to greedy until we encounter a non-digit and error out.
                        break;
                    }
                    //Currently float.
                    if (floatState.gotExponent || floatState.gotDecimalPoint) {
                        if (floatState.gotExponent) {
                            if (floatValue > (numeric_limits<double>::max() / 10.0)) {
                                //todo: Float overflow. Greedy?
                            }
                        }
                        else {

                        }
                    }
                    else intValue = intValue * 10 + (fileContent[cursor] - '0');
                    break;
                case '.': //Convert to floating point.
                    if (!gotDecimalPoint) gotDecimalPoint = true;
                    else { //Second decimal point. Treat this as the end of the literal.
                        
                    }
                    isFloatingPoint = true;
                    break;
                case 'e': case 'E': //Exponent. Convert to floating point implicitly.
                    if (!gotExponent) gotExponent = true;
                    else { //Second exponent. Treat this as the end of the literal.
                        
                    }
                    isFloatingPoint = true;
                    break;
                case 'u': //AOO specified size suffixes, unsigned integer
                    if (cursor + 1 < fileContent.size()) {
                        if (fileContent[cursor + 1] == '8') { //u8
                            
                        }
                        else if (cursor + 2 < fileContent.size()) {
                            if (fileContent[cursor + 1] == '1' && fileContent[cursor + 2] == '6') { //u16
                                
                            }
                            else if (fileContent[cursor + 1] == '3' && fileContent[cursor + 2] == '2') { //u32
                                
                            }
                            else if (fileContent[cursor + 1] == '6' && fileContent[cursor + 2] == '4') { //u64
                                
                            }
                            else { //error out
                                
                            }
                        }
                        else { //error out

                        }
                    }
                    else { // error out

                    }
                    break;
                case 'i': //AOO specified size suffixes, signed integer
                    if (cursor + 1 < fileContent.size()) {
                        if (fileContent[cursor + 1] == '8') { //i8
                            
                        }
                        else if (cursor + 2 < fileContent.size()) {
                            if (fileContent[cursor + 1] == '1' && fileContent[cursor + 2] == '6') { //i16
                                
                            }
                            else if (fileContent[cursor + 1] == '3' && fileContent[cursor + 2] == '2') { //i32
                                
                            }
                            else if (fileContent[cursor + 1] == '6' && fileContent[cursor + 2] == '4') { //i64
                                
                            }
                            else { //error out

                            }
                        }
                        else { //error out

                        }
                    }
                    else { // error out

                    }
                    break;
                case 'f': //AOO specified size suffixes, floating point
                    if (cursor + 2 < fileContent.size()) {
                        if (fileContent[cursor + 1] == '3' && fileContent[cursor + 2] == '2') { //f32
                            
                        }
                        else if (fileContent[cursor + 1] == '6' && fileContent[cursor + 2] == '4') { //f64
                            
                        }
                        else { //error out

                        }
                    }
                    else { // error out

                    }
                    break;
                case '_': case '\'': break; //Separators.
                default: //Invalid character. Error out instantly.
                    
                    return {.type = MISC_ERROR, .payload = span(fileContent.data() + origin, cursor - origin)};
            }
            if (stopState != StopState::Continue) break;
            lastChar = currentChar;
            cursor++;
        }
        switch (stopState) {
            case StopState::Continue: //File ended without being in a correct state -> error.
                return {.type = MISC_ERROR, .payload = span(fileContent.data() + origin, cursor - origin)};
            case StopState::Error:
                break;
            case StopState::ErrorGreedy:
                break;
            case StopState::Finish:
                break;
        }
    }

    //This may produce floats.
    [[nodiscard]] inline Token hex(u64& cursor, u64 origin) noexcept {
        return {.type = MISC_ERROR, .payload = {}};
    }

    [[nodiscard]] inline Token binary(u64& cursor, u64 origin) noexcept {
        return {.type = MISC_ERROR, .payload = {}};
    }

    [[nodiscard]] inline Token octal(u64& cursor, u64 origin) noexcept {
        return {.type = MISC_ERROR, .payload = {}};
    }

    [[nodiscard]] inline Token getNumberLiteral(u64& cursor) noexcept {
        using enum TokenType;

        //Temp
        cursor++;
        return {.type = GN_U8, .u8Payload = 0};
        //End Temp

        const u64 origin = cursor;
        //warning: PLEASE ONLY DETECT FOR EXACTLY WHAT YOU'RE GOING TO NEED. DO NOT BE GREEDY WHEN DETECTING EOF.
        if (fileContent[cursor] == '0' && cursor + 1 < fileContent.size()) {
            if (fileContent[cursor + 1] == 'x' || fileContent[cursor + 1] == 'X') {
                cursor += 2;
                return hex(cursor, origin);
            }
            else if (fileContent[cursor + 1] == 'b' || fileContent[cursor + 1] == 'B') {
                cursor += 2;
                return binary(cursor, origin);
            }
            else if (fileContent[cursor + 1] == 'o' || fileContent[cursor + 1] == 'O') {
                cursor += 2;
                return octal(cursor, origin);
            }
            //We put leading zeros into decimals and see what's going on. Maybe there is `0`, or `0.3`...
            //else return {.type = MISC_ERROR, .payload = {}};
        }
        return decimal(cursor, origin);
    }
}