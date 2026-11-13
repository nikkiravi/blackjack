// tests/test_game.cpp: tests for the Game state machine (phases, betting, dealing, hit/stand/double/split)
#include "check.h"
#include "test_helpers.h"
#include "game.h"

void test_game_setup() {
    {   // a new game starts in Betting with the money from the default rules
        SeededRandomGenerator rng(42);
        Game g(rng);
        CHECK(g.current_phase == Phase::Betting);
        CHECK(g.bankroll == 200);
        CHECK(g.bet == 2);
        CHECK(g.shoe.cards.size() == 312);
        CHECK(g.player_hands.empty());
    }
    {   // custom rules reach the shoe, bankroll, and bet
        SeededRandomGenerator rng(42);
        Rules r;
        r.numDecks = 1;
        r.startingBankroll = 50;
        r.minBet = 4;
        Game g(rng, r);
        CHECK(g.shoe.cards.size() == 52);
        CHECK(g.bankroll == 50);
        CHECK(g.bet == 4);
    }
    {   // the constructor shuffles: the first card is no longer the build-order Ace of Hearts
        SeededRandomGenerator rng(42);
        Game g(rng);
        Card first = g.shoe.cards[0];
        CHECK(!(first.suit == Suit::Hearts && first.rank == Rank::Ace));
    }
}

void test_game_betting() {
    {   // BetUp / BetDown move by betStep
        SeededRandomGenerator rng(42);
        Game g(rng);
        g.handleEvent(Event::BetUp);
        g.handleEvent(Event::BetUp);
        CHECK(g.bet == 6);
        g.handleEvent(Event::BetDown);
        CHECK(g.bet == 4);
    }
    {   // BetDown never goes below minBet
        SeededRandomGenerator rng(42);
        Game g(rng);
        g.handleEvent(Event::BetDown);
        CHECK(g.bet == 2);
    }
    {   // BetUp never goes above maxBet
        SeededRandomGenerator rng(42);
        Rules r;
        r.maxBet = 6;
        Game g(rng, r);
        for (int i = 0; i < 10; i++) {
            g.handleEvent(Event::BetUp);
        }
        CHECK(g.bet == 6);
    }
    {   // betting buttons are ignored outside Betting
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ten, Rank::Ten, Rank::Six, Rank::Seven});
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::BetUp);
        CHECK(g.bet == 2);
    }
}

void test_game_deal() {
    {   // Deal takes the bet, gives two cards each in P, D, P, D order, and starts PlayerTurn
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Two, Rank::Three, Rank::Four, Rank::Five});
        g.handleEvent(Event::Deal);
        CHECK(g.bankroll == 198);
        CHECK(g.player_hands.size() == 1);
        CHECK(g.player_hands[0].bet == 2);
        CHECK(g.player_hands[0].hand.cards.size() == 2);
        CHECK(g.player_hands[0].hand.total() == 6);   // 2 + 4
        CHECK(g.dealer.cards.size() == 2);
        CHECK(g.dealer.total() == 8);                 // 3 + 5
        CHECK(g.current_phase == Phase::PlayerTurn);
    }
    {   // player blackjack skips PlayerTurn
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ace, Rank::Ten, Rank::King, Rank::Seven});
        g.handleEvent(Event::Deal);
        CHECK(g.player_hands[0].hand.isBlackjack());
        CHECK(g.current_phase == Phase::RoundOver);
    }
    {   // dealer blackjack also skips PlayerTurn
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ten, Rank::Ace, Rank::Seven, Rank::Queen});
        g.handleEvent(Event::Deal);
        CHECK(g.dealer.isBlackjack());
        CHECK(g.current_phase == Phase::RoundOver);
    }
    {   // Deal is refused when the bankroll can't cover the bet
        SeededRandomGenerator rng(42);
        Game g(rng);
        g.bankroll = 1;
        g.handleEvent(Event::Deal);
        CHECK(g.current_phase == Phase::Betting);
        CHECK(g.bankroll == 1);
        CHECK(g.player_hands.empty());
    }
    {   // a new round clears the previous round's cards and hand index
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ten, Rank::Nine, Rank::Seven, Rank::Eight,      // round 1: P 17, D 17
                      Rank::Two, Rank::Three, Rank::Four, Rank::Five});    // round 2
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Stand);                  // dealer has 17 and stands
        g.split_idx = 1;                              // pretend round 1 ended on a split hand
        g.handleEvent(Event::Deal);                   // RoundOver -> Betting
        g.handleEvent(Event::Deal);                   // Betting -> new round
        CHECK(g.player_hands.size() == 1);
        CHECK(g.player_hands[0].hand.cards.size() == 2);
        CHECK(g.dealer.cards.size() == 2);
        CHECK(g.split_idx == 0);
    }
}

