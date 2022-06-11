
#include <SoftwareSerial.h>
#define rxPin 7
#define txPin 8
#define AUDIOPIN 11

// Set up a new SoftwareSerial object
SoftwareSerial mysoft = SoftwareSerial(rxPin, txPin);
int period = 4000;
int x = 0;
int ftone = 200;
bool verbose = false;

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
  case 11 : if ( c == 'A' ) { step=100; sentence = 1; Serial.println(); Serial.print("$GPGGA"); } else step=0; break;
  case 20 : if ( c == 'T' ) step=21; else step=0; break;  // $GPVTG for heading & speed
  case 21 : if ( c == 'G' ) { step=100; sentence = 2; Serial.println(); Serial.print("$GPVTG"); } else step=0; break;
  case 100 : if ( ( c >= '0' ) && ( c <= '9' ) ) {
                if  ( dot )
                    frac = frac * 10 + ( c - '0' );
                else val = val * 10 + ( c - '0' );
             }
       else  if ( c == '.' ) dot = true; 
       else  if ( ( c == ',' ) || ( c == '*' ) ) {
                //Serial.print("sentence="); Serial.print(sentence); Serial.print(" ");
                //Serial.print("ifield="); Serial.print(ifield); Serial.print(" ");
                //Serial.println();
                if  ( sentence == 1 ) {
                    switch ( ifield ) {
                      case 1 : utime = val; Serial.print(" utime="); Serial.print(utime); x = utime % 10; break;
                      case 7 : sats = val;  Serial.print(" sats="); Serial.print(sats); break;
                    }
                }
                if  ( sentence == 2 ) {
                    switch ( ifield ) {
                      case 7 : kspeed = val; fspeed = frac;
                      Serial.print(" kspeed="); Serial.print(kspeed);
                      Serial.print("."); Serial.print(fspeed); break;
                    }
                }    
                ifield++; dot=false; val=0; frac=0;
             }
      else   if ( c < ' ' ) { step=0; Serial.println(); }
      break;
  }
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
}

void beeps( int N ) {
    /*
    for ( int i = 0; i < N; i++  ) {
        if  ( i ) delay(200);
        tone( AUDIOPIN, ftone, 100 ); // non-blocking function !!
    }
    */
    for ( int i = 0; i < N; i++  ) {
        if  ( i ) delay(130);
        analogWrite( AUDIOPIN, 85 );
        delay(130);
        analogWrite( AUDIOPIN, 0 );
       
    }
}

void loop() {
    static long tloop = 0;
    
    
     while  ( mysoft.available() ) {
          char c = mysoft.read();
          if ( verbose ) Serial.print( c );
          nmea_proc( c );
     }
     
     if  ( Serial.available() ) {
        char c = Serial.read();
        if  ( c == 'n' ) verbose = false;
        if  ( c == 'v' ) verbose = true;
        if  ( c == 'T' ) period = Serial.parseInt();
        if  ( c == 'x' ) x = Serial.parseInt();
        if  ( c == 'f' ) ftone = Serial.parseInt();
     }

     if  ( millis() > tloop ) {
         beeps( x ); tloop += period;
     }
}
