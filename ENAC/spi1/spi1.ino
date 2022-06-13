#include <SPI.h>
/*
 * SCK  = 13
 * MISO = 12
 * MOSI = 11
 * SS = wherever you want
 */
void setup() {
  Serial.begin(115200);
  SPI.begin();
  SPI.beginTransaction( SPISettings( 125000, MSBFIRST, SPI_MODE0 ) );
}

void loop() {
  static unsigned int rx, tx;
  if  ( Serial.available() ) {
      char c = Serial.read();
      if ( c == 't' ) {
          tx = Serial.parseInt();
          rx = SPI.transfer16(tx);
          Serial.print("tx="); Serial.print(tx);
          Serial.print(" rx="); Serial.print(rx);
          Serial.println();
      }
  } 
}
