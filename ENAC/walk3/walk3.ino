/*
 * walkGPS stage 3 : emit beeps just after receiving the last sentence of the burst (supposed to be $GPVTG)
 * to test the behaviour of tone() undisturbed.
 * Uses variable x
 * Conclusion : tone is undisturbed during about half a second ==> the disturbance was caused by mysoft.read()
 * but is we want more than 0.5s we cannot...
 * Note : this program has the "bogus frac fsm"
 */
#include <SoftwareSerial.h>
#define rxPin 7
#define txPin 8
#define AUDIOPIN 11
#define LF 10   // linefeed character
// Set up a new SoftwareSerial object
SoftwareSerial mysoft = SoftwareSerial(rxPin, txPin);

// variables for remote control
int period = 0;
int x = 0;
int ftone = 200;
bool verb1 = false;
bool verb2 = false;

int step = 0;
int sentence = 0; // 1 = $GPGGA, 2 = $GPVTG
int ifield = 0;  // current field
bool dot = false;    // a dot was seen
long val = 0;
long frac = 0;

int kspeed = 0;
int fspeed = 0;
long utime = 0;
int sats = 0;

void beeps( char cnt );

/* steps :
 *    0 : waiting for $
 */
void nmea_proc( char c ) {
switch ( step ) {
  // sentence prefix
  case 0 : if ( c == '$' ) { step=1; sentence = 0; ifield = 0; dot = false; val = 0; frac = 0; } break;   
  case 1 : if ( c == 'G' ) step=2; else step=0; break;   
  case 2 : if ( c == 'P' ) step=3; else step=0; break;   
  case 3 : if ( c == 'G' ) step=10;
           else if ( c == 'V' ) step=20; else step=0; break;  
  case 10 : if ( c == 'G' ) step=11; else step=0; break;  // $GPGGA for time and number of satellites 
  case 11 : if ( c == 'A' ) {
            step=100; sentence = 1;
            if ( verb2 ) { Serial.println(); Serial.print("$GPGGA "); }
            if  ( period ) delay( period ); // robustness test
            } else step=0; break;
  case 20 : if ( c == 'T' ) step=21; else step=0; break;  // $GPVTG for heading & speed
  case 21 : if ( c == 'G' ) {
            step=100; sentence = 2;
            if ( verb2 ) { Serial.println(); Serial.print("$GPVTG "); }
            if  ( period ) delay( period ); // robustness test
            } else step=0; break;
  case 100 : if ( ( c >= '0' ) && ( c <= '9' ) ) {
                //if  ( dot )
                //    frac = frac * 10 + ( c - '0' ); // BUG makes 0.1 instead of 0.01
                if  ( !dot )
                    val = val * 10 + ( c - '0' );
             }
       else  if ( c == '.' ) dot = true; 
       else  if ( ( c == ',' ) || ( c == '*' ) ) {
                //Serial.print("sentence="); Serial.print(sentence); Serial.print(" ");
                //Serial.print("ifield="); Serial.print(ifield); Serial.print(" ");
                //Serial.println();
                if  ( sentence == 1 ) {
                    switch ( ifield ) {
                      case 1 : utime = val;
                            if ( verb2 ) { Serial.print(" utime="); Serial.print(utime); }
                            break;
                      case 7 : sats = val;
                            if ( verb2 ) { Serial.print(" sats="); Serial.print(sats); }
                            break;
                    }
                }
                if  ( sentence == 2 ) {
                    switch ( ifield ) {
                      case 7 : kspeed = val; fspeed = frac;
                            if  ( verb2 ) {
                                Serial.print(" kspeed="); Serial.print(kspeed);
                                Serial.print("."); Serial.print(fspeed);
                            }
                            break;
                    }
                }    
                ifield++; dot=false; val=0; frac=0;
             }
      else   if ( c == LF ) {
                step=0;
                if ( verb2 ) Serial.println();
                if ( sentence == 2 ) {
                   if ( verb1 ) Serial.println("-----");  // burst separation
                   // here we are in the interval between burst, do something that takes time
                   beeps( x );
                }
             }
      break;
  } // switch step
// Serial.print(' '); Serial.print(step); Serial.print(' ');
}

void setup()  {
    Serial.begin(115200);
    Serial.println( "Hello GPS" );
    // Define pin modes for TX and RX
    pinMode(rxPin, INPUT);
    pinMode(txPin, OUTPUT);

    // Set the baud rate for the SoftwareSerial object
    mysoft.begin(9600);
    mysoft.listen();

//  Serial.print("="); Serial.println();
    Serial.print("sizeof(bool)="); Serial.println(sizeof(bool));
    Serial.print("sizeof(short)="); Serial.println(sizeof(short));
    Serial.print("sizeof(int)="); Serial.println(sizeof(int));
    Serial.print("sizeof(long)="); Serial.println(sizeof(long));
    // testing strtod
    long m1 = millis(); double d = strtod( "3.0456", NULL ); long m2 = millis();
    Serial.println(d, 13); Serial.println( m2 - m1 );
    // testing snprintf MARCHE PO BIEN !!?!
    // char tbuf[64];
    // snprintf( tbuf, sizeof(tbuf), "%g %d", d, m2 - m1 ); Serial.println(tbuf);
}

void beeps( char N ) {
    int f = ftone;
    for ( int i = 0; i < N; i++  ) {
        if  ( i ) delay(110);
        tone( AUDIOPIN, f, 100 ); // non-blocking function !!
        f *= 100; f /= 106;
    }
    
    /*
    for ( int i = 0; i < N; i++  ) {
        if  ( i ) delay(130);
        analogWrite( AUDIOPIN, 85 );
        delay(130);
        analogWrite( AUDIOPIN, 0 );
    }
    */ 
}

void loop() {
    static long tloop = 0;
    
    
     while  ( mysoft.available() ) {
          char c = mysoft.read();
          if ( verb1 ) Serial.print( c );
          nmea_proc( c );
     }
     
     if  ( Serial.available() ) {
        char c = Serial.read();
        if  ( c == 'n' ) { verb1 = false; verb2 = false; }
        if  ( c == 'v' ) verb1 = true;
        if  ( c == 'w' ) verb2 = true;
        if  ( c == 'T' ) period = Serial.parseInt();
        if  ( c == 'x' ) x = Serial.parseInt();
        if  ( c == 'f' ) ftone = Serial.parseInt();
     }


}