void test_game_hit_stand() {
    {   // Hit adds one card and stays in PlayerTurn while not bust
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Two, Rank::Ten, Rank::Three, Rank::Seven, Rank::Four});
        g.handleEvent(Event::Deal);                   // P 2,3 = 5
        g.handleEvent(Event::Hit);                    // + 4 = 9
        CHECK(g.player_hands[0].hand.cards.size() == 3);
        CHECK(g.player_hands[0].hand.total() == 9);
        CHECK(g.current_phase == Phase::PlayerTurn);
    }
    {   // Hit that busts ends the round
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ten, Rank::Ten, Rank::Six, Rank::Seven, Rank::King});
        g.handleEvent(Event::Deal);                   // P 16
        g.handleEvent(Event::Hit);                    // + 10 = 26
        CHECK(g.player_hands[0].hand.isBust());
        CHECK(g.current_phase == Phase::RoundOver);
    }
    {   // Stand: dealer draws until 17 or more, then RoundOver
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ten, Rank::Ten, Rank::Nine, Rank::Two,          // P 19, D 12
                      Rank::Three, Rank::Four});                           // D draws 3 (15), 4 (19)
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Stand);
        CHECK(g.dealer.cards.size() == 4);
        CHECK(g.dealer.total() == 19);
        CHECK(g.current_phase == Phase::RoundOver);
    }
    {   // S17 (default): dealer stands on soft 17
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ten, Rank::Ace, Rank::Nine, Rank::Six, Rank::Five});
        g.handleEvent(Event::Deal);                   // D A,6 = soft 17
        g.handleEvent(Event::Stand);
        CHECK(g.dealer.cards.size() == 2);
    }
    {   // H17: dealer hits soft 17
        SeededRandomGenerator rng(42);
        Rules r;
        r.dealerHitsSoft17 = true;
        Game g(rng, r);
        stackShoe(g, {Rank::Ten, Rank::Ace, Rank::Nine, Rank::Six, Rank::Two});
        g.handleEvent(Event::Deal);                   // D A,6 = soft 17
        g.handleEvent(Event::Stand);                  // draws 2 -> soft 19
        CHECK(g.dealer.cards.size() == 3);
        CHECK(g.dealer.total() == 19);
    }
    {   // player buttons are ignored outside PlayerTurn
        SeededRandomGenerator rng(42);
        Game g(rng);
        g.handleEvent(Event::Hit);
        g.handleEvent(Event::Stand);
        g.handleEvent(Event::Double);
        g.handleEvent(Event::Split);
        CHECK(g.current_phase == Phase::Betting);
        CHECK(g.player_hands.empty());
        CHECK(g.bankroll == 200);
    }
}

void test_game_double() {
    {   // Double: second bet taken, bet doubled, exactly one card, round over
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Five, Rank::Ten, Rank::Six, Rank::Seven, Rank::Nine});
        g.handleEvent(Event::Deal);                   // P 11, bankroll 198
        g.handleEvent(Event::Double);                 // + 9 = 20, bankroll 196; dealer 17 -> win
        CHECK(g.bankroll == 196 + 8);                 // a doubled $4 win returns $8
        CHECK(g.player_hands[0].bet == 4);
        CHECK(g.player_hands[0].doubled);
        CHECK(g.player_hands[0].hand.cards.size() == 3);
        CHECK(g.player_hands[0].hand.total() == 20);
        CHECK(g.current_phase == Phase::RoundOver);
    }
    {   // Double is refused when the bankroll can't cover the second bet
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Five, Rank::Ten, Rank::Six, Rank::Seven});
        g.handleEvent(Event::Deal);
        g.bankroll = 1;
        g.handleEvent(Event::Double);
        CHECK(g.player_hands[0].bet == 2);
        CHECK(!g.player_hands[0].doubled);
        CHECK(g.current_phase == Phase::PlayerTurn);
    }
}

