// esp32/esp32.ino: the blackjack gadget (ESP32 + 2.8" TFT screen, later the knob and buttons); uses the same core/ as the PC

#include <string>
#include <TFT_eSPI.h>    // the screen library
#include "game.h"        // from core/ (found through the blackjack_core link in ~/Arduino/libraries)
#include "rng.h"

// the game and the screen; rng must come before game, because game keeps a reference to it
SeededRandomGenerator rng(esp_random());   // seeded from the ESP32's hardware random number generator
Game game(rng);
TFT_eSPI tft;

// card drawing sizes, in pixels
const int CARD_W = 40;        // card width
const int CARD_H = 56;        // card height
const int CARD_RADIUS = 4;    // how rounded the corners are
const int CARD_PAD = 4;       // gap between a card's edge and the rank text
const int CARD_GAP = 10;      // space between neighbouring cards in a hand
const int CARD_INSET = 3;     // gap between a card back's edge and its decorative border

// "A", "2" ... "10", "J", "Q", "K" (the same as pc/main.cpp's rankText)
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

// draws one face-up card with its top-left corner at (x, y); red for hearts/diamonds, black for clubs/spades
void drawCard(int x, int y, const Card& card) {
  // the white card
  tft.fillRoundRect(x, y, CARD_W, CARD_H, CARD_RADIUS, TFT_WHITE);

  // pick the ink color from the suit
  bool isRed = (card.suit == Suit::Hearts || card.suit == Suit::Diamonds);
  uint16_t ink = isRed ? TFT_RED : TFT_BLACK;

  // write the rank near the top left corner of the card
  tft.setTextColor(ink, TFT_WHITE);            // ink color, on a white background
  tft.setTextSize(2);
  tft.setCursor(x + CARD_PAD, y + CARD_PAD);   // CARD_PAD pixels in from the card's corner
  tft.print(rankText(card.rank).c_str());
}

// draws every card in the hand in a row, left to right, starting with the first card's top-left at (x, y)
void drawHand(int x, int y, const Hand& hand) {
  for(size_t idx = 0; idx < hand.cards.size(); idx++) {
    int cardX = x + static_cast<int>(idx) * (CARD_W + CARD_GAP);
    drawCard(cardX, y, hand.cards[idx]);
  }
}

// draws a face-down card (the dealer's hidden card) with its top-left corner at (x, y)
void drawCardBack(int x, int y) {
  // the card itself: same size and corners as face-up card but a "back" color
  tft.fillRoundRect(x, y, CARD_W, CARD_H, CARD_RADIUS, TFT_NAVY);

  // a thin white border just inside the edge
  tft.drawRoundRect(x + CARD_INSET, y + CARD_INSET,
                    CARD_W - 2 * CARD_INSET, CARD_H - 2 * CARD_INSET,
                    CARD_RADIUS, TFT_WHITE);
}

// draws the whole screen for the current phase (read-only, like the terminal's drawScreen)
void drawTable(const Game& game) {
  tft.fillScreen(TFT_DARKGREEN);                // start from an empty table

  tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);   // all table text: white on green, size 2
  tft.setTextSize(2);

  // dealer along the top (nothing while betting, since no cards are on the table yet)
  if(game.current_phase == Phase::PlayerTurn) {
    // hole card hidden: no total, since it would give the hidden card away
    tft.setCursor(20, 0);
    tft.printf("DEALER");
    drawCard(20, 20, game.dealer.cards[0]);              // first card face up
    drawCardBack(20 + CARD_W + CARD_GAP, 20);            // second card face down
  } else if(game.current_phase != Phase::Betting) {
    // revealed: every card, plus the total
    tft.setCursor(20, 0);
    tft.printf("DEALER  %d", game.dealer.total());
    drawHand(20, 20, game.dealer);
  }

  // player's hand(s) along the bottom: side by side after a split
  // each hand is two short text lines (kept under ~12 characters so split hands fit in 150 px), then the cards
  const int totalY = 86;    // "TOTAL 19"
  const int betY   = 104;   // "BET $2"   (text size 2 is 16 px tall, so 18 px apart leaves a small gap)
  const int cardsY = 124;   // the cards, just under the bet line

  if(game.current_phase != Phase::Betting) {
    for(size_t idx = 0; idx < game.player_hands.size(); idx++) {
      const PlayerHand& player_hand = game.player_hands[idx];
      int handX = 20 + static_cast<int>(idx) * 150; // the second hand starts 150 px to the right

      tft.setCursor(handX, totalY);
      tft.printf("TOTAL %d", player_hand.hand.total());
      tft.setCursor(handX, betY);
      tft.printf("BET $%d", player_hand.bet);
      drawHand(handX, cardsY, player_hand.hand);

      // during player turn, underline the hand being played (useful for split)
      if(game.current_phase == Phase::PlayerTurn && idx == game.split_idx) {
        tft.fillRect(handX, cardsY + CARD_H + 4, 140, 3, TFT_YELLOW);
      }
    }
  }

  // money along the bottom: what you have, and what the next Deal will bet
  tft.setCursor(20, 200);
  tft.printf("$%d   bet $%d", game.bankroll, game.bet);

  // what to do next
  tft.setCursor(20, 220);
  switch(game.current_phase) {
    case Phase::Betting:    tft.print("Feeling lucky?");           break;
    case Phase::PlayerTurn: tft.print("Your move, high roller");   break;
    case Phase::RoundOver:  tft.print("One more round?");          break;
    case Phase::Broke:      tft.print("Bankrupt! Press to retry"); break;
  }
}

void setup() {
  Serial.begin(115200);
  tft.init();                      // also switches the backlight on (TFT_BL in TFT_eSPI's User_Setup.h)
  tft.setRotation(3);              // landscape, 320 x 240 (1 = the same, turned 180 degrees)

  game.handleEvent(Event::Deal);   // deal one round, as a test (until the buttons are wired)
  drawTable(game);                 // the whole table: dealer, player hand(s), money, message
}

void loop() {
  Serial.printf("alive: bankroll %d\n", game.bankroll);
  delay(1000);                     // once a second, so it doesn't flood the Serial Monitor
}
