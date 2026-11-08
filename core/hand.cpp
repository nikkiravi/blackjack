#include "hand.h"

Hand::Hand() {
    cards.reserve(12);
}

void Hand::addCard(const Card& card) {
    cards.push_back(card);
}

bool Hand::isSoft() const {
    return hasAce() && hardTotal() + 10 <= 21;
}

bool Hand::hasAce() const {
    for (const Card &card : cards) {
        if(card.rank == Rank::Ace) {
            return true;
        }
    }
    return false;
}

bool Hand::isBust() const {
    return total() > 21;
}

bool Hand::isBlackjack() const {
    return cards.size() == 2 && total() == 21;
}

int Hand::hardTotal() const {
    int sum = 0;
    for (const Card &card : cards) {
        sum += card.value();
    }
    return sum;
}

int Hand::total() const {
    if(isSoft()) {
        return hardTotal() + 10;
    }

    return hardTotal();
}