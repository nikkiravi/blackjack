// tests/test_game_money.cpp: tests for money inside a full round (payouts, Broke, bet limits)
// every game starts with the default $200 bankroll and $2 bet unless the test changes the rules
#include "check.h"
#include "test_helpers.h"
#include "game.h"

void test_game_payouts() {
    {   // win: 19 vs dealer 18 -> 198 + 4
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ten, Rank::Ten, Rank::Nine, Rank::Eight});
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Stand);
        CHECK(g.bankroll == 202);
        CHECK(g.current_phase == Phase::RoundOver);
    }
    {   // loss: 17 vs dealer 19 -> stays at 198
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ten, Rank::Ten, Rank::Seven, Rank::Nine});
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Stand);
        CHECK(g.bankroll == 198);
    }
    {   // push: 18 vs 18 -> bet comes back, 200
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ten, Rank::Ten, Rank::Eight, Rank::Eight});
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Stand);
        CHECK(g.bankroll == 200);
    }
    {   // player bust loses the bet
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ten, Rank::Ten, Rank::Six, Rank::Seven, Rank::King});
        g.handleEvent(Event::Deal);                   // P 16, D 17
        g.handleEvent(Event::Hit);                    // + K = 26
        CHECK(g.bankroll == 198);
        CHECK(g.current_phase == Phase::RoundOver);
    }
    {   // dealer bust pays a low hand: 12 vs dealer 16 + K = 26
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ten, Rank::Ten, Rank::Two, Rank::Six, Rank::King});
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Stand);
        CHECK(g.dealer.isBust());
        CHECK(g.bankroll == 202);
    }
    {   // doubled loss costs both bets: 5,6 + 2 = 13 vs 19 -> 200 - 2 - 2
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Five, Rank::Ten, Rank::Six, Rank::Nine, Rank::Two});
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Double);
        CHECK(g.bankroll == 196);
    }
}

void test_game_blackjack_payouts() {
    {   // player blackjack on the deal pays 3:2 right away: 198 + 5 = 203 (odd!)
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ace, Rank::Ten, Rank::King, Rank::Seven});
        g.handleEvent(Event::Deal);
        CHECK(g.bankroll == 203);
        CHECK(g.current_phase == Phase::RoundOver);
    }
    {   // dealer blackjack on the deal takes the bet
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ten, Rank::Ace, Rank::Nine, Rank::King});
        g.handleEvent(Event::Deal);
        CHECK(g.bankroll == 198);
        CHECK(g.current_phase == Phase::RoundOver);
    }
    {   // both blackjack -> push, bet comes back
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ace, Rank::Ace, Rank::King, Rank::Queen});
        g.handleEvent(Event::Deal);
        CHECK(g.bankroll == 200);
    }
}

void test_game_split_payouts() {
    {   // split 8,8: hand 0 wins (18 vs 17), hand 1 loses (16 vs 17)
        //   200 - 2 (deal) - 2 (split) + 4 (win) + 0 (loss) = 200
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Eight, Rank::Ten, Rank::Eight, Rank::Seven,     // P 8,8  D 17
                      Rank::King, Rank::Two,                               // hand 0: 18, hand 1: 10
                      Rank::Six});                                         // hand 1 hits: 16
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);
        g.handleEvent(Event::Stand);                  // hand 0 done
        g.handleEvent(Event::Hit);                    // hand 1: 16
        g.handleEvent(Event::Stand);                  // hand 1 done -> dealer stands on 17 -> settle
        CHECK(g.bankroll == 200);
        CHECK(g.current_phase == Phase::RoundOver);
    }
    {   // split aces: A + K is 21 but NOT a blackjack, so it pays 1:1 (+4), not 3:2 (+5)
        //   200 - 2 - 2 + 4 (21 beats 19) + 0 (18 loses to 19) = 200
        SeededRandomGenerator rng(42);
        Game g(rng);
        stackShoe(g, {Rank::Ace, Rank::Ten, Rank::Ace, Rank::Nine,         // P A,A  D 19
                      Rank::King, Rank::Seven});                           // hand 0: A,K  hand 1: A,7
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Split);
        g.handleEvent(Event::Stand);
        g.handleEvent(Event::Stand);
        CHECK(g.bankroll == 200);
    }
}

