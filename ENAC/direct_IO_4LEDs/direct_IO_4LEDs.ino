/*
This program demonstrates :
    - direct access to peripheral registers, here registers PORTB and DDRB of GPIO
    - bit level operations : bitwise logic and shift
    - interpreting single-char commands from the serial connexion with the host computer
The program is used to control 4 LEDs connected to digital pins 8, 9, 10, 11 (arduino numbers)
which are pins PB0, PB1, PB2, PB3, the 4 LSBs of PORT B.
The program MUST NOT interfere with the other bits of PORT B which may be used for something else.
*/

void setup() {
Serial.begin(115200);
PORTB &= ~0x0F;   // output zero on the 4 LSB's (same as &= 0xF0)
DDRB  |=  0x0F;   // configure the 4 LSBs as output
}

void loop() {
  // put your main code here, to run repeatedly:
if ( Serial.available() )
  {
    char c = Serial.read();
    unsigned char tmp;
    Serial.println( c );
    switch( c ) {
      case 'a' : PORTB |=  0x0F; break;      // all ON
      case 'n' : PORTB &= ~0x0F; break;      // all OFF
      case 'b' : PORTB |= 1; break;          // blue ON   ( 1 = 0b0001 = ( 1 << 0 ) )
      case 'g' : PORTB |= 0b0010; break;     // green ON  ( 2 = 0b0010 = ( 1 << 1 ) )
      case 'y' : PORTB |= 4; break;          // yellow ON ( 4 = 0b0100 = ( 1 << 2 ) )
      case 'r' : PORTB |= ( 1 << 3 ); break; // red ON    ( 8 = 0b1000 = ( 1 << 3 ) )
      case '<' :                             // shift towards blue
              // PORTB <<= 1 would do the shift Ok, but would leak on the 4 MSBs
              tmp = PORTB;
              PORTB = ( tmp & 0xF0 ) | ( ( tmp << 1 ) & 0x0F );
              break;
      case '>' :                             // shift towards red
              // PORTB >>= 1 would do the shift Ok, but would leak from the 4 MSBs
              tmp = PORTB;
              PORTB = ( tmp & 0xF0 ) | ( ( tmp >> 1 ) & 0x07 );
              break;
      }
  }
}
