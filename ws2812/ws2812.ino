/*
Notes sur FastLED

https://github.com/FastLED/FastLED/wiki/Overview

1) l'objet FastLED de la classe CfastLED est cree en singleton global par FastLED.cpp

2) on cree un array de CRGB qui est un frame-buffer 1D = ruban
   (un CRGB est une structure de 3 bytes r, g, b)

3) on connecte ce frame buffer au FastLED:
	FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
   le 1er param du template est une designation de type (qui est un template de classe!!!)
   le 2nd est un byte, le 3eme une valeur de l'enum Eorder. (cf ligne 302 de FastLed.h)
   - Ce addLeds est un template de fonction qui va engendrer une fonction qui cree un objet
     statique c de la classe WS2812, et appelle addleds( &c, leds, NUM_LEDS, 0 ).
   - WS2812 etant un template va recevoir comme params LED_PIN et GRB, et heriter du template
     WS2812Controller800Khz en lui passant ces memes params (cf ligne 105 de FastLed.h)
   - Le template WS2812Controller800Khz va heriter du template ClocklessController, en lui passant
     en plus 3 valeurs de timing 250ns, 625ns, 375ns (cf ligne 474 de chipsets.h)
     representees par 2*FMUL, 5*FMUL, 3*FMUL, car l'unite de temps est 125 ns (ref clock = 8 MHz)
     et les tempo sont affectees d'un multiplicateur FMUL pour freq F_CPU multiple de 8MHz
   - le template ClocklessController est dependant de la plateforme, il est defini dans:
	- clockless_trinket.h pour AVR
	- clockless_arm_stm32.h pour cortex M3
	- etc...
     il herite de CPixelLEDController (qui va gerer l'ordre RGB), et expose (enfin) 2 methodes:
	- init()
	- showpixels(pixels) qui appelle showRGBInternal(pixels) qui appelle 3 fois
	  writeBits() qui effectue le job de serialisation d'un byte avec timing hardware
   - tout cela pour appeler la methode addleds( &c, leds, NUM_LEDS, 0 ) cf ligne 30 de FastLED.cpp
     qui considere c comme un CLEDController (ancetre de CPixelLEDController) et appelle ses methodes init() et setLeds().
   - Comment FastLED sait-il sur quelle plateforme on est ?
     Dans platforms.h il evalue differents macros:
	- AVR c'est le choix par defaut !
	- STM32 si STM32F10X_MD ou __STM32F1__ ou STM32F2XX (c'est bon pour le F103)
 */

/* notes sur l'implementation STM32 (cortex M3) clockless_arm_stm32.h
	- mesure le temps au moyen du registre DWT_CYCCNT du "Data Watchpoint and Trace Unit"
	  qui se trouve a l'adresse 0xE0001004
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
