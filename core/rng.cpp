#include "rng.h"

// -------------------------- Fake -----------------------------

FakeRandomGenerator::FakeRandomGenerator(const std::vector<uint32_t>& nums) {
    numbers = nums;
}

uint32_t FakeRandomGenerator::below(uint32_t limit) {
    if(limit == 0) {
        return 0;
    }
    if(numbers.empty()) {
        return 0;
    }
    if(next >= numbers.size()) {
        next = numbers.size() - 1; // repeat the last number if we run out, repeat the last number
    }

    uint32_t value = numbers[next] % limit;
    next++;
    return value;
}

// -------------------------- Seeded -----------------------------

SeededRandomGenerator::SeededRandomGenerator(uint32_t seed) {
    state = seed;
    if(state == 0) {
        state = 42; // xorshift's whole memory must never be 0
    }
}

uint32_t SeededRandomGenerator::below(uint32_t limit) {
    if(limit == 0) {
        return 0;
    }

    // xorshift32
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;

    return state % limit;
}