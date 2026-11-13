// tests/test_payout.cpp: tests for judging a finished hand (handOutcome) and paying it (payout)
#include "check.h"
#include "test_helpers.h"
#include "payout.h"

void test_hand_outcome() {
    // --- blackjack checks come first ---
    {   // both have blackjack -> push
        Hand p = makeHand({Rank::Ace, Rank::King});
        Hand d = makeHand({Rank::Ace, Rank::Queen});
        CHECK(handOutcome(p, d, false) == Outcome::Push);
    }
    {   // player blackjack beats a dealer 20 (and isn't just a "win")
        Hand p = makeHand({Rank::Ace, Rank::King});
        Hand d = makeHand({Rank::Ten, Rank::Queen});
        CHECK(handOutcome(p, d, false) == Outcome::Blackjack);
    }
    {   // player blackjack vs a dealer 3-card 21 is still a blackjack
        Hand p = makeHand({Rank::Ace, Rank::King});
        Hand d = makeHand({Rank::Seven, Rank::Seven, Rank::Seven});
        CHECK(handOutcome(p, d, false) == Outcome::Blackjack);
    }
    {   // dealer blackjack beats a player 3-card 21
        Hand p = makeHand({Rank::Seven, Rank::Seven, Rank::Seven});
        Hand d = makeHand({Rank::Ace, Rank::King});
        CHECK(handOutcome(p, d, false) == Outcome::Lose);
    }
    {   // dealer blackjack beats a player 20
        Hand p = makeHand({Rank::King, Rank::Queen});
        Hand d = makeHand({Rank::Ace, Rank::Jack});
        CHECK(handOutcome(p, d, false) == Outcome::Lose);
    }

    // --- split hands: two-card 21 is a plain 21, not a blackjack ---
    {   // split A,K vs dealer 19 -> ordinary win (1:1)
        Hand p = makeHand({Rank::Ace, Rank::King});
        Hand d = makeHand({Rank::Ten, Rank::Nine});
        CHECK(handOutcome(p, d, true) == Outcome::Win);
    }
    {   // split A,K vs dealer 3-card 21 -> push, not blackjack
        Hand p = makeHand({Rank::Ace, Rank::King});
        Hand d = makeHand({Rank::Seven, Rank::Seven, Rank::Seven});
        CHECK(handOutcome(p, d, true) == Outcome::Push);
    }
    {   // isSplit doesn't change ordinary hands
        Hand p = makeHand({Rank::Ten, Rank::Eight});
        Hand d = makeHand({Rank::Ten, Rank::Seven});
        CHECK(handOutcome(p, d, true) == Outcome::Win);
    }

    // --- busts ---
    {   // player bust loses
        Hand p = makeHand({Rank::Ten, Rank::Six, Rank::King});
        Hand d = makeHand({Rank::Ten, Rank::Seven});
        CHECK(handOutcome(p, d, false) == Outcome::Lose);
    }
    {   // player bust loses EVEN IF the dealer also busts (the house edge)
        Hand p = makeHand({Rank::Ten, Rank::Six, Rank::King});
        Hand d = makeHand({Rank::Ten, Rank::Six, Rank::Nine});
        CHECK(handOutcome(p, d, false) == Outcome::Lose);
    }
    {   // dealer bust -> any live player hand wins, even a low one
        Hand p = makeHand({Rank::Ten, Rank::Two});
        Hand d = makeHand({Rank::Ten, Rank::Six, Rank::Nine});
        CHECK(handOutcome(p, d, false) == Outcome::Win);
    }

    // --- comparing totals ---
    {   // higher total wins
        Hand p = makeHand({Rank::Ten, Rank::Nine});
        Hand d = makeHand({Rank::Ten, Rank::Eight});
        CHECK(handOutcome(p, d, false) == Outcome::Win);
    }
    {   // lower total loses
        Hand p = makeHand({Rank::Ten, Rank::Seven});
        Hand d = makeHand({Rank::Ten, Rank::Nine});
        CHECK(handOutcome(p, d, false) == Outcome::Lose);
    }
    {   // equal totals push
        Hand p = makeHand({Rank::Ten, Rank::Eight});
        Hand d = makeHand({Rank::Nine, Rank::Nine});
        CHECK(handOutcome(p, d, false) == Outcome::Push);
    }
    {   // soft totals compare correctly: A,7 (soft 18) vs 10,8 -> push
        Hand p = makeHand({Rank::Ace, Rank::Seven});
        Hand d = makeHand({Rank::Ten, Rank::Eight});
        CHECK(handOutcome(p, d, false) == Outcome::Push);
    }
    {   // a 3-card 21 vs a dealer 20 is a win, not a blackjack
        Hand p = makeHand({Rank::Seven, Rank::Seven, Rank::Seven});
        Hand d = makeHand({Rank::King, Rank::Queen});
        CHECK(handOutcome(p, d, false) == Outcome::Win);
    }
}

void test_payout() {
    Rules r;                                  // default 3:2

    {   // $2 bet: the table from the design (amount RETURNED, bet included)
        CHECK(payout(Outcome::Lose, 2, r) == 0);
        CHECK(payout(Outcome::Push, 2, r) == 2);
        CHECK(payout(Outcome::Win, 2, r) == 4);
        CHECK(payout(Outcome::Blackjack, 2, r) == 5);
    }
    {   // bigger even bets stay exact: 3:2 on $10 returns 10 + 15
        CHECK(payout(Outcome::Win, 10, r) == 20);
        CHECK(payout(Outcome::Blackjack, 10, r) == 25);
        CHECK(payout(Outcome::Blackjack, 1000, r) == 2500);   // maxBet
    }
    {   // a doubled $2 hand is just a $4 bet
        CHECK(payout(Outcome::Win, 4, r) == 8);
    }
    {   // the payout ratio comes from the rules: 6:5 on $10 returns 10 + 12
        Rules sixFive;
        sixFive.blackjackPayNum = 6;
        sixFive.blackjackPayDen = 5;
        CHECK(payout(Outcome::Blackjack, 10, sixFive) == 22);
        CHECK(payout(Outcome::Win, 10, sixFive) == 20);       // only blackjack changes
    }
    {   // multiply BEFORE dividing: 6:5 on $4 is 4*6/5 = 24/5 = 4 (rounded down, like a casino)
        //  dividing first would give 4/5*6 = 0*6 = 0 winnings
        Rules sixFive;
        sixFive.blackjackPayNum = 6;
        sixFive.blackjackPayDen = 5;
        CHECK(payout(Outcome::Blackjack, 4, sixFive) == 8);
    }
}
