# DJ-RexBody-ESP32
ESP32 / PlatformIO port of [grnwaveworkshop/DJ-RexBody](https://github.com/grnwaveworkshop/DJ-RexBody), sample code to control DJ-Rex Body LEDs from Grnwave Workshop.

Uses GPIO 4 of an ESP32 (ported from the original Arduino Nano sketch) and the FastLED library.

Eye LEDs are also included as daisy chain from the body LEDs.

Assumes 3 Body LED boards with 8 small LEDs, and 6 groups of 4 of the 5050 LEDs.

## Build

This is a [PlatformIO](https://platformio.org/) project targeting a generic ESP32 dev board.

```
pio run                    # build
pio run --target upload    # flash
pio device monitor         # serial console
```

Source lives in `src/main.cpp`. The random seed uses `esp_random()` instead of the Nano's `analogRead(0)`, since ESP32 doesn't need a floating analog pin to seed randomness.
