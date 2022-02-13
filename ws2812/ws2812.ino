/*********************************************************************************************************
 */

#include <FastLED.h>

#define NUM_LEDS 128
#define LED_PIN 6 // D6

CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setDither(DISABLE_DITHER);
  FastLED.setBrightness(12);

}

void loop() {
/* blink 2 LEDs *
leds[0] = CRGB::Yellow; 
FastLED.show(); delay(250); 
leds[3] = CRGB::Green; 
FastLED.show(); delay(250); 
leds[0] = CRGB::Blue; 
FastLED.show(); delay(250); 
leds[3] = CRGB::Red; 
FastLED.show(); delay(250); 
//*/

/* scroll along all LEDs, monochrome *
for ( int i = 0; i < NUM_LEDS; ++i )
  {
   leds[i] = 0;
   leds[(i+1)%NUM_LEDS] = CRGB::White;;
   FastLED.show(); delay(10);
  }
//*/

/* scroll along all LEDs, rainbow */
for ( int i = 0; i < NUM_LEDS; ++i )
  {
   leds[i] = 0;
   leds[(i+1)%NUM_LEDS] = CHSV(0,255,255);
   leds[(i+2)%NUM_LEDS] = CHSV(36,255,255);;
   leds[(i+3)%NUM_LEDS] = CHSV(73,255,255);
   leds[(i+4)%NUM_LEDS] = CHSV(109,255,255);
   leds[(i+5)%NUM_LEDS] = CHSV(145,255,255);
   leds[(i+6)%NUM_LEDS] = CHSV(182,255,255);
   leds[(i+7)%NUM_LEDS] = CHSV(218,255,255);
   FastLED.show(); delay(100);
  }
//*/

}
