/* 
This program demonstrates :
    - using a PWM output with variable duty cycle
    - adjusting a parameter form the host computer via serial communication
*/

void setup() {
  Serial.begin(115200);
}

void loop() {
  static int pw = 0;    // pulse width (0 to 255)
  if ( Serial.available() )
    {
    char c = Serial.read();
    Serial.println( c );
    switch( c ) {
      case '+' : pw += 1;  break;
      case '-' : pw -= 1;  break;
      case '>' : pw += 10;  break;
      case '<' : pw -= 10;  break;
      }
    // it is important not to led pw overflow,
    // pw was declared as an int in order to make these tests possible
    if ( pw > 255 ) pw = 255;
    if ( pw < 0 )   pw = 0;
    analogWrite( 11, pw );   // sets the PWM pulse width at pin 11
    }
}
