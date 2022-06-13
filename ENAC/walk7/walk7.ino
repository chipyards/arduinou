/*
 * walkGPS stage 7 : regression : un template de FSM base sur la demo standalone de walk6
 * 
 */

// variables for remote control
bool verb1 = false;
bool verb2 = false;

const char * demotext =
"$GPGGA,133449.000,4332.7181,N,00129.2813,E,1,7,1.32,136.0,M,50.4,M,,*5D\n"
"$GPGSA,A,3,08,21,10,32,14,27,23,,,,,,1.59,1.32,0.88*05\n"
"$GPRMC,133449.000,A,4332.7181,N,00129.2813,E,0.22,213.72,130622,,,A*6C\n"
"$GPVTG,213.72,T,,M,0.22,N,0.40,K,A*3C\n"
"$GPGGA,133453.000,4332.7180,N,00129.2811,E,1,7,1.32,136.0,M,50.4,M,,*55\n"
"$GPGSA,A,3,08,21,10,32,14,27,23,,,,,,1.59,1.32,0.88*05\n"
"$GPGSV,3,1,12,08,85,299,27,21,62,295,27,27,52,128,22,10,38,050,44*70\n"
"$GPGSV,3,2,12,01,33,272,17,32,26,104,42,22,17,125,17,14,12,322,15*7A\n"
"$GPGSV,3,3,12,16,10,177,19,44,09,108,,23,08,042,20,03,07,213,17*7A\n"
"$GPRMC,133453.000,A,4332.7180,N,00129.2811,E,0.11,118.87,130622,,,A*66\n"
"$GPVTG,118.87,T,,M,0.11,N,0.21,K,A*39\n";

void fsm( char c );

void demo() {
  Serial.println("DEMO");
  int i = 0; char c;
  // this loop replaces the real-time loop containing the soft serial read() method
  while ( c = demotext[i++] ) {
          if  ( verb1 ) Serial.print(c);
          fsm( c );
      }
}


// variables describing the current state of the FSM
int step = 0;
int sentence = 0; // 1 = $GPGGA, 2 = $GPVTG
int ifield = 0;   // current field
int ichar = 0;    // index of current char in fbuf[]
char fbuf[20];    // buffer for current field

// variables for storing the extracted values 
double kspeed = 0;
double useconds = 0;
int sats = 0;

void conclusion();  // to be called by the FSM when sentence burst is finished

#define LF 10   // linefeed character
/* FSM steps :
  0 : waiting for '$', any other char ignored
  1, 2, 3 : waiting for G, P, (G or V), any other char sends back to zero
  10, 11 : waiting for G, A (end of $GPGGA)
  20, 21 : waiting for T, G (end of $GPVTG)
  100 : waiting for ',', '*', LF, any other char is stored in the current field buffer
  LF sends back to 0
 */
void fsm( char c ) {
switch ( step ) {
  // sentence prefix
  case 0 : if ( c == '$' ) { step=1; sentence = 0; ifield = 0; ichar = 0; } break;   
  case 1 : if ( c == 'G' ) step=2; else step=0; break;   
  ...
  // remainder of the sentence
  case 100 :
    if  ( c == LF ) {
        // process the end of sentence
        step=0;
        ...
    } else {
        if  ( ( c == ',' ) || ( c == '*' ) ) {
            // process the end of field
            fbuf[ichar] = 0;  // string terminator
            // select each one of the relevant fields, according to sequence and ifield
            // extract numerical values using atoi() or strtod()
            ...
            ifield++; ichar = 0;  // prepare for next field
        } else {
            // process an ordinary char
            if  ( ichar < ( sizeof(fbuf) - 1 ) ) {
                fbuf[ichar++] = c;  // store in the field buffer
            }
        }
    }
    break;
  } // switch step
}

void conclusion() {
  Serial.print(" usec="); Serial.print(useconds);
  Serial.print(" sats="); Serial.print(sats);
  Serial.print(" kspeed="); Serial.print(kspeed);
  Serial.println();
}

void setup()  {
    Serial.begin(115200);
    Serial.println("HELLO");
    demo();
}

void loop() {
     if  ( Serial.available() ) {
        char c = Serial.read();
        if  ( c == 'n' ) { verb1 = false; verb2 = false; }
        if  ( c == 'v' ) verb1 = true;
        if  ( c == 'w' ) verb2 = true;
        if  ( c < ' ' ) demo();
     }
}
