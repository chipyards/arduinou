/*********************************************************************************************************
 * Credits:
   Firepit Effect: FastLED Fire 2018 by Stefan Petrick, see https://www.youtube.com/watch?v=SWMu-a9pbyk
   code https://gist.github.com/StefanPetrick
   Firework Effect: FireworksXY by Mark Kriegsman, July 2013
 */

/* 
Le programme utilise leds[], un frame buffer 1D de 128 pixels de 32 bits, le ruban en zig-zag forme une matrice de 16 x 8
La fonction XY(x,y) rend un index sur leds[], lineaire sur le ruban.
Son contenu est emis vers le ruban par FastLED.show(), appele par chaque effet sauf fireworks qui utilise LEDS.show();
(#define LEDS FastLED : that's how we're screwed !!)
 
Commentaires des 8 effets:
	xyTester()
		une spirale qui s'allonge autour de la lampe en changeant lentement de teinte a chaque pixel.
		modifie 1 pixel par loop, utilise XY()
	hueRotationEffect,
		modulation de la teinte de tous les pixels, qui sont allumes en permanence
		vagues sinusoidales, utilise XY()
		pas de tempo : depend de la vitesse du CPU ? et la fonction millis() ?
	animatePacChase,
		utilise 3 sprites pinkGhost, pacmanOpenMouth, pacmanClosedMouth, tous de 9 x 8 (pleine hauteur)
		=> les sprites ne bougent que en X
		moveSprite() calcule x et y d'un sprite selon les vitesses dx et dy qui sont dans le sprite
		showSprite() copie le sprite dans le frame buffer pixel par pixel avec XY() puis appelle FastLED.show()
    N.B. il n'y a pas d'effacement des sprites !! mais ils font la pleine hauteur et ont une colonne de pixels noirs de chaque cote
	animatePacman,
		utilise 3 sprites pacmanOpenMouth, pacmanClosedMouth, et 4 copies de pacGum (4 pixels blancs)
		les sprites ne bougent que en X
	hue,
		modulation de la teinte sur des bandes verticales
	fireworks,
		utilise une classe Dot pour tracer des dots de 4 pixels
	firepit,
		utilise une palette de 16 couleurs pour rendre 16 "temperatures"
	nothing,
    rien

*/
#include <FastLED.h>
#include "pixelamp2.h"

// options
#define FIRST_LED_BURNT
// #define SERIAL_DEBUG

const uint8_t kMatrixWidth = 16;
const uint8_t kMatrixHeight = 8;
uint8_t CentreX =  (kMatrixWidth / 2) - 1;
uint8_t CentreY = (kMatrixHeight / 2) - 1;

uint16_t brightness = 1;
uint16_t currentEffect = 0;

CRGB leds[NUM_LEDS];
#ifdef FIRST_LED_BURNT
CRGB * leds_skip1 = leds + 1;  // pointeur pour sauter la premiere LED, dessoudee
#endif


/*********************************************************************************************************
 * MAIN
 */


// table de fonctions
void (*effects[])() = {
  xyTester,
  hueRotationEffect,
  animatePacChase,
  animatePacman,
  hue,
  fireworks,
  firepit,
  theMatrix,
  TeslaRings,
  concentric
  //nothing
};

// table de brightness max
uint8_t lumax[] = {
  60,   // 133,  // xyTester,         
  62,   // 138,  // hueRotationEffect,
  45,   // 99,   // animatePacChase,  
  45,   // 99,   // animatePacman,
  60,   // 133,  // hue,
  90,   // 126,  // fireworks,
  66,   // 255,  // firepit,
  60,   // 160,  // theMatrix,
  90,   // 200,  // TeslaRings,
  90,   // 200   // concentric
};

#ifdef SERIAL_DEBUG
char opcode = 0;
#endif

void setup() {
#ifdef SERIAL_DEBUG
  Serial.begin(115200);
  //Serial.println(F("Hello c'est imposant"));
#endif

#ifdef FIRST_LED_BURNT
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds_skip1, NUM_LEDS-1).setCorrection(Halogen);
#else
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(Halogen);
#endif

  FastLED.setBrightness(brightness);
  FastLED.setDither(DISABLE_DITHER);

  // copier la palette de flash vers RAM
  // Pal = LavaColors_p;
  // ce n'est pas trivial : cf colorutils.h de FastLED
  // CRGBPalette16& operator=( const TProgmemRGBPalette16& rhs)

}

