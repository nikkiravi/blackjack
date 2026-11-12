// core/dealer.h: dealer's hand and rules for hitting/standing
#pragma once
#include "rules.h"
#include "hand.h"


// true if the dealer must draw: below 17, or soft 17 when rules.dealerHitsSoft17 is on
bool dealerShouldHit(const Hand& hand, const Rules& rules);