// tests/test_main.cpp: runs every test function and prints a summary
#include "check.h"

// defined in test_card.cpp / test_hand.cpp; the linker connects them
void test_card_value();
void test_hand_total();

int main() {
    test_card_value();
    test_hand_total();

    if(g_failures == 0){
        std::printf("All tests passed!\n");
    } else {
        std::printf("%d test(s) failed.\n", g_failures);
    }
    
    return g_failures != 0;
}
