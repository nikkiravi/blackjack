// esp32/esp32.ino: the blackjack gadget (ESP32 + 2.8" TFT screen, later the knob and buttons); uses the same core/ as the PC

#include <TFT_eSPI.h> // the screen library
#include "game.h"
#include "rng.h"

SeededRandomGenerator rng(esp_random());
Game game(rng);
TFT_eSPI tft;

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_DARKGREEN);

  game.handleEvent(Event::Deal);
  Serial.printf("bankroll %d, player total %d\n", game.bankroll, game.player_hands[0].hand.total());

}

void loop() {
  Serial.printf("alive: bankroll %d\n", game.bankroll);
  delay(1000);    // once a second, so it doesn't flood the screen
}
