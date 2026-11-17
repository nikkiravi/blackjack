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
    }
}

void Game::settleRound(){
    bool isSplit = player_hands.size() > 1;
    for(const PlayerHand& ph : player_hands){
        Outcome result = handOutcome(ph.hand, dealer, isSplit);
        bankroll += payout(result, ph.bet, rules); 
    }

    // can't afford the minimum bet anymore -> Broke screen; otherwise show the results
    if(bankroll < rules.minBet) {
        current_phase = Phase::Broke;
    } else {
        current_phase = Phase::RoundOver;
    }
}

void Game::clampBet(){
    if(bet > bankroll){
        bet = (bankroll / rules.betStep) * rules.betStep; // round down to a whole number of betStep
    }
}

bool Game::canDouble() const{
    if(current_phase != Phase::PlayerTurn){
        return false;
    }
    
    const PlayerHand& current_player_hand = player_hands[split_idx];
    if(current_player_hand.hand.cards.size() != 2){
        return false; // first two cards only
    }
    if(bankroll < current_player_hand.bet){
        return false; // must afford the second bet
    }
    if(!rules.canDoubleAfterSplit && player_hands.size() > 1){
        return false; // no doubling after a split
    }

    int total = current_player_hand.hand.total();
    if(!rules.canDoubleAnyTwo && (total < 9 || total > 11)){
        return false; //9-11 cards only
    }

    return true;
}

bool Game::isLegal(Event event) const{
    switch(event){
        case Event::BetUp:
            return current_phase == Phase::Betting && 
                bet + rules.betStep <= rules.maxBet && 
                bet + rules.betStep <= bankroll;
        case Event::BetDown:
            return current_phase == Phase::Betting && bet - rules.betStep >= rules.minBet;
        case Event::Deal:
            return (current_phase == Phase::Betting && bankroll >= bet) ||
                current_phase == Phase::RoundOver ||
                current_phase == Phase::Broke;
        case Event::Hit:
            return current_phase == Phase::PlayerTurn;
        case Event::Stand:
            return current_phase == Phase::PlayerTurn;
        case Event::Double:
            return canDouble();
        case Event::Split:
            return current_phase == Phase::PlayerTurn && 
                player_hands[split_idx].hand.cards.size() == 2 && 
                player_hands[split_idx].hand.cards[0].rank == player_hands[split_idx].hand.cards[1].rank &&
                player_hands.size() <static_cast<size_t>(rules.maxHands) &&
                bankroll >= player_hands[split_idx].bet;
    }
    return false;
}

void Game::handleEvent(Event event) {
    if(!isLegal(event)){ return; }
    switch(event) {
        case Event::BetUp:
            bet += rules.betStep;
            break;
        case Event::BetDown:
            bet -= rules.betStep;
            break;
        case Event::Deal:
            if(current_phase == Phase::Betting) {
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
                    settleRound();
                } else {
                    current_phase = Phase::PlayerTurn; 
                }
            } 
            else if(current_phase == Phase::RoundOver) {
                if(shoe.needsReshuffle(rules.penetrationPercent)){
                    shoe.shuffle(rng);            
                }
                clampBet();
                current_phase = Phase::Betting;
            }
            else if(current_phase == Phase::Broke) {
                bankroll = rules.startingBankroll;
                shoe.shuffle(rng);
                clampBet();
                current_phase = Phase::Betting;
            }
            break;
        case Event::Hit:
            player_hands[split_idx].hand.addCard(shoe.dealCard());
            if(player_hands[split_idx].hand.isBust()) {
                finishHand();
            } else {
                current_phase = Phase::PlayerTurn; // Continue player's turn
            }
            break;
        case Event::Stand:
            finishHand();
            break;
        case Event::Double:
            bankroll -= player_hands[split_idx].bet; // Deduct the additional bet amount
            player_hands[split_idx].bet *= 2; // Double the bet
            player_hands[split_idx].doubled = true; // Mark the hand as doubled
            player_hands[split_idx].hand.addCard(shoe.dealCard()); // Deal one more card to the hand
            finishHand();
            break;
        case Event::Split: {
            // Deduct the additional bet amount for the new hand
            bankroll -= player_hands[split_idx].bet; 
            PlayerHand newHand{Hand(), player_hands[split_idx].bet, false};
            newHand.hand.addCard(player_hands[split_idx].hand.cards[1]); // Move the second card to the new hand
            
            player_hands[split_idx].hand.cards.pop_back(); // Remove the second card from the original hand
            player_hands[split_idx].hand.addCard(shoe.dealCard()); // Deal a new card to the original hand
            newHand.hand.addCard(shoe.dealCard()); // Deal a new card to the new hand
            player_hands.push_back(newHand); // Add the new hand to the list of player hands
            current_phase = Phase::PlayerTurn; // Continue with the player's turn
            break;
        }
    }
}
