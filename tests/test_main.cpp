// tests/test_main.cpp: runs every test function and prints a summary
#include "check.h"

// defined in the other test_*.cpp files; the linker connects them
void test_card_value();
void test_hand_total();
void test_fake_rng();
void test_seeded_rng();
void test_shoe();
void test_shoe_penetration();
void test_dealer();
void test_game_setup();
void test_game_betting();
void test_game_deal();
void test_game_hit_stand();
void test_game_double();
void test_game_split();
void test_game_round_over_and_broke();

int main() {
    test_card_value();
    test_hand_total();
    test_fake_rng();
    test_seeded_rng();
    test_shoe();
    test_shoe_penetration();
    test_dealer();
    test_game_setup();
    test_game_betting();
    test_game_deal();
    test_game_hit_stand();
    test_game_double();
    test_game_split();
    test_game_round_over_and_broke();

    if(g_failures == 0){
        std::printf("All tests passed!\n");
    } else {
        std::printf("%d test(s) failed.\n", g_failures);
    }
    
    return g_failures != 0;
}
