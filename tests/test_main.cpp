// tests/test_main.cpp
#include "check.h"
#include "card.h"

void test_card_value() {
    Card card1{Suit::Hearts, Rank::Ace};
    CHECK(card1.value() == 1);

    Card card2{Suit::Diamonds, Rank::Ten};
    CHECK(card2.value() == 10);

    Card card3{Suit::Clubs, Rank::Queen};
    CHECK(card3.value() == 10);
}

int main() {
    test_card_value();
    
    if(g_failures == 0){
        std::printf("All tests passed!\n");
    } else {
        std::printf("%d test(s) failed.\n", g_failures);
    }
    
    return g_failures != 0;
}