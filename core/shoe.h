//core/shoe.h handles the card dealing and shoe management for the blackjack game. 

#pragma once
#include "card.h"
#include "rng.h"
#include <vector>

struct Shoe {
    std::vector<Card> cards; 
    size_t next = 0; // Index of the next card to be dealt

    Shoe(int numDecks = 6); 
    void shuffle(RandomGenerator& rng);
    Card dealCard(); // safety net when empty: wraps to the start (game reshuffles before this happens)
    size_t remainingCards() const; 

private:
    void initializeShoe(int numDecks);
};
