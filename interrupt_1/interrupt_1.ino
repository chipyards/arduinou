
// timer interrupt, based on the TimerInterrupt library by Khoi Hoang
// il mesure la periode d'interrupt du timer 1 en utilisant milli() (timer 0)
// https://github.com/khoih-prog/TimerInterrupt

#define USE_TIMER_1     true    // doit etre avant #include <TimerInterrupt.h>
#include <TimerInterrupt.h>

#define DISP_PERIOD 1000

volatile unsigned long ticks = 0;
unsigned long next_ms = 0;
unsigned int secs = 0;

void my_timer_irtn(void) {
ticks += 1;
}

void setup() {
Serial.begin( 115200 );
Serial.println("starting");
// Init timer ITimer1
ITimer1.init();
// ITimer1.attachInterruptInterval( 1L, my_timer_irtn );  // tested @ 1ms
ITimer1.attachInterrupt( float(50000.0), my_timer_irtn ); // tested @ 20uS
}

void loop() {
if  ( Serial.available() ) {
    char c = Serial.read();
    // ITimer1.setFrequency( 100, my_timer_irtn );
    ticks = 0; next_ms = millis() - 1; secs = 0;
    }
if  ( millis() > next_ms ) {
    Serial.print( secs );  Serial.print("\t");
    Serial.print( ticks ); Serial.print("\t");
    if  ( secs < 1 ) Serial.println();
    else Serial.println( ticks / secs );
    next_ms += DISP_PERIOD;
    ++secs;
    }
}
