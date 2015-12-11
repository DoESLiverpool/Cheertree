/*

 CheerLights!

 CheerLights Channel --> Arduino Ethernet --> Addressable LED strip

 requires:
     FastLED library
     http://fastled.io/
     TimerOne
     https://github.com/PaulStoffregen/TimerOne
*/

#include <SPI.h>
#include "FastLED.h"

// Color Effects Setup
#define NUM_LEDS 50 // Total # of lights on string (usually 50, 48, or 36)
#define FIRST_LED 12 // ignore any LEDs before this number

#define BRIGHTNESS 30 // overall strip brightness level
#define STAR_BRIGHTNESS 100 // you might want to adjust this if encapsulated

// some case constants:
#define RISE 0
#define HOLD 1
#define FALL 2
#define PAUSE 3

// Set the max and min values for flicker
#define LOW_MIN 10
#define LOW_MAX 30
#define HIGH_MIN 30
#define HIGH_MAX 40
// hold length at high value
#define HOLD_MIN 100  //milliseconds
#define HOLD_MAX 500 //milliseconds
// pause length at low value
#define PAUSE_MAX 100 //milliseconds
#define PAUSE_MIN 500 //milliseconds
// rate of rise and fall
#define RISE_MIN 30 // dV_out/ms
#define RISE_MAX 100   // dV_out/ms
#define FALL_MIN 30 // dV_out/ms
#define FALL_MAX 100 // dV_out/ms

#define WARMWHITE 0xFFDFDF
#define OLDLACE   0xFFDFDF
#define WHITE     0xFFFFFF
#define BLACK     0x000000
#define RED       0xFF0000
#define GREEN     0x00FF00
#define BLUE      0x0000FF
#define CYAN      0x00FFFF
#define MAGENTA   0xFF00FF
#define YELLOW    0xFFFF00
#define PURPLE    0x800080
#define ORANGE    0xFF5A00
#define PINK      0xCBC0FF

#define NO_STAR	-1
// Position of the LED which is the "star" on the tree
// Set to NO_STAR to not have a star LED
int starLED = 49;
// Number of colours in the starColours sequence
const int starColoursCount = 10;
// Colour sequence that the "star" LED will cycle through whenever
// the main tree colour changes
uint32_t starColours[starColoursCount] = { RED, ORANGE, YELLOW, GREEN, CYAN, BLUE, MAGENTA, PURPLE };
// Final colour that the star is set to in between changes
uint32_t starEndColour = YELLOW;

#define DATA_PIN 2
#define CLOCK_PIN 3
//Define the array of LEDs
CRGB leds[NUM_LEDS];
uint8_t current_brightness[NUM_LEDS];
uint8_t target_brightness[NUM_LEDS];
uint8_t rate_of_change[NUM_LEDS];
uint8_t state[NUM_LEDS];
uint32_t change_time[NUM_LEDS];

// Variable Setup
uint32_t lastCommand = WHITE;
uint32_t currentCommand = WHITE;

void setup() {
  // Setup Serial
  Serial.begin(9600);

  Serial.flush();
  delay(100);

  // Choose your attached LED strip, colour order set uncommented
  // e.g.
  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);

  // set all the LEDs to off
  colorWipe(BLACK, 0);

  // show initial status
  colorWipe(WHITE, 10);
}

void loop() {
  // change the colour has changed
  if (currentCommand != lastCommand) {
    colorWipe(currentCommand, 50);
    lastCommand = currentCommand;
    Serial.print(F("OK"));
  }

  animateLEDs();

  if (Serial.available()) {
    // quick delay, so we don't miss the end of the instruction
    delay(100);

    char c = Serial.read();
    if (c == 'r' || c == 'R') {
      currentCommand = RED;
    } else if (c == 'g') {
      currentCommand = GREEN;
    } else if (c == 'c') {
      currentCommand = CYAN;
    } else if (c == 'm') {
      currentCommand = MAGENTA;
    } else if (c == 'y') {
      currentCommand = YELLOW;
    } else if (c == 'p') {
      c = Serial.read();
      if (c == 'u') currentCommand = PURPLE;
      else if (c == 'i') currentCommand = PINK;
      else {
        Serial.println ("unknown");
        currentCommand = lastCommand;
      }
    } else if (c == 'o') {
      c = Serial.read();
      if (c == 'r') currentCommand = ORANGE;
      else if (c == 'l') currentCommand = OLDLACE;
      else {
        Serial.println ("unknown");
        currentCommand = lastCommand;
      }
    } else if (c == 'w') {
      c = Serial.read();
      if (c == 'h') currentCommand = WHITE;
      else if (c == 'a') currentCommand = WARMWHITE;
      else {
        Serial.println ("unknown");
        currentCommand = lastCommand;
      }
    } else if (c == 'b') {
      c = Serial.read();
      if (c == 'l') {
        c = Serial.read();
        if (c == 'a') currentCommand = BLACK;
        else if (c == 'u') currentCommand = BLUE;
        else {
          Serial.println ("unknown");
          currentCommand = lastCommand;
        }
      }
    } else {
      Serial.print("unknown: ");
      Serial.println(c);
      currentCommand = lastCommand;
    }

    while (Serial.available()) {
      c = Serial.read();
    }
  }
}

