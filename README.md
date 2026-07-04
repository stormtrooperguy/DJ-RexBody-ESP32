# DJ-RexBody-ESP32
ESP32 / PlatformIO port of [grnwaveworkshop/DJ-RexBody](https://github.com/grnwaveworkshop/DJ-RexBody), sample code to control DJ-Rex Body LEDs from Grnwave Workshop.

## Hardware

- **ESP32 development board**
- **3x body panel boards**, WS2811 LEDs (5V), daisy-chained via JST connectors. Each board carries 32 LEDs: 8 individually-addressable small LEDs plus 6 "buttons" of 4 LEDs each (the 4 LEDs in a button are always wired/driven as one unit, never split between colors).
- **2x eye LEDs**, WS2811, chained off the end of the last body panel.
- **5V power supply** for the LED chain, common ground with the ESP32.

Of the 6 buttons per panel, the current firmware only actively drives 3 (12 of the 24 button LEDs), plus the 2 eyes and the 8 small LEDs per panel (see [Small LED Map](#small-led-map)). The remaining 12 LEDs per panel are physically wired and present in the chain - left dark for now as headroom beyond that.

## Wiring Connections

### ESP32 Pin Assignments

| Component | ESP32 Pin | Notes |
|-----------|-----------|-------|
| **LED chain data** | GPIO 4 | WS2811 data line, start of the chain |

### LED Chain (WS2811)

```
ESP32 GPIO 4 → Panel A DIN ... DOUT → Panel B DIN ... DOUT → Panel C DIN ... DOUT → Eye 1 → Eye 2
ESP32 GND    → common ground with 5V supply
5V supply    → Panel/Eye VCC
```

Chain order matters and matches the LED index numbering in `src/main.cpp`: Panel A occupies indices 0-31, Panel B 32-63, Panel C 64-95, then the 2 eyes at 96-97. Boards connect to each other via JST.

**Note**: WS2811 data lines are commonly happier with a clean signal - consider a 470Ω series resistor on the data line if you see flicker, and a bulk capacitor (~1000µF) across 5V/GND near the start of the chain for power stability. Confirm current draw against your specific board's datasheet before sizing the 5V supply; 98 LEDs at full brightness/full white can add up quickly even though this firmware runs well under that (`BRIGHTNESS` is capped at 90/255, and most patterns use white/blue, not full RGB white).

### Active Button Map

For reference while testing - the button positions the firmware currently drives, by absolute LED index in the chain:

| Button | LED indices | Color |
|--------|-------------|-------|
| A1 | 12-15 | White |
| A2 | 24-27 | Blue |
| A3 | 28-31 | Blue |
| B1 | 40-43 | White |
| B2 | 44-47 | Blue |
| B3 | 48-51 | White |
| C1 | 84-87 | Blue |
| C2 | 88-91 | Blue |
| C3 | 92-95 | Blue |

Eyes are indices 96-97 and animate independently of the buttons (see `RandomEyes()`).

### Small LED Map

Each panel also has 8 individually-addressable "small" LEDs, confirmed by lighting candidate ranges and checking against the physical boards: they're the first 8 LEDs of each panel, right before that panel's buttons. Driven by `SmallLEDTwinkle()` (see below) - each LED is independent, unlike the buttons.

| Panel | LED indices |
|-------|-------------|
| A | 0-7 |
| B | 32-39 |
| C | 64-71 |

The remaining 12 LEDs per panel (not buttons, not these 8) are wired into the chain but intentionally left dark.

## Build

This is a [PlatformIO](https://platformio.org/) project targeting a generic ESP32 dev board.

```
pio run                    # build
pio run --target upload    # flash
pio device monitor         # serial console
```

Source lives in `src/main.cpp`. The random seed uses `esp_random()` instead of the Nano's `analogRead(0)`, since ESP32 doesn't need a floating analog pin to seed randomness.

The button flicker, small LED twinkle, and eye animation all run off a single tick paced by `FRAMES_PER_SECOND` (30fps), each keeping its own independent cycle timer.

`ButtonFlicker()` re-rolls all 9 active buttons on a randomized 300-1200ms cycle: each button independently gets a fresh on/off coin flip, but at least one button is always forced on so the panels are never fully dark.

`SmallLEDTwinkle()` re-rolls all 24 small LEDs on a separate, faster randomized 150-500ms cycle: each LED independently rolls white, red, or off. Unlike the buttons, there's no "keep at least one on" constraint here - all 24 can go dark at once.

The remaining 12 LEDs per panel (36 total) are currently left unlit.
