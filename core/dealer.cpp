#include "dealer.h"

bool dealerShouldHit(const Hand& hand, const Rules& rules) {
    int total = hand.total();
    if(total < 17) {
        return true;
    }

    if(total == 17 && hand.isSoft() && rules.dealerHitsSoft17) {
        return true;
    }

    return false;
}