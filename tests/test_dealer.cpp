// tests/test_dealer.cpp: tests for the dealer's hit/stand decision under S17 and H17
#include "check.h"
#include "test_helpers.h"
#include "dealer.h"

void test_dealer() {
    Rules s17;                                // default: dealer stands on all 17s
    Rules h17;
    h17.dealerHitsSoft17 = true;

    {   // the default table is S17
        CHECK(!s17.dealerHitsSoft17);
    }
    {   // hard 16 -> hit under both rules
        Hand h = makeHand({Rank::Ten, Rank::Six});
        CHECK(dealerShouldHit(h, s17));
        CHECK(dealerShouldHit(h, h17));
    }
    {   // soft 16 -> hit under both rules
        Hand h = makeHand({Rank::Ace, Rank::Five});
        CHECK(dealerShouldHit(h, s17));
        CHECK(dealerShouldHit(h, h17));
    }
    {   // hard 17 -> stand under both rules (the flag is only about SOFT 17)
        Hand h = makeHand({Rank::Ten, Rank::Seven});
        CHECK(!dealerShouldHit(h, s17));
        CHECK(!dealerShouldHit(h, h17));
    }
    {   // soft 17 (A,6) -> the one case where the rules disagree
        Hand h = makeHand({Rank::Ace, Rank::Six});
        CHECK(!dealerShouldHit(h, s17));
        CHECK(dealerShouldHit(h, h17));
    }
    {   // multi-card soft 17 (A,2,4) behaves the same as A,6
        Hand h = makeHand({Rank::Ace, Rank::Two, Rank::Four});
        CHECK(!dealerShouldHit(h, s17));
        CHECK(dealerShouldHit(h, h17));
    }
    {   // hard 17 that contains an ace (A,6,K: ace must count as 1) -> stand under both
        Hand h = makeHand({Rank::Ace, Rank::Six, Rank::King});
        CHECK(!dealerShouldHit(h, s17));
        CHECK(!dealerShouldHit(h, h17));
    }
    {   // soft 18 -> stand under both rules
        Hand h = makeHand({Rank::Ace, Rank::Seven});
        CHECK(!dealerShouldHit(h, s17));
        CHECK(!dealerShouldHit(h, h17));
    }
    {   // hard 20 -> stand
        Hand h = makeHand({Rank::King, Rank::Queen});
        CHECK(!dealerShouldHit(h, s17));
        CHECK(!dealerShouldHit(h, h17));
    }
    {   // busted (22) -> stand; the dealer never draws past a bust
        Hand h = makeHand({Rank::King, Rank::Queen, Rank::Two});
        CHECK(!dealerShouldHit(h, s17));
        CHECK(!dealerShouldHit(h, h17));
    }
    {   // empty hand (total 0) -> hit; the dealer always starts by drawing
        Hand h;
        CHECK(dealerShouldHit(h, s17));
    }
}
