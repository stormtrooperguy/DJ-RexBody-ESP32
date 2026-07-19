// DJ Rex LED Panels
// Requires FASTLED Library - https://github.com/FastLED/FastLED
// Ported from DJLEDNanoV2.ino (Arduino Nano) to ESP32 / PlatformIO.

#include <Arduino.h>
#include <FastLED.h>

// How many NeoPixels are attached
#define NUM_LEDS 98
#define BRIGHTNESS 90

// Setup the LED Matrix
#define LED_PIN    4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define FRAMES_PER_SECOND  30

// define the 4 LED button starting LED numbers
#define PanelAStart 0
#define PanelA1 PanelAStart + 12
#define PanelA2 PanelA1 + 12
#define PanelA3 PanelA2 + 4

#define PanelBStart PanelAStart + 32
#define PanelB1 PanelBStart + 8
#define PanelB2 PanelB1 + 4
#define PanelB3 PanelB2 + 4

#define PanelCStart PanelBStart + 32
#define PanelC1 PanelCStart + 20
#define PanelC2 PanelC1 + 4
#define PanelC3 PanelC2 + 4

#define cGOLD 0xFFDD88
#define cWHITE 0xFFFFFF
#define cBLUE 0x0000FF
#define cRED 0xFF0000

uint16_t IntervalTime[NUM_LEDS];
unsigned long LEDMillis[NUM_LEDS];
bool LEDOn[NUM_LEDS];

CRGB DJLEDs[NUM_LEDS];

uint8_t LEDBrightness[NUM_LEDS] = { BRIGHTNESS,BRIGHTNESS };
uint8_t LEDMinBrightness[NUM_LEDS] = { BRIGHTNESS,BRIGHTNESS };

// Command loop processing time: one tick drives the button pattern, eyes,
// and FastLED.show(), paced to FRAMES_PER_SECOND.
unsigned long previousMillis = millis();
const unsigned long FrameInterval = 1000 / FRAMES_PER_SECOND;

// The 9 large buttons (4 LEDs each), one per panel position that's actually
// wired. Each button is a single fixed color and is only ever fully on or
// fully off - its 4 LEDs never show different colors from each other.
#define NUM_BUTTONS 9
const uint8_t ButtonStart[NUM_BUTTONS] = {PanelA1, PanelA2, PanelA3, PanelB1, PanelB2, PanelB3, PanelC1, PanelC2, PanelC3};
const CRGB ButtonColor[NUM_BUTTONS]    = {cBLUE,   cBLUE,   cWHITE,  cBLUE,   cWHITE,  cWHITE,  cWHITE,  cBLUE,   cBLUE};
bool ButtonOn[NUM_BUTTONS];
unsigned long ButtonCycleMillis = millis();
unsigned long ButtonCycleInterval = 0;

// The 8 individually-addressable "small" LEDs per panel that we actually use,
// confirmed by physical LED testing: they're the first 8 LEDs of each panel,
// right before that panel's buttons. Each panel also wires 12 further LEDs
// beyond these 8 + the 3 active buttons; those stay dark for now.
#define NUM_PANELS 3
#define NUM_SMALL_LEDS_PER_PANEL 8
const uint8_t SmallLEDStart[NUM_PANELS] = {PanelAStart, PanelBStart, PanelCStart};
unsigned long SmallLEDCycleMillis = millis();
unsigned long SmallLEDCycleInterval = 0;

void RandomEyes();
void ButtonFlicker();
void SmallLEDTwinkle();

// setup() function -- runs once at startup --------------------------------

void setup() {

	Serial.begin(115200);

	// tell FastLED about the LED strip configuration
	FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(DJLEDs, NUM_LEDS).setCorrection(TypicalLEDStrip);

	// set master brightness control
	FastLED.setBrightness(BRIGHTNESS);

	randomSeed(esp_random());
	// Seed the Array
	for (byte x = 0; x < NUM_LEDS; x++) {
		IntervalTime[x] = random16(3000);
		LEDMillis[x] = millis();
		LEDOn[x] = 0;
	}

}

// loop() function -- runs repeatedly as long as board is on ---------------

void loop() {
	if (millis() - previousMillis > FrameInterval) {
		previousMillis = millis();
		ButtonFlicker();
		SmallLEDTwinkle();
		RandomEyes();
		FastLED.show();
	}
}

// Random Eyes
// Simple mostly solid eyes with a bit of flicker to them.
void RandomEyes()
{
  byte i;
  byte y;
  int pos;

  for (pos = 96; pos < NUM_LEDS; pos++) {
    if (!LEDOn[pos]) {	// Fade LEDs up or down
        DJLEDs[pos].maximizeBrightness(LEDBrightness[pos]);
        if (LEDBrightness[pos] < BRIGHTNESS) LEDBrightness[pos]++;
//      DJLEDs[pos].fadeToBlackBy(8);
    }
    else {
        DJLEDs[pos].maximizeBrightness(LEDBrightness[pos]);
        if (LEDBrightness[pos] > LEDMinBrightness[pos]) LEDBrightness[pos]--;
    }
    if (millis() - LEDMillis[pos] > IntervalTime[pos]) {
      if (!LEDOn[pos]) { // LED Off - turn in on
          DJLEDs[pos] = cGOLD;
        IntervalTime[pos] = random(200, 1600);
        LEDMillis[pos] = millis();
        LEDOn[pos] = 1;
        LEDMinBrightness[pos] = random(BRIGHTNESS * 0.2, BRIGHTNESS);
      }
      else {	// Turn the LED off
        IntervalTime[pos] = random(200, 2000);
        LEDMillis[pos] = millis();
        LEDOn[pos] = 0;
      }
    }
  }
}

// Button Flicker
// Every randomized 300-1200ms cycle, re-rolls each of the 9 buttons on or
// off (instant, no fade). Multiple buttons can be on or off together, but
// at least one is always forced on so the panels are never fully dark.
void ButtonFlicker()
{
	if (millis() - ButtonCycleMillis > ButtonCycleInterval) {
		ButtonCycleMillis = millis();
		ButtonCycleInterval = random(300, 1201);

		bool anyOn = false;
		for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
			ButtonOn[i] = random8() < 128;
			if (ButtonOn[i]) anyOn = true;
		}
		if (!anyOn) {
			ButtonOn[random8(NUM_BUTTONS)] = true;
		}

		for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
			CRGB color = ButtonOn[i] ? ButtonColor[i] : CRGB::Black;
			for (uint8_t j = 0; j < 4; j++) {
				DJLEDs[ButtonStart[i] + j] = color;
			}
		}
	}
}

// Small LED Twinkle
// Every randomized 150-500ms cycle (faster than the button cycle), each of
// the 24 small LEDs independently rolls white, red, or off (instant, no fade).
void SmallLEDTwinkle()
{
	if (millis() - SmallLEDCycleMillis > SmallLEDCycleInterval) {
		SmallLEDCycleMillis = millis();
		SmallLEDCycleInterval = random(150, 501);

		for (uint8_t p = 0; p < NUM_PANELS; p++) {
			for (uint8_t i = 0; i < NUM_SMALL_LEDS_PER_PANEL; i++) {
				uint8_t pos = SmallLEDStart[p] + i;
				switch (random8(3)) {
					case 0:  DJLEDs[pos] = cWHITE; break;
					case 1:  DJLEDs[pos] = cRED; break;
					default: DJLEDs[pos] = CRGB::Black; break;
				}
			}
		}
	}
}
