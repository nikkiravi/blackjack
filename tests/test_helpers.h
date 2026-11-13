// tests/test_helpers.h: small helpers shared by several test files
#pragma once
#include "hand.h"
#include <vector>

// builds a hand from ranks; suit doesn't matter for totals
// inline: this header is included by several .cpp files, so the linker keeps one copy (like g_failures)
inline Hand makeHand(const std::vector<Rank>& ranks) {
    Hand h;
    for (Rank r : ranks) {
        h.addCard(Card{Suit::Spades, r});
    }
    return h;
}
