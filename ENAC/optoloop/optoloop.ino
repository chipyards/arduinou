#define BUTPIN 2
#define LEDPIN 13
#define ADCPIN 0
#define ADCPWM 4
#define PWMPIN 9

int period = 20;
int pgain = 1;
int igain = 4;
bool close = false;

void setup() {
pinMode( BUTPIN, INPUT_PULLUP );
pinMode( LEDPIN, OUTPUT );
Serial.begin( 115200 );
Serial.println("min max potentiometer phototransistor");
}

void loop() {
// variables for timing measurement
static long start_time;
static long loop_count;

// precision loop control
static long tloop = 0;
if  ( tloop == 0 )
    tloop = millis();
tloop += period;




  // analog processing
  int pot = analogRead( ADCPIN );
  int optical_return = 1023 - analogRead( ADCPWM );
  static int integrator = 0;
  int diff;

  // servo loop control
  if  ( close ) {
      // closed loop operation
      diff = pot - optical_return;
      integrator += diff;
      if  ( integrator > 10000 ) integrator = 10000; 
      if  ( integrator < -10000 ) integrator = -10000; 
      diff *= pgain;
      diff += ( integrator / igain );
      diff += ( pot/4 );
      if  ( diff > 255 ) diff = 255;
      if  ( diff < 0 ) diff = 0;
      analogWrite( PWMPIN, diff );
  } else {
      // open loop operation
      analogWrite( PWMPIN, pot / 4 );
  }



  
  // toggle button
  static bool plotting = true;
  static bool oldbutton = true;
  bool newbutton = digitalRead( BUTPIN );
  if ( newbutton != oldbutton )  {
      if ( newbutton == false ) {
      //   plotting = ! plotting;
        if ( plotting ) {
              plotting = false;
              long total_time = millis() - start_time;
              float ftotal = (float)total_time;
              float fperiod = ftotal / (float)loop_count;
              Serial.print("actual_period=");
              Serial.print(fperiod);
              Serial.print("_vs_");
              Serial.println(period);
            } else {
             plotting = true;
             start_time = millis();
             loop_count = 0;
         }
      digitalWrite( LEDPIN, plotting );    
      }
    oldbutton = newbutton;
    }

// remote control from the PC
if  ( Serial.available() ) {
  char c = Serial.read();
  if  ( c == 'T' )  { period = Serial.parseInt(); }          
  if  ( c == 'g' )  { pgain = Serial.parseInt(); }          
  if  ( c == 'i' )  { igain = Serial.parseInt(); }
  if  ( c == 'c' )  { close = true; }
  if  ( c == 'o' )  { close = false; }
}
  
    // plotting process
    if  ( plotting ) {
      Serial.print( "0 1023 " );
      Serial.print( pot );
      Serial.print( " " );  Serial.print( optical_return );
      // Serial.print( " " );  Serial.print( diff );
      Serial.println();
    }
    loop_count++;

 // precision loop control
 //   delay(period);
 while ( millis()< tloop ) {}
}
