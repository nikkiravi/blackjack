// core/game.h: the game itself, which is a state machine that runs rounds until the player quits

#pragma once
#include <cstdint>
#include "rules.h"
#include "shoe.h"
#include "hand.h"
#include "rng.h"
#include <vector>

enum class Phase : uint8_t { Betting, PlayerTurn, RoundOver, Broke };
enum class Event : uint8_t { BetUp, BetDown, Deal, Hit, Stand, Split, Double };

struct Game {
    Phase current_phase = Phase::Betting; // current phase of the game
    Rules rules = Rules(); // the table settings
    Shoe shoe = Shoe(rules.numDecks); // where cards are drawn from
    Hand dealer = Hand(); // dealer cards (no bet, so plain Hand)
    std::vector<PlayerHand> player_hands; // player hands (one or more, each with its own bet)
    size_t split_idx = 0; // index of the hand currently being played (0 = first hand, 1 = second hand if split)
    int bankroll = rules.startingBankroll; // player's current bankroll
    int bet = rules.minBet; // player's next wager

    RandomGenerator& rng; // random number generator (injected for testing)
    Game (RandomGenerator& generator, const Rules& tableRules = Rules()); 

    void handleEvent(Event event); // process a player action and advance the game state

private:
    void playDealer(); // dealer draws card
    void finishHand();
    void settleRound(); // payout depending when the round is over 
};