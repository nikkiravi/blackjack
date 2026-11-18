// pc/main.cpp: play blackjack in the terminal (draws the table as text, turns keys into game events)

#include <string>
#include <cstdio>
#include <cstdint>
#include <ctime>
#include <iostream>
#include "card.h"
#include "hand.h"
#include "game.h"
#include "rng.h"
#include "file_storage.h"

struct KeyHelp { 
    char key; // what the player types
    Event event; 
    const char* label;
};
// every key the game understands (shared by drawScreen and keyToEvent)
const KeyHelp help[] = {
    {'+', Event::BetUp, "[+] bet up  "},
    {'-', Event::BetDown, "[-] bet down  "},
    {'\n', Event::Deal, "[Enter] deal  "},
    {'h', Event::Hit, "[h] hit  "},
    {'s', Event::Stand, "[s] stand  "},
    {'d', Event::Double, "[d] double  "},
    {'p', Event::Split, "[p] split  "},
};

static std::string rankText(Rank rank){
    switch(rank){
        case Rank::Ace: return "A";
        case Rank::Two: return "2";
        case Rank::Three: return "3";
        case Rank::Four: return "4";
        case Rank::Five: return "5";
        case Rank::Six: return "6";
        case Rank::Seven: return "7";
        case Rank::Eight: return "8";
        case Rank::Nine: return "9";
        case Rank::Ten: return "10";
        case Rank::Jack: return "J";
        case Rank::Queen: return "Q";
        case Rank::King: return "K";
    }
    return "?";
}

static std::string suitText(Suit suit){
    switch(suit){
        case Suit::Hearts: return "♥";
        case Suit::Diamonds: return "♦";
        case Suit::Clubs: return "♣";
        case Suit::Spades: return "♠";
    }
    return "?";
}

static std::string cardText(const Card& card){
    return rankText(card.rank) + suitText(card.suit);
}

static std::string handText(const Hand& hand){
    std::string out; 
    for(const Card& card : hand.cards){
        out += cardText(card) + " ";
    }
    return out;
}

static bool keyToEvent(char key, Event& out){
    for(const KeyHelp& kh : help){
        if(key == kh.key){
            out = kh.event;
            return true;
        }
    }
    return false;
}

static void drawScreen(const Game& game){
    // drawing top-down so dealer is handled first

    std::printf("\033[2J\033[H");

    // while betting, the table is empty (last round's cards are cleared from view)
    if(game.current_phase != Phase::Betting){
        if(game.dealer.cards.empty()){
            std::printf("Dealer:\n");
        }
        else if(game.current_phase == Phase::PlayerTurn){
            std::string first = cardText(game.dealer.cards[0]);
            std::printf("Dealer: %s ??\n", first.c_str());
        } else {
            std::string text = handText(game.dealer);
            std::printf("Dealer: %s(%d)\n", text.c_str(), game.dealer.total());
        }

        for(size_t idx = 0; idx < game.player_hands.size(); idx++){
            const PlayerHand& playerHand = game.player_hands[idx];
            std::string marker = (game.current_phase == Phase::PlayerTurn && idx == game.split_idx) ? " <-" : "";
            std::printf("Hand %zu: %s(%d) bet $%d%s\n", idx + 1, handText(playerHand.hand).c_str(), playerHand.hand.total(), playerHand.bet, marker.c_str());
        }
    }

    std::printf("Bankroll: $%d        Bet: $%d\n", game.bankroll, game.bet);

    // a short message for each phase
    switch(game.current_phase){
        case Phase::Betting: 
            std::printf("Place your bet.\n");
            break;
        case Phase::PlayerTurn: 
            std::printf("Your move.\n");
            break;
        case Phase::RoundOver: 
            std::printf("Round over.\n");
            break;
        case Phase::Broke: 
            std::printf("Out of money! Press Enter to start over.\n");
            break;
    }

    // only show the keys that are allowed right now
    std::string keys;
    for(const KeyHelp& key : help){
        if(game.isLegal(key.event)){
            keys += key.label;
        }
    }
    std::printf("%s[q] quit\n", keys.c_str());
}

int main(){
    SeededRandomGenerator rng(static_cast<uint32_t>(std::time(nullptr))); // a different shuffle every run
    Game game(rng);
    FileStorage storage("build/bankroll.txt");

    game.bankroll = storage.loadBankroll(game.rules.startingBankroll);
    if(game.bankroll < game.rules.minBet){
        game.current_phase = Phase::Broke;
    }
    int lastSaved = game.bankroll;
    std::string message;
    while(true){
        drawScreen(game);
        if(!message.empty()){
            printf("%s\n", message.c_str());
            message.clear();
        } 

        std::printf("> ");
        std::fflush(stdout);
        std::string line;
        if(!std::getline(std::cin, line)){
            break; // Ctrl+D closes the input
        }
        char key = line.empty() ? '\n' : line[0]; // an empty line means Enter
        if(key == 'q'){
            break;
        } 

        Event event;
        if(!keyToEvent(key, event)){
            message = "Unknown key.";
        } else {
            game.handleEvent(event);
        }

        // only write when the money changed
        if(game.bankroll != lastSaved){
            storage.saveBankroll(game.bankroll);
            lastSaved = game.bankroll;
        } 
    }
    return 0;
}