// fill the dots one after the other with said color
void colorWipe(uint32_t c, uint8_t wait) {
  int i;
  int starColourIdx = 0;

  for (i=0; i < NUM_LEDS; i++) {
      leds[i] = c;
      if (starLED != NO_STAR) {
        // Show a star
        leds[starLED] = starColours[starColourIdx++];
        starColourIdx = starColourIdx % starColoursCount;
      }
      if (i >= FIRST_LED) {
        leds[i].nscale8_video(BRIGHTNESS);
        //FastLED.setBrightness(BRIGHTNESS);
        FastLED.show();
        delay(wait);
      } else {
        leds[i] = 0;
      }
  }
  if (starLED != NO_STAR) {
    // Ensure the star ends on the right colour
    leds[starLED] = starEndColour;
    leds[starLED].nscale8_video(STAR_BRIGHTNESS);
    //FastLED.setBrightness(BRIGHTNESS);
    FastLED.show();
  }
}

void animateLEDs()
{
  unsigned long frame_time = millis();

  for (int i=0; i<NUM_LEDS; i++) {
    if (state[i] == RISE) {
      // increment the led level
      int no_steps = (frame_time - change_time[i])*100 / rate_of_change[i];
      if (no_steps > 0) {
        current_brightness[i] = current_brightness[i] + no_steps;
        change_time[i] = frame_time;
      }
      // if we've reached the max, change to HOLD mode
      if (current_brightness[i] >= target_brightness[i]) {
        current_brightness[i] = target_brightness[i];
        change_time[i] = frame_time + random(HOLD_MIN, HOLD_MAX);
        state[i] = HOLD;
      }
      setBrightness(i);
    } else if (state[i] == HOLD) {
      // wait until the next required change time
      if (frame_time >= change_time[i]) {
        //switch to FALL mode and calculate the rate of change
        rate_of_change[i] = random(FALL_MIN, FALL_MAX);
        if (rate_of_change[i] < FALL_MIN) rate_of_change[i] = FALL_MIN;
        target_brightness[i] = random(LOW_MIN, LOW_MAX);
        state[i] = FALL;
     }
    } else if (state[i] == FALL) {
      // decrement the led level
      int no_steps = (frame_time - change_time[i])*100 / rate_of_change[i];
      if (no_steps > 0) {
        current_brightness[i] = current_brightness[i] - no_steps;
        change_time[i] = frame_time;
      }
      // if we've reached the min, change to HOLD mode
      if (current_brightness[i] <= target_brightness[i]) {
        current_brightness[i] = target_brightness[i];
        change_time[i] = frame_time + random(PAUSE_MIN, PAUSE_MAX);
        state[i] = PAUSE;
      }
      setBrightness(i);
     } else if (state[i] == PAUSE) {
      // wait until the next required change time
      if (frame_time >= change_time[i]) {
        //switch to RISE mode and calculate the rate of change
        rate_of_change[i] = random(RISE_MIN, RISE_MAX);
        if (rate_of_change[i] < RISE_MIN) rate_of_change[i] = RISE_MIN;
        target_brightness[i] = random(HIGH_MIN, HIGH_MAX);
        state[i] = RISE;
      }
     } else {
      // reset the state to rise
      state[i] = PAUSE;
    }
  }

  // then display the frame
  FastLED.show();

  delay(1);
}

void setBrightness(uint8_t led_number) {
  if (starLED != NO_STAR && led_number == starLED) {
    leds[led_number] = starEndColour;
    leds[led_number].nscale8_video(STAR_BRIGHTNESS);
  } else if (led_number >= FIRST_LED) {
    leds[led_number] = currentCommand;
    leds[led_number].nscale8_video(current_brightness[led_number]);
  } else {
    leds[led_number] = 0;
  }
}