void test_game_split() {
    {   // Split a pair: two hands of two cards, second bet taken, still on hand 0
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Eight, Rank::Ten, Rank::Eight, Rank::Seven,     // P 8,8  D 17
                      Rank::Three, Rank::King});                           // hand 0 gets 3, hand 1 gets K
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);
        CHECK(g.player_hands.size() == 2);
        CHECK(g.bankroll == 196);
        CHECK(g.player_hands[1].bet == 2);
        CHECK(g.player_hands[0].hand.total() == 11);  // 8 + 3
        CHECK(g.player_hands[1].hand.total() == 18);  // 8 + K
        CHECK(g.split_idx == 0);
        CHECK(g.current_phase == Phase::PlayerTurn);
    }
    {   // Stand on hand 0 moves to hand 1; Stand on hand 1 ends the round
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Eight, Rank::Ten, Rank::Eight, Rank::Seven, Rank::Three, Rank::King});
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);
        g.handleEvent(Event::Stand);
        CHECK(g.split_idx == 1);
        CHECK(g.current_phase == Phase::PlayerTurn);
        CHECK(g.dealer.cards.size() == 2);            // dealer waits for the last hand
        g.handleEvent(Event::Stand);
        CHECK(g.current_phase == Phase::RoundOver);
    }
    {   // Double on hand 0 after a split moves on to hand 1 instead of ending the round
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Eight, Rank::Ten, Rank::Eight, Rank::Seven,
                      Rank::Three, Rank::King, Rank::Nine});               // hand 0: 8,3 + 9 = 20
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);
        g.handleEvent(Event::Double);
        CHECK(g.player_hands[0].hand.total() == 20);
        CHECK(g.split_idx == 1);
        CHECK(g.current_phase == Phase::PlayerTurn);
    }
    {   // a bust on hand 0 moves on to hand 1 instead of ending the round
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Eight, Rank::Ten, Rank::Eight, Rank::Seven,
                      Rank::Six, Rank::King, Rank::Queen});                // hand 0: 8,6 + Q = 24
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);
        g.handleEvent(Event::Hit);
        CHECK(g.player_hands[0].hand.isBust());
        CHECK(g.split_idx == 1);
        CHECK(g.current_phase == Phase::PlayerTurn);
    }
    {   // Split is refused for a non-pair (same value but different rank: K,Q)
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::King, Rank::Ten, Rank::Queen, Rank::Seven});
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);
        CHECK(g.player_hands.size() == 1);
        CHECK(g.bankroll == 198);
    }
    {   // Split is refused once maxHands (2) is reached, even on a new pair
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Eight, Rank::Ten, Rank::Eight, Rank::Seven,
                      Rank::Eight, Rank::Two});                            // hand 0 becomes 8,8 again
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);
        g.handleEvent(Event::Split);
        CHECK(g.player_hands.size() == 2);
        CHECK(g.bankroll == 196);
    }
    {   // Split is refused when the bankroll can't cover the second bet
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Eight, Rank::Ten, Rank::Eight, Rank::Seven});
        g.handleEvent(Event::Deal);
        g.bankroll = 1;
        g.handleEvent(Event::Split);
        CHECK(g.player_hands.size() == 1);
    }
}

void test_game_round_over_and_broke() {
    {   // Deal in RoundOver goes back to Betting
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ace, Rank::Ten, Rank::King, Rank::Seven});
        g.handleEvent(Event::Deal);                   // player blackjack -> RoundOver
        g.handleEvent(Event::Deal);
        CHECK(g.current_phase == Phase::Betting);
    }
    {   // leaving RoundOver reshuffles once the shoe is past penetration
        SeededRandomGenerator rng(42);
        Game g(rng);
        g.current_phase = Phase::RoundOver;
        g.shoe.next = 300;                            // 300 of 312 dealt, well past 75%
        g.handleEvent(Event::Deal);
        CHECK(g.shoe.remainingCards() == 312);
        CHECK(g.current_phase == Phase::Betting);
    }
    {   // ...but not before penetration
        SeededRandomGenerator rng(42);
        Game g(rng);
        g.current_phase = Phase::RoundOver;
        g.shoe.next = 10;
        g.handleEvent(Event::Deal);
        CHECK(g.shoe.remainingCards() == 302);
    }
    {   // Deal in Broke resets the bankroll, reshuffles, and goes back to Betting
        SeededRandomGenerator rng(42);
        Game g(rng);
        g.current_phase = Phase::Broke;
        g.bankroll = 0;
        g.shoe.next = 10;
        g.handleEvent(Event::Deal);
        CHECK(g.bankroll == 200);
        CHECK(g.shoe.remainingCards() == 312);
        CHECK(g.current_phase == Phase::Betting);
    }
}
