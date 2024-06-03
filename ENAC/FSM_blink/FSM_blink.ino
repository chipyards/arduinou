/* 
This program demonstrates 3 tasks :
    - create a periodic sequence of events according to a time schedule
    - memorizing a number of button presses
    - handling commands from the host computer via the serial line

The periodic sequence consists of blinking N times the LED connected at pin 13,
then pause until the end of the period.

Each button press should increase N by one, up to 4, then reset it to zero

The serial process just aknowledges each 1-char command received from the PC

Several versions of the solution are proposed for comparison

 */

// variable shared by 2 tasks: we choose to make it global
int blink_cnt = 2;  // from 0 to 4

void setup() {
pinMode( 11, OUTPUT );
pinMode( 13, OUTPUT );
pinMode( 4, INPUT_PULLUP );	// pullup, for button connected to ground
Serial.begin( 115200 );
// displaying the sizes of number types (sizeof() returns sizes in bytes)
Serial.println("sizes: ");
Serial.print( sizeof(char) ); Serial.print(" ");
Serial.print( sizeof(int) ); Serial.print(" ");
Serial.println( sizeof(long int) ); 
}

void serial_process() {
     int c = Serial.read();
   if ( c > 0 ) {
      Serial.print("received "); Serial.println( (char)c );
      }
}

void button_process( )
{
static bool old_button_state = true;
bool new_button_state;
bool change;		// change is detected
bool change_to_pressed;	// change detected from up to pressed
new_button_state = !digitalRead(4);	// active-low button	
change = ( new_button_state != old_button_state );
change_to_pressed = ( change && new_button_state );
digitalWrite( 11, change_to_pressed );  // debug display
  if  ( change_to_pressed ) {
      if ( blink_cnt >= 4 )
           blink_cnt = 0;
      else blink_cnt++;
      Serial.println( blink_cnt );
  }
old_button_state = new_button_state;
}

/*
// Basic solution for the LED process, based on nested loops
// this solution works, but has a strong disadvantage:
// the other tasks and the time delays must be nested
// in the inner loops, making the maintenance of the code impractical 

void loop() {
   int c = Serial.read();
   if ( c > 0 ) {
      Serial.print("received "); Serial.println( (char)c );
   }
   // blink the LED blink_cnt times
   for ( int i = 0; i < blink_cnt; i++ ) {
       button_process( );
       digitalWrite( 13, 1 );
       delay( 100 );
       button_process( );
       digitalWrite( 13, 0 );
       delay( 100 );
   }
   // pause until the end of the period (1000 ms)
   for ( int i = 0; i < ( 10 - 2 * blink_cnt ); i++ ) {
       button_process( );
       delay( 100 );
   }
}
*/

// First FSM (Finite States Machine) solution for the LED:
// no more nested loops, instead 3 state variables are used 
// to remember where the process is currently in the sequence.
// This solution allows to separate the tasks, and avoid any duplication
// of code in the main loop (see below) 
void LED_process_v1() {
    static int i_blink = 0;	// count the blinks, pause if equal to blink_cnt
    static int i_wait = 0;	// count the pause time
    static int led_on = 0;	// remember the LED state
    if  ( i_blink < blink_cnt )
        {	// blinks
        if ( led_on == 0 ) {
             digitalWrite( 13, 1 ); led_on = 1;
             }
        else {
             digitalWrite( 13, 0 ); led_on = 0; 
             i_blink++;
             }
        }
    else
        {	// pause
          if ( i_wait > ( 10 - 2 * blink_cnt ) ) {
              i_blink = i_wait = 0; // prepare new period
              }
          else i_wait++;
        }
}

// Simplified FSM solution : only one state variable:
// ticks counts the time in main loop duration units units
// (100ms in the basic solution)
void LED_process_v2() {
  static int ticks = 0;	// from 0 to 9
  if ( ticks < ( blink_cnt * 2 ) )
      digitalWrite( 13, (ticks & 1)^1 );
  if ( ticks < 10 )
    ticks++;
  else ticks = 0;
}

// Interrupt based FSM solution : only one state variable:
// ticks, derived from time counted by the system timer interrupt
// The process timeline does not depend anymore on the main loop
// duration, provided that it is <= 100ms.
// this allows to run the main loop faster, providing quicker response for the other tasks.
void LED_process_v3() {
  // the modulo operator % is used to keep ticks in the [0,9] interval
  int ticks = ( millis() / 100 ) % 10;
  if ( ticks < ( blink_cnt * 2 ) )
      digitalWrite( 13, (ticks & 1)^1 );
}

// main loop
void loop() {
    serial_process();
    button_process( );
    LED_process_v3();
    // delay( 100 );
    delay( 10 );
}
