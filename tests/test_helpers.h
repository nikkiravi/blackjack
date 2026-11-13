// tests/test_helpers.h: small helpers shared by several test files
#pragma once
#include "hand.h"
#include "game.h"
#include <vector>

// inline: this header is included by several .cpp files, so the linker keeps one copy (like g_failures)

// builds a hand from ranks; suit doesn't matter for totals
inline Hand makeHand(const std::vector<Rank>& ranks) {
    Hand h;
    for (Rank r : ranks) {
        h.addCard(Card{Suit::Spades, r});
    }
    return h;
}

// replaces the game's shoe with these ranks, dealt in this exact order
// deal order is: player, dealer, player, dealer, then any hits/splits/dealer draws
// list every card the test needs: past the end the shoe wraps back to the first card
inline void stackShoe(Game& g, const std::vector<Rank>& ranks) {
    g.shoe.cards.clear();
    for (Rank r : ranks) {
        g.shoe.cards.push_back(Card{Suit::Spades, r});
    }
    g.shoe.next = 0;
}
