// tests/test_shoe.cpp: tests for building, shuffling, and dealing from the Shoe
#include "check.h"
#include "shoe.h"
#include "rng.h"


static bool sameCard(const Card& a, const Card& b) {
    return a.suit == b.suit && a.rank == b.rank;
}

// true if every rank 1..13 appears exactly `expected` times in the shoe
static bool eachRankAppears(const Shoe& s, int expected) {
    int counts[14] = {};                      
    for (const Card& c : s.cards) {
        counts[static_cast<int>(c.rank)]++;
    }
    for (int r = 1; r <= 13; r++) {
        if (counts[r] != expected) {
            return false;
        }
    }
    return true;
}

void test_shoe() {
    {   // 1. default shoe is 6 decks = 312 cards
        Shoe s;
        CHECK(s.cards.size() == 312);
        CHECK(s.remainingCards() == 312);
    }
    {   // 2. 6 decks -> each rank appears 6 decks x 4 suits = 24 times
        Shoe s;
        CHECK(eachRankAppears(s, 24));
    }
    {   // 3. one deck: 52 cards, every (suit, rank) pair exactly once
        Shoe s(1);
        CHECK(s.cards.size() == 52);
        int seen[4][14] = {};                 // [suit][rank]
        for (const Card& c : s.cards) {
            seen[static_cast<int>(c.suit)][static_cast<int>(c.rank)]++;
        }
        bool eachOnce = true;
        for (int suit = 0; suit < 4; suit++) {
            for (int rank = 1; rank <= 13; rank++) {
                if (seen[suit][rank] != 1) {
                    eachOnce = false;
                }
            }
        }
        CHECK(eachOnce);
    }
    {   // 4. dealing decreases remainingCards
        Shoe s(1);
        s.dealCard();
        s.dealCard();
        s.dealCard();
        CHECK(s.remainingCards() == 49);
    }
    {   // 5. unshuffled build order is predictable: Ace of Hearts, then Two of Hearts
        Shoe s(1);
        CHECK(sameCard(s.dealCard(), Card{Suit::Hearts, Rank::Ace}));
        CHECK(sameCard(s.dealCard(), Card{Suit::Hearts, Rank::Two}));
    }
    {   // 6. shuffling never loses or duplicates cards
        Shoe s;
        SeededRandomGenerator rng(42);
        s.shuffle(rng);
        CHECK(s.cards.size() == 312);
        CHECK(eachRankAppears(s, 24));
    }
    {   // 7. same seed -> identical shuffled order, card by card
        Shoe a;
        Shoe b;
        SeededRandomGenerator rngA(42);
        SeededRandomGenerator rngB(42);
        a.shuffle(rngA);
        b.shuffle(rngB);
        bool identical = true;
        for (size_t i = 0; i < a.cards.size(); i++) {
            if (!sameCard(a.cards[i], b.cards[i])) {
                identical = false;
            }
        }
        CHECK(identical);
    }
    {   // 8. different seeds -> orders differ somewhere
        Shoe a;
        Shoe b;
        SeededRandomGenerator rngA(1);
        SeededRandomGenerator rngB(2);
        a.shuffle(rngA);
        b.shuffle(rngB);
        bool anyDifferent = false;
        for (size_t i = 0; i < a.cards.size(); i++) {
            if (!sameCard(a.cards[i], b.cards[i])) {
                anyDifferent = true;
            }
        }
        CHECK(anyDifferent);
    }
    {   // 9. fake RNG always returning 0: Fisher-Yates swaps each position i with 0,
        //    which rotates the deck left by one: [A,B,C,D] -> [B,C,D,A]
        Shoe s(1);
        FakeRandomGenerator rng({0});         // repeats 0 forever
        s.shuffle(rng);
        CHECK(sameCard(s.cards[0],  Card{Suit::Hearts, Rank::Two}));     // was position 1
        CHECK(sameCard(s.cards[12], Card{Suit::Diamonds, Rank::Ace}));   // was position 13
        CHECK(sameCard(s.cards[51], Card{Suit::Hearts, Rank::Ace}));     // was position 0
    }
    {   // 10. shuffling resets the deal position
        Shoe s(1);
        SeededRandomGenerator rng(42);
        s.dealCard();
        s.dealCard();
        s.shuffle(rng);
        CHECK(s.remainingCards() == 52);
    }
    {   // 11. zero or negative decks -> empty shoe that fails safely
        Shoe zero(0);
        Shoe negative(-1);
        CHECK(zero.cards.empty());
        CHECK(negative.cards.empty());
        CHECK(zero.remainingCards() == 0);
        CHECK(sameCard(zero.dealCard(), Card{}));           // safety net, no crash
        SeededRandomGenerator rng(42);
        zero.shuffle(rng);                                  // must not crash or loop forever
        CHECK(zero.cards.empty());
    }
    {   // 12. dealing past the end wraps around: card 53 == card 1
        Shoe s(1);
        Card first = s.dealCard();
        for (int i = 0; i < 51; i++) {
            s.dealCard();
        }
        CHECK(s.remainingCards() == 0);
        CHECK(sameCard(s.dealCard(), first));
    }
}