void test_game_broke() {
    {   // losing the last $2 -> Broke instead of RoundOver
        SeededRandomGenerator rng(42);
        Rules r;
        r.startingBankroll = 2;
        Game g(rng, r);
        stackShoe(g, {Rank::Ten, Rank::Ten, Rank::Seven, Rank::Nine});
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Stand);
        CHECK(g.bankroll == 0);
        CHECK(g.current_phase == Phase::Broke);
    }
    {   // Broke means "below minBet", not just zero: $1 left is still broke
        SeededRandomGenerator rng(42);
        Rules r;
        r.startingBankroll = 3;
        Game g(rng, r);
        stackShoe(g, {Rank::Ten, Rank::Ten, Rank::Seven, Rank::Nine});
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Stand);
        CHECK(g.bankroll == 1);
        CHECK(g.current_phase == Phase::Broke);
    }
    {   // exactly minBet left is NOT broke
        SeededRandomGenerator rng(42);
        Rules r;
        r.startingBankroll = 4;
        Game g(rng, r);
        stackShoe(g, {Rank::Ten, Rank::Ten, Rank::Seven, Rank::Nine});
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Stand);
        CHECK(g.bankroll == 2);
        CHECK(g.current_phase == Phase::RoundOver);
    }
    {   // Deal on the Broke screen resets to the starting bankroll and allows a new round
        SeededRandomGenerator rng(42);
        Rules r;
        r.startingBankroll = 2;
        Game g(rng, r);
        stackShoe(g, {Rank::Ten, Rank::Ten, Rank::Seven, Rank::Nine});
        g.handleEvent(Event::Deal);
        g.handleEvent(Event::Stand);                  // -> Broke
        g.handleEvent(Event::Deal);                   // reset
        CHECK(g.bankroll == 2);
        CHECK(g.current_phase == Phase::Betting);
        g.handleEvent(Event::Deal);                   // can play again
        CHECK(g.bankroll == 0);
        CHECK(g.player_hands.size() == 1);
    }
}

void test_game_bet_limits() {
    {   // BetUp stops at the bankroll
        SeededRandomGenerator rng(42);
        Game g(rng);
        g.bankroll = 6;
        g.bet = 6;
        g.handleEvent(Event::BetUp);
        CHECK(g.bet == 6);
    }
    {   // BetUp may go exactly to the bankroll (all in)
        SeededRandomGenerator rng(42);
        Game g(rng);
        g.bankroll = 8;
        g.bet = 6;
        g.handleEvent(Event::BetUp);
        CHECK(g.bet == 8);
    }
    {   // leaving RoundOver leaves an affordable bet alone
        SeededRandomGenerator rng(42);
        Game g(rng);
        g.current_phase = Phase::RoundOver;
        g.bankroll = 50;
        g.bet = 4;
        g.handleEvent(Event::Deal);
        CHECK(g.bet == 4);
    }
    {   // leaving RoundOver lowers a too-big bet to the bankroll (even bankroll)
        SeededRandomGenerator rng(42);
        Game g(rng);
        g.current_phase = Phase::RoundOver;
        g.bankroll = 6;
        g.bet = 10;
        g.handleEvent(Event::Deal);
        CHECK(g.bet == 6);
        CHECK(g.current_phase == Phase::Betting);
    }
    {   // odd bankroll: rounds DOWN to a whole $2 step (7 -> 6), never an odd bet
        SeededRandomGenerator rng(42);
        Game g(rng);
        g.current_phase = Phase::RoundOver;
        g.bankroll = 7;
        g.bet = 10;
        g.handleEvent(Event::Deal);
        CHECK(g.bet == 6);
    }
    {   // leaving Broke also lowers the bet to fit the fresh bankroll
        SeededRandomGenerator rng(42);
        Game g(rng);
        g.current_phase = Phase::Broke;
        g.bankroll = 0;
        g.bet = 1000;
        g.handleEvent(Event::Deal);
        CHECK(g.bankroll == 200);
        CHECK(g.bet == 200);
    }
    {   // after the clamp, Deal actually works again
        SeededRandomGenerator rng(42);
        Game g(rng);
        g.current_phase = Phase::RoundOver;
        g.bankroll = 7;
        g.bet = 10;
        g.handleEvent(Event::Deal);                   // RoundOver -> Betting, bet 6
        g.handleEvent(Event::Deal);                   // new round
        CHECK(g.bankroll == 1);
        CHECK(g.player_hands.size() == 1);
    }
}
