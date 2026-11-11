// core/rng.h: random number interface (injected so tests are deterministic).

#pragma once
#include <cstdint>
#include <vector>

struct RandomGenerator {
    virtual ~RandomGenerator() = default;
    
    // returns a number from 0 to limit - 1
    virtual uint32_t below(uint32_t limit) = 0;
};

// returns scripted numbers in order (tests only)
struct FakeRandomGenerator : public RandomGenerator {
    std::vector<uint32_t> numbers; // the script
    size_t next = 0; // the position of the next number to hand out

    FakeRandomGenerator(const std::vector<uint32_t>& nums);
    uint32_t below(uint32_t limit) override;
};

// deterministic pseudo-random numbers from a seed (real play on the PC)
struct SeededRandomGenerator : public RandomGenerator {
    uint32_t state = 42; // xorshift's whole memory; must never be 0

    SeededRandomGenerator(uint32_t seed);
    uint32_t below(uint32_t limit) override;
};