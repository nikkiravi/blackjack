// core/payout.h: how a finished hand is judged and paid

#pragma once
#include <cstdint>
#include "hand.h"
#include "rules.h"

enum class Outcome : uint8_t { Lose, Win, Push, Blackjack };

// isSplit: a split hand's 21 is not a blackjack (pays 1:1)
Outcome handOutcome(const Hand& playerHand, const Hand& dealerHand, bool isSplit);

// amount returned to bankroll, including bet
int payout(Outcome result, int bet, const Rules& tableRules);