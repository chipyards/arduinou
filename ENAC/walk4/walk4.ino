/*
 * walkGPS stage 4 : fsm using atoi and strtod on the fly instead of convertir to decimal 'by hand'
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
int ifield = 0;   // current field
int ichar = 0;
char fbuf[40];    // buffer for current field

double kspeed = 0;
double useconds = 0;
int sats = 0;

void conclusion();

/* steps :
 *    0 : waiting for $
 */
void fsm( char c ) {
switch ( step ) {
  // sentence prefix
  case 0 : if ( c == '$' ) { step=1; sentence = 0; ifield = 0; ichar = 0; } break;   
  case 1 : if ( c == 'G' ) step=2; else step=0; break;   
  case 2 : if ( c == 'P' ) step=3; else step=0; break;   
  case 3 : if ( c == 'G' ) step=10;
           else if ( c == 'V' ) step=20; else step=0; break;  
  case 10 : if ( c == 'G' ) step=11; else step=0; break;  // $GPGGA for time and number of satellites 
  case 11 : if ( c == 'A' ) {
            step=100; sentence = 1;
            if ( verb2 ) { Serial.println(); Serial.print("$GPGGA "); }
            } else step=0; break;
  case 20 : if ( c == 'T' ) step=21; else step=0; break;  // $GPVTG for heading & speed
  case 21 : if ( c == 'G' ) {
            step=100; sentence = 2;
            if ( verb2 ) { Serial.println(); Serial.print("$GPVTG "); }
            } else step=0; break;
  case 100 : if ( c == LF ) {
                step=0;
                if ( verb2 ) Serial.println();
                if ( sentence == 2 ) {
                   if ( verb1 ) Serial.println("-----");  // burst separation
                   // here we are in the interval between burst, do something that takes time
                   conclusion();
                }
             } else 
                if ( ( c == ',' ) || ( c == '*' ) ) {
                fbuf[ichar] = 0;  // string terminator
                if  ( ( sentence == 1 ) && ( ifield == 1 ) ) {
                            useconds = strtod( fbuf+4, NULL ); 
                            if ( verb2 ) { Serial.print(" usec="); Serial.print(useconds); }
                }
                if  ( ( sentence == 1 ) && ( ifield == 7 ) ) {
                            sats = atoi( fbuf );
                            if ( verb2 ) { Serial.print(" sats="); Serial.print(sats); }
                }
                if  ( ( sentence == 2 ) && ( ifield == 7 ) ) {
                            kspeed = strtod( fbuf , NULL );
                            if  ( verb2 ) { Serial.print(" kspeed="); Serial.print(kspeed); }
                }    
                ifield++; ichar = 0;
             } else {
                if  ( ichar < ( sizeof(fbuf) - 1 ) ) {
                    fbuf[ichar++] = c;
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
}

void beeps( char N ) {
    for ( int i = 0; i < N; i++  ) {
        if  ( i ) delay(130);
        analogWrite( AUDIOPIN, 85 );
        delay(130);
        analogWrite( AUDIOPIN, 0 );
    }
}

void conclusion() {
  static long beeptime = 0;
  if  ( beeptime == 0 ) beeptime = millis();  
  Serial.print(" usec="); Serial.print(useconds);
  Serial.print(" sats="); Serial.print(sats);
  Serial.print(" kspeed="); Serial.print(kspeed);
  Serial.println();
  if  ( millis() > beeptime ) {
      beeps( (int)kspeed );
      // beeps( x );
      beeptime += 4000;
  }
}

void loop() {
    
     if  ( mysoft.available() ) {
          char c = mysoft.read();
          if ( verb1 ) Serial.print( c );
          fsm( c );
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
