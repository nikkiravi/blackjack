// esp32/TFT_eSPI_User_Setup.h: the screen library's settings for this gadget (a backup copy kept in git)
//
// TFT_eSPI does NOT read this file from here. It reads ~/Arduino/libraries/TFT_eSPI/User_Setup.h.
// If the library is ever reinstalled or updated, that file is reset and the screen stays blank/white:
// copy this file over it (keep the name User_Setup.h there), then upload again.
//
//   cp esp32/TFT_eSPI_User_Setup.h ~/Arduino/libraries/TFT_eSPI/User_Setup.h
//
// (It has a different name here on purpose, so the Arduino build can't confuse it with the real one.)

#define USER_SETUP_INFO "Blackjack gadget"

// ---- the display ----
#define ILI9341_DRIVER          // the red 2.8" 240x320 SPI module

// ---- wiring: ESP32 DevKit (30-pin) pins, see esp32/README.md for the breadboard ----
#define TFT_MISO  19            // screen SDO(MISO): not wired, but the SPI bus needs a pin number
#define TFT_MOSI  23            // screen SDI(MOSI)
#define TFT_SCLK  18            // screen SCK
#define TFT_CS     5            // screen CS
#define TFT_DC    16            // screen DC     (board label RX2)
#define TFT_RST    4            // screen RESET
#define TFT_BL    22            // screen LED    (backlight; tft.init() switches it on)
#define TFT_BACKLIGHT_ON HIGH

// ---- fonts (the library's defaults; the sketch uses the built-in font 1, scaled with setTextSize) ----
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// ---- SPI speed ----
#define SPI_FREQUENCY       27000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
