#include <FastLED.h>
// How many leds in your strip?
#define NUM_LEDS 360 //201 LEDs per branch

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 23
#define CLOCK_PIN 18
// Define the array of leds
CRGB leds[NUM_LEDS];
int offset = 0;
byte masterhue;
void setup() { 
// Uncomment/edit one of the following lines for your leds arrangement.
// FastLED.addLeds<TM1803, DATA_PIN, RGB>(leds, NUM_LEDS);
// FastLED.addLeds<TM1804, DATA_PIN, RGB>(leds, NUM_LEDS);
// FastLED.addLeds<TM1809, DATA_PIN, RGB>(leds, NUM_LEDS);
// FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
// FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
 FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
//FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
// FastLED.addLeds<APA104, DATA_PIN, RGB>(leds, NUM_LEDS);
// FastLED.addLeds<UCS1903, DATA_PIN, RGB>(leds, NUM_LEDS);
// FastLED.addLeds<UCS1903B, DATA_PIN, RGB>(leds, NUM_LEDS);
// FastLED.addLeds<GW6205, DATA_PIN, RGB>(leds, NUM_LEDS);
// FastLED.addLeds<GW6205_400, DATA_PIN, RGB>(leds, NUM_LEDS);
// FastLED.addLeds<WS2801, RGB>(leds, NUM_LEDS);
// FastLED.addLeds<SM16716, RGB>(leds, NUM_LEDS);
// FastLED.addLeds<LPD8806, RGB>(leds, NUM_LEDS);
// FastLED.addLeds<P9813, RGB>(leds, NUM_LEDS);
// FastLED.addLeds<APA102, RGB>(leds, NUM_LEDS);
// FastLED.addLeds<DOTSTAR, RGB>(leds, NUM_LEDS);
// FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
// FastLED.addLeds<SM16716, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
// FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
// FastLED.addLeds<P9813, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  //     FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS);
// FastLED.addLeds<DOTSTAR, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);

//This is where the power is regulated.  These pebble lights are kinda weird, so it will be some trial an error....
 FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);


leds[0] = CRGB::Blue;
FastLED.show();
delay(500);
masterhue = 0;
}


void loop() { 
//Each loop 
// *output 1 LED frame
// *check for wifi communications
// *check for sensor detection


// // Turn the LED on, then pause
// for(int i=0;i<NUM_LEDS;i++){
//     leds[i] = CRGB::Red;
//   }
//   FastLED.show();
// delay(500);
// for(int i=0;i<NUM_LEDS;i++){
//     leds[i] = CRGB::Green;
//   }
//   FastLED.show();
// delay(500);
// for(int i=0;i<NUM_LEDS;i++){
//     leds[i] = CRGB::Blue;
//   }
//   FastLED.show();
// delay(500);
// // Now turn the LED off, then pause
// for(int i=0;i<NUM_LEDS;i++){
//     leds[i] = CRGB::Black;
//   }
//   FastLED.show();
// delay(500);

  for (int i = 0; i < NUM_LEDS; ++i) leds[i] = CRGB::Black; 
  byte hue = 0;
  if (masterhue >= 255) {
    masterhue = 0;
  } else {
    ++masterhue;
  }
  hue = masterhue;
 
  int step = 1;
  for (int i=0;i<(NUM_LEDS/step)-offset; ++i) {
    if (i*step+offset < NUM_LEDS) {
      leds[i*step+offset] = CHSV(hue, 255, 255);
      if (hue >= 255) {
        hue = 0;
      } else {
        ++hue;
      }
    }
  }
  FastLED.show();
  //delay(1);
  ++offset;
  if (offset>=step) offset = 0;
}