#include "payout.h"

Outcome handOutcome(const Hand& playerHand, const Hand& dealerHand, bool isSplit){
    bool naturalPlayerHand = (!isSplit) && playerHand.isBlackjack();
    bool blackjackPush = naturalPlayerHand && dealerHand.isBlackjack();
    
    if(blackjackPush){
        return Outcome::Push;
    }
    else if(naturalPlayerHand){
        return Outcome::Blackjack;
    }
    else if(dealerHand.isBlackjack()){
        return Outcome::Lose;
    }
    else if(playerHand.isBust()){
        return Outcome::Lose;
    }
    else if(dealerHand.isBust()){
        return Outcome::Win;
    }
    else if(playerHand.total() > dealerHand.total()){
        return Outcome::Win;
    }
    else if(playerHand.total() < dealerHand.total()){
        return Outcome::Lose;
    } else {
        return Outcome::Push;
    }
}

int payout(Outcome result, int bet, const Rules& tableRules){
    switch(result){
        case Outcome::Lose:
            return 0;
        case Outcome::Push:
            return bet;
        case Outcome::Win:
            return 2 * bet;
        case Outcome::Blackjack:
            return bet + bet * tableRules.blackjackPayNum / tableRules.blackjackPayDen;
    }
    return 0;
}