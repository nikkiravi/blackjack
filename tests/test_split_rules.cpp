// tests/test_split_rules.cpp: tests for canSplit, split-aces rules, and skipping the dealer when every hand busts
#include "check.h"
#include "test_helpers.h"
#include "game.h"

void test_can_split() {
    {   // a pair on the first two cards -> allowed
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Eight, Rank::Ten, Rank::Eight, Rank::Seven});
        g.handleEvent(Event::Deal);
        CHECK(g.canSplit());
    }
    {   // not a pair -> not allowed
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Eight, Rank::Ten, Rank::Nine, Rank::Seven});
        g.handleEvent(Event::Deal);
        CHECK(!g.canSplit());
    }
    {   // outside PlayerTurn -> not allowed, and checking with no hands dealt doesn't crash
        SeededRandomGenerator rng(42);
        Game g(rng);
        CHECK(g.player_hands.empty());
        CHECK(!g.canSplit());
    }
    {   // three cards -> not allowed, even if the first two were a pair
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Two, Rank::Ten, Rank::Two, Rank::Seven, Rank::Three});
        g.handleEvent(Event::Deal);                   // P 2,2
        g.handleEvent(Event::Hit);                    // P 2,2,3
        CHECK(!g.canSplit());
    }
    {   // can't afford the second bet -> not allowed
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Eight, Rank::Ten, Rank::Eight, Rank::Seven});
        g.handleEvent(Event::Deal);
        g.bankroll = 1;
        CHECK(!g.canSplit());
    }
    {   // maxHands = 2 (default): a new pair after splitting can't be split again
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Eight, Rank::Ten, Rank::Eight, Rank::Seven,
                      Rank::Eight, Rank::Two});                            // hand 0 becomes 8,8 again
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);
        CHECK(!g.canSplit());
    }
    {   // maxHands = 3: the same new pair CAN be split again
        SeededRandomGenerator rng(42);
        Rules r;
        r.maxHands = 3;
        Game g(rng, r);
        stackShoe(g, {Rank::Eight, Rank::Ten, Rank::Eight, Rank::Seven,
                      Rank::Eight, Rank::Two});
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);
        CHECK(g.canSplit());
    }
}

void test_resplit_aces() {
    // these use maxHands = 3 (so a second split has room) and oneCardOnSplitAces = false
    // (otherwise the split aces would finish automatically before anyone could resplit)
    {   // the FIRST split of aces is always allowed, even with canResplitAces off
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ace, Rank::Ten, Rank::Ace, Rank::Nine});
        g.handleEvent(Event::Deal);
        CHECK(!g.rules.canResplitAces);
        CHECK(g.canSplit());
    }
    {   // canResplitAces = false: a new ace pair after splitting aces can't be split
        SeededRandomGenerator rng(42);
        Rules r;
        r.maxHands = 3;
        r.oneCardOnSplitAces = false;
        r.canResplitAces = false;
        Game g(rng, r);
        stackShoe(g, {Rank::Ace, Rank::Ten, Rank::Ace, Rank::Nine,
                      Rank::Ace, Rank::Five});                             // hand 0 becomes A,A again
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);
        CHECK(!g.canSplit());
    }
    {   // canResplitAces = true: the same ace pair CAN be split again
        SeededRandomGenerator rng(42);
        Rules r;
        r.maxHands = 3;
        r.oneCardOnSplitAces = false;
        r.canResplitAces = true;
        Game g(rng, r);
        stackShoe(g, {Rank::Ace, Rank::Ten, Rank::Ace, Rank::Nine,
                      Rank::Ace, Rank::Five});
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);
        CHECK(g.canSplit());
    }
    {   // canResplitAces only blocks ACES: other pairs can still be resplit
        SeededRandomGenerator rng(42);
        Rules r;
        r.maxHands = 3;
        r.canResplitAces = false;
        Game g(rng, r);
        stackShoe(g, {Rank::Eight, Rank::Ten, Rank::Eight, Rank::Seven,
                      Rank::Eight, Rank::Two});
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);
        CHECK(g.canSplit());
    }
}

