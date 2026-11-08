// tests/test_card.cpp: tests for Card
#include "check.h"
#include "card.h"

void test_card_value() {
    {   // Ace: always 1 here; Hand decides when it counts as 11
        Card c{Suit::Hearts, Rank::Ace};
        CHECK(c.value() == 1);
    }
    {   // Nine: last card worth its own number
        Card c{Suit::Spades, Rank::Nine};
        CHECK(c.value() == 9);
    }
    {   // Ten: first card worth 10
        Card c{Suit::Diamonds, Rank::Ten};
        CHECK(c.value() == 10);
    }
    {   // Jack: first face card
        Card c{Suit::Hearts, Rank::Jack};
        CHECK(c.value() == 10);
    }
    {   // Queen
        Card c{Suit::Clubs, Rank::Queen};
        CHECK(c.value() == 10);
    }
    {   // King: last rank in the enum
        Card c{Suit::Hearts, Rank::King};
        CHECK(c.value() == 10);
    }
    {   // default suit is Hearts
        Card c{};
        CHECK(c.suit == Suit::Hearts);
    }
    {   // default rank is Ace
        Card c{Suit::Diamonds};
        CHECK(c.rank == Rank::Ace);
    }
}
