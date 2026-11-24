// esp32/esp32.ino: the blackjack gadget (ESP32 + 2.8" TFT screen, later the knob and buttons); uses the same core/ as the PC

#include <TFT_eSPI.h> // the screen library
#include "game.h"
#include "rng.h"
#include <string>

SeededRandomGenerator rng(esp_random());
Game game(rng);
TFT_eSPI tft;

// card drawing sizes, in pixels
const int CARD_W = 40;        // card width
const int CARD_H = 56;        // card height
const int CARD_RADIUS = 4;    // how rounded the corners are
const int CARD_PAD = 4;       // gap between a card's edge and the rank text
const int CARD_GAP = 10;      // space between neighbouring cards in a hand
const int CARD_INSET = 3;     // gap between a card back's edge and its decorative border

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
  tft.drawRoundRect(x + CARD_INSET, y + CARD_INSET, CARD_W - 2 * CARD_INSET, CARD_H - 2 * CARD_INSET, CARD_RADIUS, TFT_WHITE);
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_DARKGREEN);

  game.handleEvent(Event::Deal);                  // deal one round, as a test

  // player's hand along the bottom
  drawHand(20, 150, game.player_hands[0].hand);

  // dealer along the top: first card face up, second card hidden
  drawCard(20, 20, game.dealer.cards[0]);
  drawCardBack(70, 20); // hidden
}

void loop() {
  Serial.printf("alive: bankroll %d\n", game.bankroll);
  delay(1000);    // once a second, so it doesn't flood the screen
}