void test_split_aces_one_card() {
    {   // default rules: split aces get one card each and the round ends by itself
        //   A,9 = 20 beats 19 (+4), A,5 = 16 loses: 200 - 2 - 2 + 4 = 200
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ace, Rank::Ten, Rank::Ace, Rank::Nine,         // P A,A  D 19
                      Rank::Nine, Rank::Five});                            // hand 0: A,9  hand 1: A,5
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);
        CHECK(g.player_hands.size() == 2);
        CHECK(g.player_hands[0].hand.cards.size() == 2);
        CHECK(g.player_hands[1].hand.cards.size() == 2);
        CHECK(g.current_phase == Phase::RoundOver);
        CHECK(g.bankroll == 200);
    }
    {   // after the automatic finish, Hit is no longer allowed and does nothing
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ace, Rank::Ten, Rank::Ace, Rank::Nine, Rank::Nine, Rank::Five});
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);
        CHECK(!g.isLegal(Event::Hit));
        g.handleEvent(Event::Hit);
        CHECK(g.player_hands[0].hand.cards.size() == 2);
    }
    {   // oneCardOnSplitAces = false: split aces are played like any other split
        SeededRandomGenerator rng(42);
        Rules r;
        r.oneCardOnSplitAces = false;
        Game g(rng, r);
        stackShoe(g, {Rank::Ace, Rank::Ten, Rank::Ace, Rank::Nine,
                      Rank::Five, Rank::Six, Rank::Two});                  // hand 0: A,5  hits a 2
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);
        CHECK(g.current_phase == Phase::PlayerTurn);
        CHECK(g.split_idx == 0);
        CHECK(g.isLegal(Event::Hit));
        g.handleEvent(Event::Hit);
        CHECK(g.player_hands[0].hand.cards.size() == 3);
    }
    {   // a non-ace split is NOT finished automatically
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Eight, Rank::Ten, Rank::Eight, Rank::Seven, Rank::Three, Rank::King});
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);
        CHECK(g.current_phase == Phase::PlayerTurn);
        CHECK(g.split_idx == 0);
    }
}

void test_skip_dealer_when_all_bust() {
    {   // single hand busts: dealer (16) does NOT draw, even though 16 would normally hit
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ten, Rank::Ten, Rank::Six, Rank::Six, Rank::King});
        g.handleEvent(Event::Deal);                   // P 16, D 16
        g.handleEvent(Event::Hit);                    // P 26
        CHECK(g.dealer.cards.size() == 2);
        CHECK(g.bankroll == 198);
        CHECK(g.current_phase == Phase::RoundOver);
    }
    {   // doubled hand busts: dealer does not draw either
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ten, Rank::Ten, Rank::Six, Rank::Six, Rank::King});
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Double);                 // P 26
        CHECK(g.dealer.cards.size() == 2);
        CHECK(g.bankroll == 196);
    }
    {   // split, BOTH hands bust: dealer does not draw
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Eight, Rank::Ten, Rank::Eight, Rank::Six,      // P 8,8  D 16
                      Rank::Eight, Rank::Six,                              // hand 0: 16, hand 1: 14
                      Rank::King, Rank::Queen});                           // both hit and bust
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);
        g.handleEvent(Event::Hit);                    // hand 0: 26 -> move to hand 1
        CHECK(g.split_idx == 1);
        g.handleEvent(Event::Hit);                    // hand 1: 24 -> round over
        CHECK(g.dealer.cards.size() == 2);
        CHECK(g.bankroll == 196);
        CHECK(g.current_phase == Phase::RoundOver);
    }
    {   // split, only ONE hand busts: the dealer still plays for the live hand
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Eight, Rank::Ten, Rank::Eight, Rank::Six,      // P 8,8  D 16
                      Rank::Eight, Rank::King,                             // hand 0: 16, hand 1: 18
                      Rank::King,                                          // hand 0 hits: 26
                      Rank::Five});                                        // dealer draws: 21
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);
        g.handleEvent(Event::Hit);                    // hand 0 busts
        g.handleEvent(Event::Stand);                  // hand 1 stands on 18
        CHECK(g.dealer.cards.size() == 3);
        CHECK(g.dealer.total() == 21);
        CHECK(g.bankroll == 196);                     // both hands lost
    }
}
