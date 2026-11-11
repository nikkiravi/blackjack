#include "shoe.h"
#include <utility>

Shoe::Shoe(int numDecks) {
    if(numDecks < 0) {
        numDecks = 0; // Ensure non-negative number of decks
    }

    cards.reserve(numDecks * 52); // one allocation instead of growing step by step
    initializeShoe(numDecks);
}

void Shoe::initializeShoe(int numDecks) {
    cards.clear();
    for (int deck = 0; deck < numDecks; ++deck) {
        for (int suit = 0; suit < 4; ++suit) {
            for (int rank = 1; rank <= 13; ++rank) {
                cards.push_back(Card{static_cast<Suit>(suit), static_cast<Rank>(rank)});
            }
        }
    }
    next = 0;
}

void Shoe::shuffle(RandomGenerator& rng) {
    if(cards.empty()) {return;}

    // safe: empty case returned above
    for(size_t idx = cards.size() - 1; idx > 0; --idx) {
        size_t swapIdx = rng.below(static_cast<uint32_t>(idx + 1));
        std::swap(cards[idx], cards[swapIdx]);
    }

    next = 0; // Reset the next index after shuffling
}

Card Shoe::dealCard() {
    if(cards.empty()) {
        return Card{}; // Return a default card if the shoe is empty
    }
    if(next >= cards.size()) {
        next = 0; // Wrap around if we reach the end of the shoe
    }
    return cards[next++];
}

size_t Shoe::remainingCards() const {
    return cards.size() - next;
}