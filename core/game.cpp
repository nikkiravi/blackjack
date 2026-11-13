#include "game.h"
#include "dealer.h"
#include "payout.h"

Game::Game(RandomGenerator& generator, const Rules& tableRules) : rules(tableRules), rng(generator) {
    shoe.shuffle(rng);
}

void Game::playDealer(){
    while(dealerShouldHit(dealer, rules)){
        dealer.addCard(shoe.dealCard());
    }
}

void Game::finishHand(){
    if(split_idx + 1 < player_hands.size()){
        split_idx += 1;
    } else {
        playDealer();
        settleRound();
        current_phase = Phase::RoundOver;
    }
}

void Game::settleRound(){
    bool isSplit = player_hands.size() > 1;
    for(const PlayerHand& ph : player_hands){
        Outcome result = handOutcome(ph.hand, dealer, isSplit);
        bankroll += payout(result, ph.bet, rules); 
    }
}

void Game::handleEvent(Event event) {
    switch(event) {
        case Event::BetUp:
            if(current_phase == Phase::Betting && bet + rules.betStep <= rules.maxBet) {
                bet += rules.betStep;
            }
            break;
        case Event::BetDown:
            if(current_phase == Phase::Betting && bet - rules.betStep >= rules.minBet) {
                bet -= rules.betStep;
            }
            break;
        case Event::Deal:
            if(current_phase == Phase::Betting && bankroll >= bet) {
                bankroll -= bet;

                // Reset the game state for a new round
                player_hands.clear();
                dealer.cards.clear();
                split_idx = 0;
                player_hands.push_back(PlayerHand{Hand(), bet, false});

                // Deal two cards to the player and two to the dealer
                player_hands[split_idx].hand.addCard(shoe.dealCard());
                dealer.addCard(shoe.dealCard());
                player_hands[split_idx].hand.addCard(shoe.dealCard());
                dealer.addCard(shoe.dealCard());
                
                if(player_hands[split_idx].hand.isBlackjack() || dealer.isBlackjack()) {
                    current_phase = Phase::RoundOver;
                    settleRound();
                } else {
                    current_phase = Phase::PlayerTurn; 
                }
            } 
            else if(current_phase == Phase::RoundOver) {
                if(shoe.needsReshuffle(rules.penetrationPercent)){
                    shoe.shuffle(rng);            
                }
                current_phase = Phase::Betting;
            }
            else if(current_phase == Phase::Broke) {
                bankroll = rules.startingBankroll;
                shoe.shuffle(rng);
                current_phase = Phase::Betting;
            }
            break;
        case Event::Hit:
            if(current_phase == Phase::PlayerTurn) {
                player_hands[split_idx].hand.addCard(shoe.dealCard());
                if(player_hands[split_idx].hand.isBust()) {
                    finishHand();
                } else {
                    current_phase = Phase::PlayerTurn; // Continue player's turn
                }
            }
            break;
        case Event::Stand:
            if(current_phase == Phase::PlayerTurn) {
                finishHand();
            }
            break;
        case Event::Double:
            if(current_phase == Phase::PlayerTurn && bankroll >= player_hands[split_idx].bet) {
                bankroll -= player_hands[split_idx].bet; // Deduct the additional bet amount
                player_hands[split_idx].bet *= 2; // Double the bet
                player_hands[split_idx].doubled = true; // Mark the hand as doubled
                player_hands[split_idx].hand.addCard(shoe.dealCard()); // Deal one more card to the hand
                finishHand();
            }
            break;
        case Event::Split:
            if(current_phase == Phase::PlayerTurn && 
                player_hands[split_idx].hand.cards.size() == 2 && 
                player_hands[split_idx].hand.cards[0].rank == player_hands[split_idx].hand.cards[1].rank &&
                player_hands.size() <static_cast<size_t>(rules.maxHands) &&
                bankroll >= player_hands[split_idx].bet) {
                
                // Deduct the additional bet amount for the new hand
                bankroll -= player_hands[split_idx].bet; 
                PlayerHand newHand{Hand(), player_hands[split_idx].bet, false};
                newHand.hand.addCard(player_hands[split_idx].hand.cards[1]); // Move the second card to the new hand
                
                player_hands[split_idx].hand.cards.pop_back(); // Remove the second card from the original hand
                player_hands[split_idx].hand.addCard(shoe.dealCard()); // Deal a new card to the original hand
                newHand.hand.addCard(shoe.dealCard()); // Deal a new card to the new hand
                player_hands.push_back(newHand); // Add the new hand to the list of player hands
                current_phase = Phase::PlayerTurn; // Continue with the player's turn
            }
            break;
    }
}
