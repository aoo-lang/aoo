#pragma once
#include <array>
#include <chrono>

namespace AOO::Manul {
    typedef uint32_t u32;
    typedef uint64_t u64;
    using std::array, std::chrono::high_resolution_clock;

    namespace detail {
        inline constexpr array<const char*, 10> splashes = {
            "Warning: Binary is too round. Attempting to compress fluff.",
            "Concurrency error: Another process detected in file territory. Initiating hiss.",
            "Flattening ears... hiding from the predator bugs (there may be some).",
            "Stamina depleted. Execution paused for a 14-hour nap.",
            "Optimization standing-on-tail-to-prevent-reference-freeze applied.",
            "Deploying ambush protocol. Waiting for valid syntax to pass by.",
            "Fatal Error: User attempted to pet the compiler!",
            "Round pupils are searching for imported modules.",
            "Compiling... Please maintain a minimum distance of 500 meters.",
            "Leave a frozen pika at the cave entrance. Do NOT knock."
        };

        struct PCG32 {

            PCG32(u64 initState, u64 initSeq) noexcept {
                state = 0U;
                inc = (initSeq << 1U) | 1U;
                next();
                state += initState;
                next();
            }

            u32 next() noexcept {
                const u64 oldState = state;
                state = oldState * 6364136223846793005ull + inc;
                const u32 xorShifted = static_cast<u32>(((oldState >> 18u) ^ oldState) >> 27u);
                const u32 rot = static_cast<u32>(oldState >> 59u);
                return (xorShifted >> rot) | (xorShifted << ((-rot) & 31));
            }

            u32 nextBound(u32 bound) noexcept {
                u64 multiResult = static_cast<u64>(next()) * bound;
                u32 leftover = multiResult;
                if (leftover < bound) {
                    const u32 threshold = -bound % bound;
                    while (leftover < threshold) {
                        multiResult = static_cast<u64>(next()) * bound;
                        leftover = multiResult;
                    }
                }
                return multiResult >> 32u;
            }

        private:
            u64 state;
            u64 inc;
        };
    }

    [[nodiscard]] inline const char* getSplash() noexcept {
        const u64 timeSeed = static_cast<u64>(high_resolution_clock::now().time_since_epoch().count()), stackSeed = reinterpret_cast<u64>(&timeSeed);
        detail::PCG32 rng(timeSeed, stackSeed);
        return detail::splashes[rng.nextBound(detail::splashes.size())];
    }
}