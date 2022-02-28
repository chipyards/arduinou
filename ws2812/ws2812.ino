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
     et les tempo sont affectees d'un multiplicateur FMUL pour freq F_CPU multiple de 8MHz.
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
	- STM32 si STM32F10X_MD ou __STM32F1__ ou STM32F2XX (serait bon pour le F103)
 */

/* notes sur l'implementation STM32 (cortex M3) clockless_arm_stm32.h
	- mesure le temps au moyen du registre DWT_CYCCNT du "Data Watchpoint and Trace Unit"
	  qui se trouve a l'adresse 0xE0001004
   mais la compilation de la lib FastLED avec Atollic True Studio echoue (pb de STL, trop complique)
*/

/* note sur le signal vu a l'oscillo a la sortie de l'AVR:
   - periode 1250ns constante
   - pulse courte 250ns (la spec dit 400ns +- 150, on est au min)
   - pulse longue 875ns (la spec dit 850ns +- 150, on est bon)
   a la sortie d'une des LEDs:
   - pulse courte 315ns (la spec dit 400ns +- 150, on est bon )
   - pulse longue 660ns (la spec dit 850ns +- 150, on est bon )
 */

/* notes sur la commande des LEDs :
   - il y a une regulation de courant analog, assez lâche :
	- pour une baisse de voltage de 10% (de 5V a 4.5V) la conso baisse de 5%
	- pour une baisse de voltage de 20% (de 5V a 4V), la conso baisse de 9%
     (mesure sur 1 seule LED parmi RGB)
   - la commande d'intensite lumineuse est par PWM, linéaire,
	- frequence d'env. 4 kHz, résolution env. 1 us
	- non synchonisee avec le stream de commande ni avec les LEDs voisines
   - la spec de conso nominale est vague : 20 mA par LED pour WS2812, pas de spec pour WS2812B
     mesures sur la lampe Pi (128 LEDs), en mA par LED :
	VDD, PW		R		G		B		W
	5V, 0%		-		-		-		0.73
	5V, 25%		3.82		3.64		3.45		9.06
	4V, 0%		-		-		-		0.41
	4V, 25%		3.50		3.30		3.13
	4V, 50%		6.56		6.25		5.81
	4V, 100%	12.7		12.03		11.18
     Aussi bas que 3.5V un fonctionnement normal est obtenu.
     En white pur on a mesure moins que la somme des consos R, G, B (a elucider)
 */

#include <FastLED.h>

#define NUM_LEDS 4
#define LED_PIN 6 // D6

/// global storage ///

// LEDS
CRGB leds[NUM_LEDS];		// frame buffer

// intergral joysticks
const int loop_frequency = 20;  // should be a divider of 1000
const int loop_delay = 1000 / loop_frequency; // approximately
int centerX, centerY;
int paramX = 128, paramY = 128;	// valeurs integrees
const int deadspan = 100;
const int paramin = 0;   	// to be adjusted according to application needs
const int paramax = 255;	// "	"
// integration rate : at each iteration of the main loop,
// the joystick deviation divided by this number is added to the parameter.
// from 0 to paramax, it takes (rate_div*paramax)/(loop_frequency*joydev) seconds
// example : with main loop at 20 Hz (50 ms), rate_div = 32, paramax = 255, joydev = 412 :
// it takes 0.99 s to reach paramax from 0
const int rate_div = 64;


void setup() {
  Serial.begin(115200);
// LEDs
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setDither(DISABLE_DITHER);
  FastLED.setBrightness(69);
// on va utiliser le plotter pour observer les variables pilotees par joystick
// plot labels (Note : keep the AVR reset while starting the plotter)
  pinMode( 13, OUTPUT );	// board LED
  Serial.println("paramX paramY refPulse");
  centerX = analogRead(A0);
  centerY = analogRead(A1);
}

// a function to remove the deadband
int killdead( int v ) {
  if  ( v >= 0 ) {
      v -= deadspan;
      if  ( v < 0 ) v = 0;  
      }
  else {
      v += deadspan;
      if  ( v > 0 ) v = 0;  
      }
return v;
}

// traiter l'acquisition joystick
void joyproc() {
// read the inputs
  int joyX = analogRead(A0) - centerX;
  int joyY = analogRead(A1) - centerY;
  joyX = killdead( joyX );
  joyY = killdead( joyY );  
// integrate
  paramX += ( joyX / rate_div );
  paramY += ( joyY / rate_div );
// bound
  paramX = constrain( paramX, paramin, paramax );
  paramY = constrain( paramY, paramin, paramax );
// limit alarm
  if  ( ( paramX == paramin ) || ( paramX == paramax ) ||
        ( paramY == paramin ) || ( paramY == paramax )
      ) digitalWrite( 13, 1 );
  else digitalWrite( 13, 0 );
// plot  
  static int count = 0;
  // Note : the IDE plotter X range is 500 samples (fixed)
  // Serial.print( joyX );   Serial.print("  ");
  // Serial.print( joyY );  Serial.print("  ");
  Serial.print( paramX );   Serial.print("  ");
  Serial.print( paramY );   Serial.print("  ");
  // print also a reference pulse to keep the Y autoscale quiet
  if  ( count == 0 )
      Serial.println( 0 );
  else if ( count == 1 )
      Serial.println( 255 );
  else
      Serial.println( 0 );
  count = count + 1;
  if  ( count > 495 ) count = 0;  // a little less than the full plotter X range
}

void loop() {
// joystick
joyproc();
// LEDs
   leds[0] = CHSV(paramX,paramY,255);
   leds[1] = CHSV(paramX,paramY,192);
   leds[2] = CHSV(paramX,paramY,128);
   leds[3] = CHSV(paramX,paramY,64);
FastLED.show();
delay(loop_delay);        // approximate servo loop period (ms)
}
