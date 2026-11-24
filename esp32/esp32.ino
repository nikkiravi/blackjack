// esp32/esp32.ino: the blackjack gadget (ESP32 + 2.8" TFT screen, later the knob and buttons); uses the same core/ as the PC

#include <string>
#include <TFT_eSPI.h>    
#include "game.h"        
#include "rng.h"
#include "payout.h"

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

const unsigned long REVEAL_PAUSE_MS = 2000;   // how long the final cards show before the result banner appears

// one push button (INPUT_PULLUP: HIGH = not pressed, LOW = pressed), plus what debouncing needs to remember
struct Button {
  int pin;                          // which GPIO it's wired to
  int stableState = HIGH;           // the trusted reading, only changes once the pin has settled
  int lastRaw = HIGH;               // the latest digitalRead, bounces and all
  unsigned long lastChangeMs = 0;   // millis() when lastRaw last changed (0 = nothing has changed yet)
};

// the play buttons, plus the knob's press (only the pin is given; the other fields start at their defaults)
Button hitButton{26};      // green  (D26, small board row 13)
Button standButton{25};    // red    (D25, small board row 12)
Button doubleButton{33};   // yellow (D33, small board row 11)
Button splitButton{32};    // blue   (D32, small board row 10)
Button knobButton{13};     // pressing the knob = Deal (D13, small board row 17)

// the knob's turning pins (KY-040 rotary encoder)
const int KNOB_CLK = 27;   // D27, small board row 14 (rotation signal A)
const int KNOB_DT  = 14;   // D14, small board row 15 (rotation signal B)

const unsigned long DEBOUNCE_MS = 20;   // a change only counts once the pin has stayed put this long (bounces last a few ms)

// reads a button and debounces it; call it on every loop() pass
// returns true only when stableState really changed (pressed or released), never for a bounce
bool updateButton(Button& button){
  int raw = digitalRead(button.pin);
  unsigned long now = millis();

  if(raw != button.lastRaw) { // the pin moved
    button.lastRaw = raw; 
    button.lastChangeMs = now; // restart the "how long has it been steady" clock
    return false;
  }

  if(now - button.lastChangeMs >= DEBOUNCE_MS && raw != button.stableState) {
    button.stableState = raw; // steady long enough: it's a real change
    return true;
  }

  return false;
}

// remembers CLK from the previous loop() pass, to spot the moment it falls
int lastClk = HIGH;

// true once per click of the knob; sets out to BetUp (clockwise) or BetDown (counter-clockwise)
// rule (from the knob test): at the moment CLK falls, DT is still HIGH when turning clockwise, already LOW counter-clockwise
bool knobTurned(Event& out) {
  int clk = digitalRead(KNOB_CLK);
  bool turned = false;
  if(lastClk == HIGH && clk == LOW) {                                        // CLK just fell
    out = (digitalRead(KNOB_DT) == HIGH) ? Event::BetUp : Event::BetDown;
    turned = true;
  }
  lastClk = clk;            // remember for next time, whether or not it fell
  return turned;
}


// "A", "2" ... "10", "J", "Q", "K" 
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

