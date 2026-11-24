# esp32/ — the blackjack gadget

The Arduino sketch for the desk gadget: an ESP32 drives a 2.8" screen and plays the same game as
`make play`. All the blackjack rules come from `../core/`, unchanged. This folder only holds the
"glue": drawing on the screen and reading input.

**Status:** the full table draws on the screen, and you can play by typing keys in the Serial Monitor.
The knob and buttons are not wired yet.

## Parts

| Part | Notes |
|---|---|
| ESP32 DevKit, 30-pin (ESP-WROOM-32) | USB chip CP210x; shows up as `/dev/ttyUSB0` |
| 2.8" SPI TFT, ILI9341, 240×320 (red board) | touch and SD-card pins unused |
| breadboards, male-to-female and male-to-male jumper wires | |
| KY-040 rotary encoder + 4 push buttons | not wired yet |

## Screen wiring

Unplug the USB before changing any wire. Screen pins are listed left to right as printed on the module.

| Screen pin | ESP32 pin | Why |
|---|---|---|
| VCC | 3V3 | power (3.3 V, never 5 V) |
| GND | GND | ground |
| CS | D5 | chip select (the ESP32's standard SPI CS pin) |
| RESET | D4 | screen reset |
| DC | RX2 (GPIO 16) | command vs. pixel data |
| SDI (MOSI) | D23 | data to the screen (hardware SPI) |
| SCK | D18 | SPI clock (hardware SPI) |
| LED | D22 | backlight, so the program can switch it off |
| SDO (MISO), T_* pins | not connected | |

Pins avoided on purpose: TX0/RX0 (USB uploads), D0/D2/D12/D15 (boot "strapping" pins), D34–D39 (input only).

## Setting up the Arduino side (one time)

1. **Arduino IDE 2.x.** On Ubuntu it must be started with `--no-sandbox`:
   ```
   ~/Downloads/arduino-ide_2.3.10_Linux_64bit.AppImage --no-sandbox
   ```
2. **Allow USB serial access** (then log out and back in): `sudo usermod -aG dialout $USER`
3. **ESP32 board support:** Boards Manager → "esp32" by Espressif (tested with 3.3.12). Board: **ESP32 Dev Module**.
4. **Screen library:** Library Manager → **TFT_eSPI** (tested with 2.5.43). Then give it this gadget's pins:
   ```
   cp esp32/TFT_eSPI_User_Setup.h ~/Arduino/libraries/TFT_eSPI/User_Setup.h
   ```
5. **Let the Arduino build see `core/`:** link it into the Arduino libraries folder (one copy, shared with
   the PC tests):
   ```
   ln -s ~/Documents/blackjack/core ~/Arduino/libraries/blackjack_core
   ```
6. Open `esp32/esp32.ino`, pick the board and port, and press Upload.

## Playing without buttons (for now)

Open the Serial Monitor at **115200** baud, type a key, and press Enter:

| Key | Does |
|---|---|
| `+` / `-` | bet up / down |
| `n` | deal, and "next round" after a round ends |
| `h` `s` `d` `p` | hit, stand, double, split |

(`n` stands in for Enter, because the Serial Monitor sends a newline after every line.)

## Troubleshooting

- **The screen is white:** the screen library lost its settings. Redo step 4.
- **Upload stuck on "Connecting…":** hold the board's **BOOT** button until the percentages start.
- **The IDE becomes very slow:** usually a `Serial.print` in `loop()` with no delay, flooding the Serial Monitor.
  Only print or draw when something changes.
