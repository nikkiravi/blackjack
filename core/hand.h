// core/hand.h: a hand of cards and its blackjack math (shared by player and dealer)
#pragma once
#include "card.h"
#include <vector>

struct Hand {
    std::vector<Card> cards;

    void addCard(const Card& card);

    // total is computed from cards, never stored, so it can't go out of sync
    // aces count as 1, and one ace is promoted to 11 if that doesn't bust
    int total() const;
    // true if an ace is currently counting as 11
    bool isSoft() const;
    bool isBust() const;
    // exactly two cards totaling 21 (split hands are handled by PlayerHand later)
    bool isBlackjack() const;

    // reserves card space up front; reuse with cards.clear() between rounds
    Hand();
};

// player-only facts live here, so the dealer's Hand stays bet-free
struct PlayerHand {
    Hand hand;
    int bet = 0;
    bool doubled = false;
};