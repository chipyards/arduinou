#define BUTPIN 2
#define LEDPIN 13
#define ADCPIN 0

int period = 20;

void setup() {
pinMode( BUTPIN, INPUT_PULLUP );
pinMode( LEDPIN, OUTPUT );
Serial.begin( 115200 );
Serial.println("min max analog_in");
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

  // toggle button
  static bool plotting = false;
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
              //Serial.print("total="); 
              //Serial.print( total_time );
              //Serial.print("ms_count=");
              //Serial.println(loop_count);
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
  if  ( Serial.read() == 'T' )  {
      period = Serial.parseInt();
  }          
}
  
    // plotting process
    if  ( plotting ) {
      Serial.print( "0 1023 " );
      Serial.println( pot );
    }
    loop_count++;

 // precision loop control
 //   delay(period);
 while ( millis()< tloop ) {}
}
