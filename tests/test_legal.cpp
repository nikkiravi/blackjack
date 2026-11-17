// tests/test_legal.cpp: tests for which buttons are allowed (canDouble, isLegal)
#include "check.h"
#include "test_helpers.h"
#include "game.h"

void test_can_double() {
    {   // first two cards, can afford it -> allowed
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Five, Rank::Ten, Rank::Six, Rank::Seven});
        g.handleEvent(Event::Deal);                   // P 5,6 = 11
        CHECK(g.canDouble());
    }
    {   // after a hit (3 cards) -> not allowed, and pressing Double does nothing
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Two, Rank::Ten, Rank::Three, Rank::Seven, Rank::Four, Rank::Five});
        g.handleEvent(Event::Deal);                   // P 2,3
        g.handleEvent(Event::Hit);                    // P 2,3,4 = 9
        CHECK(!g.canDouble());
        g.handleEvent(Event::Double);
        CHECK(g.player_hands[0].bet == 2);
        CHECK(g.player_hands[0].hand.cards.size() == 3);
        CHECK(g.current_phase == Phase::PlayerTurn);
    }
    {   // outside PlayerTurn -> not allowed, and checking with no hands dealt doesn't crash
        SeededRandomGenerator rng(42);
        Game g(rng);
        CHECK(g.player_hands.empty());
        CHECK(!g.canDouble());
    }
    {   // can't afford the second bet -> not allowed
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Five, Rank::Ten, Rank::Six, Rank::Seven});
        g.handleEvent(Event::Deal);
        g.bankroll = 1;
        CHECK(!g.canDouble());
    }
    {   // canDoubleAnyTwo = false: only totals 9, 10, 11
        Rules r;
        r.canDoubleAnyTwo = false;
        struct Case { Rank a; Rank b; bool allowed; };
        const Case cases[] = {
            {Rank::Four,  Rank::Four,  false},       // 8
            {Rank::Four,  Rank::Five,  true},        // 9
            {Rank::Four,  Rank::Six,   true},        // 10
            {Rank::Five,  Rank::Six,   true},        // 11
            {Rank::Five,  Rank::Seven, false},       // 12
            {Rank::Ace,   Rank::Eight, false},       // soft 19
        };
        for (const Case& c : cases) {
            SeededRandomGenerator rng(42);
            Game g(rng, r);
            stackShoe(g, {c.a, Rank::Ten, c.b, Rank::Seven});
            g.handleEvent(Event::Deal);
            CHECK(g.canDouble() == c.allowed);
        }
    }
    {   // canDoubleAfterSplit = true (default): a split hand can double
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Eight, Rank::Ten, Rank::Eight, Rank::Seven, Rank::Three, Rank::King});
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);                  // hand 0: 8,3 = 11
        CHECK(g.canDouble());
    }
    {   // canDoubleAfterSplit = false: no doubling once split...
        SeededRandomGenerator rng(42);
        Rules r;
        r.canDoubleAfterSplit = false;
        Game g(rng, r);
        stackShoe(g, {Rank::Eight, Rank::Ten, Rank::Eight, Rank::Seven, Rank::Three, Rank::King});
        g.handleEvent(Event::Deal);
        CHECK(g.canDouble());                         // ...but before the split it's still fine
        g.handleEvent(Event::Split);
        CHECK(!g.canDouble());
        g.handleEvent(Event::Double);                 // ignored
        CHECK(g.player_hands[0].bet == 2);
        CHECK(g.split_idx == 0);
    }
}