// a big banner across the middle of the screen with the round's overall result (drawn last, so it's on top)
void drawResultBanner(const Game& game) {
  // add up the whole round: what was bet, and what came back
  bool isSplit = game.player_hands.size() > 1;
  bool anyBlackjack = false;
  int totalBet = 0;
  int totalBack = 0;
  for(const PlayerHand& player_hand : game.player_hands) {
    Outcome outcome = handOutcome(player_hand.hand, game.dealer, isSplit);
    if(outcome == Outcome::Blackjack) {
      anyBlackjack = true;
    }
    totalBet  += player_hand.bet;
    totalBack += payout(outcome, player_hand.bet, game.rules);   // money returned, bet included
  }

  // pick the words and color (PUSH unless the money says otherwise)
  const char* text  = "PUSH";
  uint16_t    color = TFT_WHITE;
  if(anyBlackjack) {
    text  = "BLACKJACK!";
    color = TFT_YELLOW;
  } else if(totalBack > totalBet) {
    text  = "YOU WIN!";
    color = TFT_YELLOW;
  } else if(totalBack < totalBet) {
    text  = "YOU LOSE";
    color = TFT_RED;
  }

  // the box: dark, with a colored border, in the middle of the screen
  tft.fillRoundRect(30, 80, 260, 80, 10, TFT_BLACK);
  tft.drawRoundRect(30, 80, 260, 80, 10, color);

  // the words, big and centered in the box
  tft.setTextSize(4);
  tft.setTextColor(color, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(text, 160, 120);
  tft.setTextDatum(TL_DATUM);          // put things back the way drawTable expects
  tft.setTextSize(2);
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

  // player's hand(s) along the bottom
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

  // the round's result, big and on top of everything (so it must be drawn last)
  // the first time a round ends, pause so the player can see the dealer's final cards before the banner covers them;
  // later redraws of the same RoundOver screen (e.g. a button pressed that does nothing) show the banner straight away
  static Phase lastDrawnPhase = Phase::Betting;   // static: remembered between calls to drawTable
  if(game.current_phase == Phase::RoundOver) {
    if(lastDrawnPhase != Phase::RoundOver) {
      delay(REVEAL_PAUSE_MS);
    }
    drawResultBanner(game);
  }
  lastDrawnPhase = game.current_phase;
}

// turns a typed key into a game event; false if the key isn't one of ours
// (Serial Monitor stand-in for the knob and buttons, until they're wired)
bool keyToEvent(char key, Event& out) {
  switch(key) {
    case '+': out = Event::BetUp;   return true;
    case '-': out = Event::BetDown; return true;
    case 'n': out = Event::Deal;    return true;   // "next": deal / continue (Enter can't be used here)
    case 'h': out = Event::Hit;     return true;
    case 's': out = Event::Stand;   return true;
    case 'd': out = Event::Double;  return true;
    case 'p': out = Event::Split;   return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  pinMode(hitButton.pin, INPUT_PULLUP);    // buttons read HIGH normally, LOW while pressed
  pinMode(standButton.pin, INPUT_PULLUP);
  pinMode(doubleButton.pin, INPUT_PULLUP);
  pinMode(splitButton.pin, INPUT_PULLUP);
  pinMode(KNOB_CLK, INPUT_PULLUP);   // the KY-040 has its own pull-ups on CLK/DT, but these don't hurt
  pinMode(KNOB_DT,  INPUT_PULLUP);
  pinMode(knobButton.pin,  INPUT_PULLUP);   // the press switch often has no pull-up on the module, so this one matters
  Serial.println("Blackjack gadget started: press a button");   // proves the board is running, even before any press
  tft.init();                      // also switches the backlight on (TFT_BL in TFT_eSPI's User_Setup.h)
  tft.setRotation(1);              // landscape, 320 x 240 (3 = the same, turned 180 degrees)

  drawTable(game);                 // the whole table: dealer, player hand(s), money, message
}

// runs forever: turn button presses (and typed keys) into game events, then redraw
void loop() {
  // the knob: each click turns into BetUp (clockwise) or BetDown (counter-clockwise)
  Event turn;
  if(knobTurned(turn)) {
    game.handleEvent(turn);
    drawTable(game);
  }

  // pressing the knob = Deal (also "next round" after the result, and "start over" when broke)
  if(updateButton(knobButton) && knobButton.stableState == LOW) {
    game.handleEvent(Event::Deal);
    drawTable(game);
  }

  // every button is checked on every pass (separate ifs, not else-if); only the press edge (going LOW) is an event,
  // so holding a button or letting go does nothing. updateButton comes first so it runs on every pass (&& skips the rest)
  if(updateButton(hitButton) && hitButton.stableState == LOW) {
    game.handleEvent(Event::Hit);
    drawTable(game);
  }
  if(updateButton(standButton) && standButton.stableState == LOW) {
    game.handleEvent(Event::Stand);
    drawTable(game);
  }
  if(updateButton(doubleButton) && doubleButton.stableState == LOW) {
    game.handleEvent(Event::Double);
    drawTable(game);
  }
  if(updateButton(splitButton) && splitButton.stableState == LOW) {
    game.handleEvent(Event::Split);
    drawTable(game);
  }

  if(Serial.available() > 0) {                     // has a key been typed?
    char key = static_cast<char>(Serial.read());   // read() returns an int (-1 = nothing), so cast to char
    if(key == '\r' || key == '\n') {
      return;                                      // the Serial Monitor's newline: ignore it
    }

    Event event;
    if(keyToEvent(key, event)) {
      game.handleEvent(event);
      drawTable(game);                             // redraw only when something happened
    }
  }
}