void loop() {
  effects[currentEffect]();
  changeAnimation();
  changeBrightness( false );
#ifdef SERIAL_DEBUG
  int c = Serial.read();
  if  ( c > ' ' ) {   // N.B. le serial monitor envoie des fins de ligne !
      opcode = c;
      Serial.print("opcode "); Serial.println( (char)opcode ); 
      }
#endif
}

 /************************************************************************
 * Controls                                                             *
 ************************************************************************/
 // les MIN et MAX sont surtout utiles si on a des potentiometres type joystick
 // qui n'utilisent pas la pleine course...
#define POT_ANIM              A1
#define POT_ANIM_MIN          24
#define POT_ANIM_MAX          1000
#define POT_BRIGHTNESS        A0
#define POT_BRIGHTNESS_MIN    200   // potentiometre de Pi defectueux
#define POT_BRIGHTNESS_MAX    1000
#define MAX_BRIGHTNESS        150
#define HYSTERESIS            9

void changeAnimation() {
  static int oldout = 0;
  int newin = analogRead( POT_ANIM );
  int newout = oldout;
  if  ( ( newin - oldout ) > HYSTERESIS )
      newout = newin - HYSTERESIS;
  if  ( ( newin - oldout ) < -HYSTERESIS )
      newout = newin + HYSTERESIS;
  if  ( newout != oldout ) {
      oldout = newout;
      uint16_t newEffect = constrain( newout, POT_ANIM_MIN, POT_ANIM_MAX );
      newEffect = map( newEffect, POT_ANIM_MIN, POT_ANIM_MAX, 0, ARRAY_SIZE(effects) - 1 );
      if  ( newEffect != currentEffect ) {
          currentEffect = newEffect;
          wipeMatrices();
          changeBrightness( true ); // pour forcer un mapping avec la nouvelle valeur max
          }
#ifdef SERIAL_DEBUG
      Serial.print("pot anim "); Serial.print( newin ); Serial.print( ' ' ); Serial.print( newout ); Serial.print(" -> anim "); Serial.println( currentEffect );
#endif
      }
}

void changeBrightness( bool force ) {
  static int oldout = 0;
  int newin = analogRead( POT_BRIGHTNESS );
  int newout = oldout;
  if  ( ( newin - oldout ) > HYSTERESIS )
      newout = newin - HYSTERESIS;
  if  ( ( newin - oldout ) < -HYSTERESIS )
      newout = newin + HYSTERESIS;
  if  ( ( newout != oldout ) || force ) {
      oldout = newout;
      brightness = constrain( newout, POT_BRIGHTNESS_MIN, POT_BRIGHTNESS_MAX );
      brightness = map( brightness, POT_BRIGHTNESS_MIN, POT_BRIGHTNESS_MAX, 0, lumax[currentEffect] );
#ifdef SERIAL_DEBUG
      Serial.print("pot bright "); Serial.print( newin ); Serial.print( ' ' ); Serial.print( newout ); Serial.print(" -> bright "); Serial.println( brightness );
#endif
      FastLED.setBrightness(brightness);
      }
}



/*********************************************************************************************************
 * Spiral Effect
 */
void xyTester() {
  static uint8_t x=0;
  static uint8_t y=0;
  static uint8_t hue = 0;
  
#ifdef SERIAL_DEBUG
  CRGB common_rgb;
  switch (opcode) {
    case 'r' : common_rgb = CRGB( 255,0,0 ); break;
    case 'g' : common_rgb = CRGB( 0,255,0 ); break;
    case 'b' : common_rgb = CRGB( 0,0,255 ); break;
    case 'w' : common_rgb = CRGB( 255, 255, 255 ); break;
    default  : common_rgb = CRGB( 50, 50, 50 );
    }
  leds[XY(x, y)] = common_rgb;
#else
  leds[XY(x, y)] = CHSV(hue, 255, 255);
#endif
  FastLED.show();
  FastLED.delay(20);
  x++;
  if (x >= kMatrixWidth) {
    y++;
    x = 0;
    hue++;
  }
  if (y >= kMatrixHeight) {
    y = x = 0;
    wipeMatrices();
  }
}

/*********************************************************************************************************
 * Concentric Effect
 */
#define CQ 5

