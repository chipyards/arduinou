/* template for beep-without-delay based on FSM
 *  
 */
#define AUDIOPIN 10

// main input: number of beeps
int x=0;

// configuration
int on_time = 130;
int off_time = 130;
int interval = 4000;

// state variables
int ibeep = 0; // beep_index
bool sound = false; // sound or silence
long next_date = 0;

// beep FSM body
void beep() {
  if  ( millis() > next_date ) {
      if  ( sound ) {
          sound = false;
          analogWrite( AUDIOPIN, 0 );
          next_date += off_time;
      } else {
          if  ( ibeep >= x ) {
              next_date += interval;
              ibeep = 0;
          } else {
              sound = true;
              analogWrite( AUDIOPIN, 85 );
              next_date += on_time;
              ibeep++;
          }

      }
      
  }

}
void setup() {
Serial.begin(115200);
next_date = millis() + interval;
}

void loop() {
    beep();
    if  ( Serial.available() ) {
        char c = Serial.read();
        if  ( c == 'x' ) {
            x = Serial.parseInt();
            Serial.println(x);
        }
        if  ( c == 'n' ) Serial.println(next_date);
    }
}
