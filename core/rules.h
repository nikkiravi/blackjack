// core/rules.h: table rules in one place; change a field instead of hard-coding rules
#pragma once

struct Rules {
    // shoe
    int numDecks = 6;
    int penetrationPercent = 75;          // reshuffle once this % of the shoe is dealt

    // dealer
    bool dealerHitsSoft17 = false;        // false = dealer stands on all 17s

    // payouts (integers, never floats: money must be exact)
    int blackjackPayNum = 3;              // blackjack pays 3...
    int blackjackPayDen = 2;              // ...for every 2 bet

    // player options
    bool canDoubleAnyTwo = true;          // double on any first two cards
    int maxHands = 2;                     // split once -> at most 2 hands
    bool oneCardOnSplitAces = true;       // split aces get exactly one card each
    bool canResplitAces = false;
    bool canDoubleAfterSplit = true;

    // money (all even, so 3:2 blackjack pays exactly)
    int minBet = 2;
    int betStep = 2;
    int maxBet = 1000;
    int startingBankroll = 200;           // also used when resetting after going broke
};