class cring { public:
  CRGB ccolor;
  uint8_t radius;
  uint8_t period; // en ticks
  uint16_t next;  // en ticks
  static uint16_t ticks;
  cring() {  ticks = 0; spawn( 0 ); };
  void spawn( uint8_t newr ) {
    ccolor = CHSV( random(70), 155 + random(100), 255 );
    radius = newr;
    period = 1; // en ticks
    next = ticks + 8 + random(12); 
    }
  void cstep( bool dir );
  void qset( uint8_t u, uint8_t v, CRGB c );
  void cset( CRGB c );
};  // class cring

cring crings[CQ];
uint16_t cring::ticks;

// trace d'un quadruple pixel, u de 1 a 8, v de 1 a 4
void cring::qset( uint8_t u, uint8_t v, CRGB c ) {
  leds[XY( 7 + u, 4 - v )] = c;
  leds[XY( 8 - u, 3 + v )] = c;
  leds[XY( 7 + u, 3 + v )] = c;
  leds[XY( 8 - u, 4 - v )] = c;
}

// trace d'un ring
void cring::cset( CRGB c ) {
  switch( radius) {
    case 1: qset( 1, 1, c ); break;
    case 2: qset( 1, 2, c ); qset( 2, 1, c ); break;
    case 3: qset( 1, 3, c ); qset( 3, 1, c ); qset( 2, 3, c ); qset( 3, 2, c ); break;
    case 4: qset( 1, 4, c ); qset( 2, 4, c ); qset( 3, 4, c );
            qset( 4, 1, c ); qset( 4, 2, c ); qset( 4, 3, c ); break;
    case 5: qset( 8, 4, c ); qset( 7, 4, c ); qset( 6, 4, c );
            qset( 5, 1, c ); qset( 5, 2, c ); qset( 5, 3, c ); break;
    case 6: qset( 8, 3, c ); qset( 6, 1, c ); qset( 7, 3, c ); qset( 6, 2, c ); break;
    case 7: qset( 8, 2, c ); qset( 7, 1, c ); break;
    case 8: qset( 8, 1, c ); break;
    
    }
}

// N.B. radius va de 1 a 8
// spawn met radius = 0 ==> prochain affichage a 1
void cring::cstep( bool dir ) {
  if  ( ticks >= next ) {
      if  ( dir )
          {
          if  ( radius > 0 ) {   // effacer ancienne position
              cset( CRGB::Black );
              }
          if  ( radius < 8 ) {
              radius++;
              next = ticks + period;
              cset( ccolor );
              }
          else spawn( 0 );
          }
      else {
          if  ( radius < 8 ) {   // effacer ancienne position
              cset( CRGB::Black );
              }
          if  ( radius > 0 ) {
              radius--;
              next = ticks + period;
              cset( ccolor );
              }
          else spawn( 8 );
          }
      }
}

void concentric() {
  for ( uint8_t i = 0; i < CQ; ++i ) {
      crings[i].cstep( cring::ticks & 128 );
  }
  ++ cring::ticks;
  FastLED.show();
  FastLED.delay(70);
}
/*********************************************************************************************************
 * Tesla Rings Effect
 */
#define TQ 5

class tring { public:
  CRGB tcolor;
  uint8_t y;
  uint8_t period; // en ticks
  uint16_t next;  // en ticks
  static uint16_t ticks;
  tring() {  ticks = 0; spawn(); };
  void spawn() {
    tcolor = CHSV( 130 + random(70), 155 + random(100), 255 );
    y = kMatrixHeight;
    period = 2 + random(5); // en ticks
    next = ticks + 25 + random(50); 
    }
  void tstep();
};  // class tring

tring rings[TQ];
uint16_t tring::ticks;

// N.B. spawn met y = kMatrixHeight ==> prochain affichage a kMatrixHeight-1
void tring::tstep() {
  if  ( ticks >= next ) {
      if  ( y < kMatrixHeight ) {   // effacer ancienne position
          for ( uint8_t x = 0; x < kMatrixWidth; ++x )
              leds[XY( x, y )] = CRGB::Black;
          }
      if  ( y > 0 ) {
          y--;
          next = ticks + period;
          for ( uint8_t x = 0; x < kMatrixWidth; ++x )
              leds[XY( x, y )] = tcolor;
          }
      else spawn();
      }
}

void TeslaRings() {
  for ( uint8_t i = 0; i < TQ; ++i ) {
      rings[i].tstep();
  }
  ++ tring::ticks;
  FastLED.show();
  FastLED.delay(20);
}