void test_is_legal() {
    {   // Betting: only the bet buttons and Deal
        SeededRandomGenerator rng(42);
        Game g(rng);
        g.bet = 4;
        CHECK(g.isLegal(Event::BetUp));
        CHECK(g.isLegal(Event::BetDown));
        CHECK(g.isLegal(Event::Deal));
        CHECK(!g.isLegal(Event::Hit));
        CHECK(!g.isLegal(Event::Stand));
        CHECK(!g.isLegal(Event::Double));
        CHECK(!g.isLegal(Event::Split));
    }
    {   // Betting limits: BetDown off at minBet, BetUp off at the bankroll, Deal off if unaffordable
        SeededRandomGenerator rng(42);
        Game g(rng);
        CHECK(!g.isLegal(Event::BetDown));            // bet is already $2
        g.bankroll = 2;
        CHECK(!g.isLegal(Event::BetUp));              // $4 wouldn't fit
        g.bankroll = 1;
        CHECK(!g.isLegal(Event::Deal));
    }
    {   // BetUp off at maxBet
        SeededRandomGenerator rng(42);
        Rules r;
        r.maxBet = 4;
        Game g(rng, r);
        g.bet = 4;
        CHECK(!g.isLegal(Event::BetUp));
    }
    {   // PlayerTurn: Hit and Stand yes; betting and Deal no; Split only on a pair
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ten, Rank::Ten, Rank::Six, Rank::Seven});
        g.handleEvent(Event::Deal);                   // P 10,6 (not a pair)
        CHECK(g.isLegal(Event::Hit));
        CHECK(g.isLegal(Event::Stand));
        CHECK(g.isLegal(Event::Double));
        CHECK(!g.isLegal(Event::Split));
        CHECK(!g.isLegal(Event::BetUp));
        CHECK(!g.isLegal(Event::BetDown));
        CHECK(!g.isLegal(Event::Deal));
    }
    {   // PlayerTurn with a pair: Split allowed
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Eight, Rank::Ten, Rank::Eight, Rank::Seven});
        g.handleEvent(Event::Deal);
        CHECK(g.isLegal(Event::Split));
    }
    {   // RoundOver: only Deal
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ace, Rank::Ten, Rank::King, Rank::Seven});
        g.handleEvent(Event::Deal);                   // blackjack -> RoundOver
        CHECK(g.current_phase == Phase::RoundOver);
        CHECK(g.isLegal(Event::Deal));
        CHECK(!g.isLegal(Event::Hit));
        CHECK(!g.isLegal(Event::Stand));
        CHECK(!g.isLegal(Event::Double));
        CHECK(!g.isLegal(Event::Split));
        CHECK(!g.isLegal(Event::BetUp));
        CHECK(!g.isLegal(Event::BetDown));
    }
    {   // Broke: only Deal
        SeededRandomGenerator rng(42);
        Game g(rng);
        g.current_phase = Phase::Broke;
        g.bankroll = 0;
        CHECK(g.isLegal(Event::Deal));
        CHECK(!g.isLegal(Event::BetUp));
        CHECK(!g.isLegal(Event::Hit));
    }
    {   // the renderer's view: isLegal and canDouble work through a read-only const Game&
        SeededRandomGenerator rng(42);
        Game g(rng);
        const Game& view = g;                         // this line would fail to compile if they weren't const
        CHECK(view.isLegal(Event::Deal));
        CHECK(!view.canDouble());
    }
}

// every illegal button press must change nothing at all (isLegal and handleEvent agree)
static void checkIllegalPressesDoNothing(Game& g) {
    const Event all[] = {Event::BetUp, Event::BetDown, Event::Deal, Event::Hit,
                         Event::Stand, Event::Double, Event::Split};
    for (Event e : all) {
        if (g.isLegal(e)) {
            continue;
        }
        int bankroll = g.bankroll;
        int bet = g.bet;
        Phase phase = g.current_phase;
        size_t hands = g.player_hands.size();
        size_t dealerCards = g.dealer.cards.size();
        size_t shoeNext = g.shoe.next;
        g.handleEvent(e);
        CHECK(g.bankroll == bankroll);
        CHECK(g.bet == bet);
        CHECK(g.current_phase == phase);
        CHECK(g.player_hands.size() == hands);
        CHECK(g.dealer.cards.size() == dealerCards);
        CHECK(g.shoe.next == shoeNext);              // no card was dealt
    }
}

void test_illegal_presses_ignored() {
    {   // in Betting
        SeededRandomGenerator rng(42);
        Game g(rng);
        checkIllegalPressesDoNothing(g);
    }
    {   // in PlayerTurn
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ten, Rank::Ten, Rank::Six, Rank::Seven});
        g.handleEvent(Event::Deal);
        checkIllegalPressesDoNothing(g);
    }
    {   // in RoundOver
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ace, Rank::Ten, Rank::King, Rank::Seven});
        g.handleEvent(Event::Deal);
        checkIllegalPressesDoNothing(g);
    }
    {   // in Broke
        SeededRandomGenerator rng(42);
        Game g(rng);
        g.current_phase = Phase::Broke;
        g.bankroll = 0;
        checkIllegalPressesDoNothing(g);
    }
}
