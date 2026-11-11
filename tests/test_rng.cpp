// tests/test_rng.cpp: tests for the random generators
#include "check.h"
#include "rng.h"

void test_fake_rng() {
    {   // returns the script in order
        FakeRandomGenerator rng({5, 10, 15});
        CHECK(rng.below(20) == 5);
        CHECK(rng.below(20) == 10);
        CHECK(rng.below(20) == 15);
    }
    {   // after the script runs out, repeats the last number
        FakeRandomGenerator rng({5, 10, 15});
        rng.below(20);
        rng.below(20);
        rng.below(20);
        CHECK(rng.below(20) == 15);
        CHECK(rng.below(20) == 15);
    }
    {   // scripted number >= limit is narrowed into range: 7 % 3 == 1
        FakeRandomGenerator rng({7});
        CHECK(rng.below(3) == 1);
    }
    {   // empty script returns 0 instead of reading past the end
        FakeRandomGenerator rng({});
        CHECK(rng.below(5) == 0);
    }
    {   // limit 0 returns 0 instead of dividing by zero
        FakeRandomGenerator rng({5});
        CHECK(rng.below(0) == 0);
    }
}

void test_seeded_rng() {
    {   // same seed -> same sequence
        SeededRandomGenerator a(42);
        SeededRandomGenerator b(42);
        for (int i = 0; i < 5; i++) {
            CHECK(a.below(1000) == b.below(1000));
        }
    }
    {   // different seeds -> different sequences (at least one number differs)
        SeededRandomGenerator a(1);
        SeededRandomGenerator b(2);
        bool anyDifferent = false;
        for (int i = 0; i < 5; i++) {
            if (a.below(1000) != b.below(1000)) {
                anyDifferent = true;
            }
        }
        CHECK(anyDifferent);
    }
    {   // results always stay in range 0..limit-1
        SeededRandomGenerator rng(7);
        bool allInRange = true;
        for (int i = 0; i < 1000; i++) {
            if (rng.below(6) >= 6) {
                allInRange = false;
            }
        }
        CHECK(allInRange);
    }
    {   // seed 0 must not get stuck at 0 forever (xorshift's zero trap)
        SeededRandomGenerator rng(0);
        bool anyNonZero = false;
        for (int i = 0; i < 5; i++) {
            if (rng.below(1000) != 0) {
                anyNonZero = true;
            }
        }
        CHECK(anyNonZero);
    }
    {   // limit 0 returns 0 instead of dividing by zero
        SeededRandomGenerator rng(42);
        CHECK(rng.below(0) == 0);
    }
    {   // golden values: tripwire if the algorithm is accidentally changed
        SeededRandomGenerator rng(42);
        CHECK(rng.below(20) == 12);
        CHECK(rng.below(20) == 8);
        CHECK(rng.below(20) == 19);
        CHECK(rng.below(20) == 16);
    }
}