/*********************************************************************************************************
 * The Matrix Effect
 */
#define MQ 12
#define MPIXELS 6

CRGB mpal[MPIXELS] = { CRGB(0,128,32), CRGB(0,100,0), CRGB(0,50,0), CRGB(0,25,0), CRGB(0,12,0), CRGB(0,0,0) };

class comet { public:
  CRGB * pal;
  uint8_t x;
  uint8_t y;
  uint8_t period; // en ticks
  uint16_t next;  // en ticks
  static uint16_t ticks;
  comet() {  ticks = 0; spawn(); };
  void spawn() {
    pal = mpal;
    x = random( kMatrixWidth );
    y = 0;
    period = 2 + random(5); // en ticks
    next = ticks + period; 
    };
  void mstep();
  };  // class comet

comet mcomets[MQ];
uint16_t comet::ticks;

void comet::mstep() {
if  ( ticks >= next ) {
    y++;
    if  ( y > kMatrixHeight + MPIXELS )
        spawn();
    else next = ticks + period;
    uint8_t yy;
    for ( uint8_t i = 0; ( ( i < MPIXELS ) && ( i <= y ) ); ++i ) {
        yy = y - i;
        if  ( yy < kMatrixHeight )
            leds[XY( x, yy )] = pal[i];
        }
    }
}


void theMatrix() {
  for ( uint8_t i = 0; i < MQ; ++i ) {
      mcomets[i].mstep();
  }
  ++ comet::ticks;
  FastLED.show();
  FastLED.delay(20);
}


/*********************************************************************************************************
 * Hue Rotation Effect
 */
void DrawOneFrame( byte startHue8, int8_t yHueDelta8, int8_t xHueDelta8) {
  byte lineStartHue = startHue8;
  for( byte y = 0; y < kMatrixHeight; y++) {
    lineStartHue += yHueDelta8;
    byte pixelHue = lineStartHue;      
    for( byte x = 0; x < kMatrixWidth; x++) {
      pixelHue += xHueDelta8;
      leds[XY(x, y)]  = CHSV( pixelHue, 255, 255);
    }
  }
}

void hueRotationEffect() {
  uint32_t ms = millis();
  int32_t yHueDelta32 = ((int32_t)cos16( ms * (27/1) ) * (350 / kMatrixWidth));
  int32_t xHueDelta32 = ((int32_t)cos16( ms * (39/1) ) * (310 / kMatrixHeight));
  DrawOneFrame( ms / 65536, yHueDelta32 / 32768, xHueDelta32 / 32768);
  FastLED.show();
}

/*********************************************************************************************************
 * PACMAN Effects
 */

void animatePacChase() {  
  static boolean openMouth = true;
  
  showSprite(pinkGhost);
  showSprite(openMouth? pacmanOpenMouth:pacmanClosedMouth);

  openMouth = !openMouth;

  FastLED.delay(200);
  moveSprite(pacmanOpenMouth);
  moveSprite(pacmanClosedMouth);
  pinkGhost.x = pacmanOpenMouth.x + 8;
}

void animatePacman() {
  static boolean openMouth = true;
  struct Sprite<4> pacGum2 = pacGum;
  struct Sprite<4> pacGum3 = pacGum;
  struct Sprite<4> pacGum4 = pacGum;

  pacGum.x = 1;
  pacGum2.x = 5;
  pacGum3.x = 9;
  pacGum4.x = 13;
    
  showSprite(openMouth? pacmanOpenMouth:pacmanClosedMouth);

  int pacmanPosition = pacmanOpenMouth.x%16;

  //pacgum dissapearance handling
  if (pacmanPosition > 2 && pacmanPosition < 11) showSprite(pacGum);
  if (pacmanPosition > 6 && pacmanPosition < 15) showSprite(pacGum2);
  if (pacmanPosition > 10 || pacmanPosition < 3) showSprite(pacGum3);
  if (pacmanPosition > 14 || pacmanPosition < 7) showSprite(pacGum4);
  
  openMouth = !openMouth;

  FastLED.delay(200);
  moveSprite(pacmanOpenMouth);
  moveSprite(pacmanClosedMouth);
}

/*********************************************************************************************************
 * Hue Effect
 */

