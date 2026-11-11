// tests/test_main.cpp: runs every test function and prints a summary
#include "check.h"

// defined in the other test_*.cpp files; the linker connects them
void test_card_value();
void test_hand_total();
void test_fake_rng();
void test_seeded_rng();
void test_shoe();

int main() {
    test_card_value();
    test_hand_total();
    test_fake_rng();
    test_seeded_rng();
    test_shoe();

    if(g_failures == 0){
        std::printf("All tests passed!\n");
    } else {
        std::printf("%d test(s) failed.\n", g_failures);
    }
    
    return g_failures != 0;
}
