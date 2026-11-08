// core/card.h: how a card is represented
#pragma once
#include <cstdint>

enum class Suit : uint8_t { Hearts, Diamonds, Clubs, Spades };
enum class Rank : uint8_t { 
    Ace = 1, 
    Two,
    Three,
    Four, 
    Five, 
    Six, 
    Seven, 
    Eight, 
    Nine, 
    Ten, 
    Jack, 
    Queen, 
    King
};

struct Card {
    Suit suit = Suit::Hearts;
    Rank rank = Rank::Ace;

    int value() const;
};