void hue() { 
  static uint8_t hue = 0;
  for(uint8_t y=0; y<kMatrixWidth; y++) {
    fill_solid(&leds[y*kMatrixHeight], kMatrixHeight, CHSV(hue+(kMatrixHeight*y), 255, 255));
  }
  FastLED.delay(10);
  FastLED.show();
  hue++;
}

/*********************************************************************************************************
 * Firework Effect
 */
CRGB overrun;
saccum78 gGravity = 10;
fract8  gBounce = 200;
fract8  gDrag = 255;
bool gSkyburst = 0;
accum88 gBurstx;
accum88 gBursty;
saccum78 gBurstxv;
saccum78 gBurstyv;
CRGB gBurstcolor;
Dot gDot;
Dot gSparks[NUM_SPARKS];

void fireworks() {
  random16_add_entropy( random() );
  CRGB sky1(0,0,2);
  CRGB sky2(2,0,2);

  memset8( leds, 0, NUM_LEDS * 3);

#if 1
   for( uint16_t v = 0; v < NUM_LEDS; v++) {
     leds[v] = sky1;
   }
   for( byte u = 0; u < 1; u++) {
    leds[random8(NUM_LEDS)] = sky2;
  }
#endif
  
  gDot.Move();
  gDot.Draw();
  for( byte b = 0; b < NUM_SPARKS; b++) {
    gSparks[b].Move();
    gSparks[b].Draw();
  }
  
  LEDS.show();
  static uint16_t launchcountdown = 0;
  if( gDot.show == 0 ) {
    if( launchcountdown == 0) {
      gDot.GroundLaunch();
      gDot.theType = SHELL;
      launchcountdown = random16( 350) + 1;
    } else {
      launchcountdown --;
    }
  }
  
  if( gSkyburst) {
    byte nsparks = random8( NUM_SPARKS / 2, NUM_SPARKS + 1);
    for( byte b = 0; b < nsparks; b++) {
      gSparks[b].Skyburst( gBurstx, gBursty, gBurstyv, gBurstcolor);
      gSkyburst = 0;
    }
  }

  FastLED.delay(10);
}

/*********************************************************************************************************
 * Firepit Effect
 */
uint32_t x;
uint32_t y;
uint32_t z;
uint32_t scale_x;
uint32_t scale_y;
uint8_t noise[16][8];
// heatmap data with the size matrix kMatrixWidth * kMatrixHeight
uint8_t heat[128];
CRGBPalette16 Pal = LavaColors_p;

void firepit() {
  // get one noise value out of a moving noise space
  uint16_t ctrl1 = inoise16(11 * millis(), 0, 0);
  // get another one
  uint16_t ctrl2 = inoise16(13 * millis(), 100000, 100000);
  // average of both to get a more unpredictable curve
  uint16_t  ctrl = ((ctrl1 + ctrl2) / 2);

  // this factor defines the general speed of the heatmap movement
  // high value = high speed
  uint8_t speed = 27;

  // here we define the impact of the wind
  // high factor = a lot of movement to the sides
  x = 3 * ctrl * speed;

  // this is the speed of the upstream itself
  // high factor = fast movement
  y = 15 * millis() * speed;

  // just for ever changing patterns we move through z as well
  z = 3 * millis() * speed ;

  // ...and dynamically scale the complete heatmap for some changes in the
  // size of the heatspots.
  // The speed of change is influenced by the factors in the calculation of ctrl1 & 2 above.
  // The divisor sets the impact of the size-scaling.
  scale_x = ctrl1 / 2;
  scale_y = ctrl2 / 2;

  // Calculate the noise array based on the control parameters.
  uint8_t layer = 0;
  for (uint8_t i = 0; i < kMatrixWidth; i++) {
    uint32_t ioffset = scale_x * (i - CentreX);
    for (uint8_t j = 0; j < kMatrixHeight; j++) {
      uint32_t joffset = scale_y * (j - CentreY);
      uint16_t data = ((inoise16(x + ioffset, y + joffset, z)) + 1);
      noise[i][j] = data >> 8;
    }
  }


  // Draw the first (lowest) line - seed the fire.
  // It could be random pixels or anything else as well.
  for (uint8_t x = 0; x < kMatrixWidth; x++) {
    // draw
    leds[XY(x, kMatrixHeight-1)] = ColorFromPalette( Pal, noise[x][0]);
    // and fill the lowest line of the heatmap, too
    heat[XY(x, kMatrixHeight-1)] = noise[x][0];
  }

  // Copy the heatmap one line up for the scrolling.
  for (uint8_t y = 0; y < kMatrixHeight - 1; y++) {
    for (uint8_t x = 0; x < kMatrixWidth; x++) {
      heat[XY(x, y)] = heat[XY(x, y + 1)];
    }
  }

  // Scale the heatmap values down based on the independent scrolling noise array.
  for (uint8_t y = 0; y < kMatrixHeight - 1; y++) {
    for (uint8_t x = 0; x < kMatrixWidth; x++) {

      // get data from the calculated noise field
      uint8_t dim = noise[x][y];

      // This number is critical
      // If it´s to low (like 1.1) the fire dosn´t go up far enough.
      // If it´s to high (like 3) the fire goes up too high.
      // It depends on the framerate which number is best.
      // If the number is not right you loose the uplifting fire clouds
      // which seperate themself while rising up.
      dim = dim / FIRE_HEIGHT;

      dim = 255 - dim;

      // here happens the scaling of the heatmap
      heat[XY(x, y)] = scale8(heat[XY(x, y)] , dim);
    }
  }

  // Now just map the colors based on the heatmap.
  for (uint8_t y = 0; y < kMatrixHeight - 1; y++) {
    for (uint8_t x = 0; x < kMatrixWidth; x++) {
      leds[XY(x, y)] = ColorFromPalette( Pal, heat[XY(x, y)]);
    }
  }

  // Done. Bring it on!
  FastLED.show();

  // I hate this delay but with 8 bit scaling there is no way arround.
  // If the framerate gets too high the frame by frame scaling doesn´s work anymore.
  // Basically it does but it´s impossible to see then...

  // If you change the framerate here you need to adjust the
  // y speed and the dim divisor, too.
  FastLED.delay(10);
}

/*
void nothing() {
#ifdef SERIAL_DEBUG
  CRGB common_rgb;      // test de toutes les LEDS
  switch (opcode) {
    case 'r' : common_rgb = CRGB( 255,0,0 ); break;
    case 'g' : common_rgb = CRGB( 0,255,0 ); break;
    case 'b' : common_rgb = CRGB( 0,0,255 ); break;
    case 'w' : common_rgb = CRGB( 255, 255, 255 ); break;
    default  : common_rgb = CRGB( 50, 50, 50 );
    }
  for ( uint16_t n=0; n<NUM_LEDS; n++ ) {
      leds[n] = common_rgb;
  }
#else
  wipeMatrices();
#endif
  FastLED.show();
}
*/
/*********************************************************************************************************
 * Helping functions
 */
//[0;0] is ont top-left-hand corner
uint16_t XY( uint8_t x, uint8_t y ) {

  //comment this line to put [0;0] on bottom-left-hand corner.
  //y = (kMatrixHeight-1) - y;
  
  x %= kMatrixWidth;
  y %= kMatrixHeight;

  //uncomment if LEDs are arranged in zigzag
  if (x%2 == 0) {
    y = (kMatrixHeight-1) - y;
  }
  return (x * kMatrixHeight) + y;
}


template <uint32_t N> void showSprite(const Sprite<N> &sprite) {
  for (uint8_t y=0; y<sprite.h; y++) {
    for (uint8_t x=0; x<sprite.w; x++) {
      uint8_t nx = sprite.x+x;
      uint8_t ny = sprite.y+y;
      leds[XY(nx, ny)] = CRGB(sprite.data[x+(y*sprite.w)]);
    }
  }
  FastLED.show();
}


template <uint32_t N> void moveSprite(Sprite<N> &sprite) {
  sprite.x += sprite.dx;
  sprite.y += sprite.dy;
}

void wipeMatrices() {
  for (uint16_t n=0; n<NUM_LEDS; n++) {
    leds[n]  = CHSV(0, 0, 0);
  }  
}

//fireworks
void screenscale( accum88 a, byte N, byte& screen, byte& screenerr) {
  byte ia = a >> 8;
  screen = scale8( ia, N);
  byte m = screen * (256 / N);
  screenerr = (ia - m) * scale8(255,N);
  return;
}


void plot88( byte x, byte y, CRGB& color) {
  byte ix = scale8( x, MODEL_WIDTH);
  byte iy = scale8( y, MODEL_HEIGHT);
  CRGB& px = leds[XY(ix, iy)];  // was false false
  px = color;
}


static int16_t scale15by8_local( int16_t i, fract8 scale ) {
    int16_t result;
    result = (int32_t)((int32_t)i * scale) / 256;
    return result;
}

Dot::Dot() {
  show = 0;
  theType = 0;
  x =  0;
  y =  0;
  xv = 0;
  yv = 0;
  r  = 0;
  color.setRGB( 0, 0, 0);
}
  
void Dot::Draw() {
  if( !show) return;
  byte ix, xe, xc;
  byte iy, ye, yc;
  screenscale( x, MODEL_WIDTH, ix, xe);
  screenscale( y, MODEL_HEIGHT, iy, ye);
  yc = 255 - ye;
  xc = 255 - xe;
  
  CRGB c00 = CRGB( dim8_video( scale8( scale8( color.r, yc), xc)), 
                   dim8_video( scale8( scale8( color.g, yc), xc)), 
                   dim8_video( scale8( scale8( color.b, yc), xc))
                   );
  CRGB c01 = CRGB( dim8_video( scale8( scale8( color.r, ye), xc)), 
                   dim8_video( scale8( scale8( color.g, ye), xc)), 
                   dim8_video( scale8( scale8( color.b, ye), xc))
                   );

  CRGB c10 = CRGB( dim8_video( scale8( scale8( color.r, yc), xe)), 
                   dim8_video( scale8( scale8( color.g, yc), xe)), 
                   dim8_video( scale8( scale8( color.b, yc), xe))
                   );
  CRGB c11 = CRGB( dim8_video( scale8( scale8( color.r, ye), xe)), 
                   dim8_video( scale8( scale8( color.g, ye), xe)), 
                   dim8_video( scale8( scale8( color.b, ye), xe))
                   );

  leds[XY(ix, iy)] += c00;
  leds[XY(ix, iy + 1)] += c01;
  leds[XY(ix + 1, iy)] += c10;
  leds[XY(ix + 1, iy + 1)] += c11;
}

void Dot::Move() {
  saccum78 oyv = yv;
  
  if( !show) return;
  yv -= gGravity;
  xv = scale15by8_local( xv, gDrag);    
  yv = scale15by8_local( yv, gDrag);

  if( theType == SPARK) {
    xv = scale15by8_local( xv, gDrag);    
    yv = scale15by8_local( yv, gDrag);
    color.nscale8( 255);
    if( !color) {
      show = 0;
    }
  }

  // if we'd hit the ground, bounce
  if( yv < 0 && (y < (-yv)) ) {
    if( theType == SPARK ) {
      show = 0;
    } else {
      yv = -yv;
      yv = scale15by8_local( yv, gBounce);
      if( yv < 500 ) {
        show = 0;
      }
    }
  }
  
  if( (yv < -300) /* && (!(oyv < 0))*/ ) {
    // pinnacle
    if( theType == SHELL ) {

      if( (y > (uint16_t)(0x8000)) /*&& (random8() < 64)*/) {
        // boom = Set all leds to the given color
        LEDS.showColor( CRGB( 91, 91, 91 ) );	// 91 * (140/255) = 50 white max pour 1A
        FastLED.delay(15);
        LEDS.showColor( CRGB::Black);
      }

      show = 0;

      gSkyburst = 1;
      gBurstx = x;
      gBursty = y;
      gBurstxv = xv;
      gBurstyv = yv;
      gBurstcolor = color;        
    }
  }
  if( theType == SPARK) {
    if( ((xv >  0) && (x > xv)) ||
        ((xv < 0 ) && (x < (0xFFFF + xv))) )  {
      x += xv;
    } else {
      show = 0;
    }
  } else {
    x += xv;
  }
  y += yv;
  
}
  
void Dot::GroundLaunch() {
  yv = 600 + random16(300 + (25 * kMatrixHeight));
  xv = (int16_t)random16(600) - (int16_t)300;
  y = 0;
  x = 0x8000;
  hsv2rgb_rainbow( CHSV( random8(), 240, 200), color);
  show = 1;
}

void Dot::Skyburst( accum88 basex, accum88 basey, saccum78 basedv, CRGB& basecolor) {
  yv = (int16_t)0 + (int16_t)random16(1500) - (int16_t)500;
  xv = basedv + (int16_t)random16(2000) - (int16_t)1000;
  y = basey;
  x = basex;
  color = basecolor;
  color *= 4;
  theType = SPARK;
  show = 1;
}
