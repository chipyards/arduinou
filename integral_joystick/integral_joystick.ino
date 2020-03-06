/*
This project uses a joystick to adjust 2 parameters paramX and paramY.

Thanks to 2 integrators, each parameter keeps its value when the stick
is returned to the center position.
Deadbands are provided to keep the integrators stable.
Each time a paramer reaches a limit, the arduino LED is lit.

Connect joystick X connected to A0, joystick Y to A1.

Joystick current position and both parameters are output to the serial plotter
*/
const int loop_frequency = 50;  // should be a divider of 1000
const int loop_delay = 1000 / loop_frequency; // approximately

int centerX, centerY;
int paramX = 0, paramY = 0;

const int deadspan = 100;
const int paramin = -512;   // to be adjusted according to application needs
const int paramax = 512;
// integration rate : at each iteration of the main loop,
// the joystick deviation divided by this number is added to the parameter.
// increase this number if variation is too fast
// from 0 to paramax, it takes (rate_div*paramax)/(loop_frequency*joydev) seconds
// example : with main loop at 50 Hz (20 ms), rate_div = 32, paramax = 512, joydev = 412 :
// it takes 0.79 s to reach paramax from 0
const int rate_div = 64;


// the setup routine runs once when you press reset:
void setup() {
  pinMode( 13, OUTPUT );
  centerX = analogRead(A0);
  centerY = analogRead(A1);
// initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  // plot labels (Note : keep the AVR reset while starting the plotter)
  Serial.println("joyX joyY paramX paramY refPulse");
  centerX = analogRead(A0);
  centerY = analogRead(A1);
}

// a function to remove the deadband
int killdead( int v ) {
  if  ( v >= 0 ) {
      v -= deadspan;
      if  ( v < 0 ) v = 0;  
      }
  else {
      v += deadspan;
      if  ( v > 0 ) v = 0;  
      }
return v;
}

// the loop routine runs over and over again forever:
void loop() {
// read the inputs
  int joyX = analogRead(A0) - centerX;
  int joyY = analogRead(A1) - centerY;
  joyX = killdead( joyX );
  joyY = killdead( joyY );  
// integrate
  paramX += ( joyX / rate_div );
  paramY += ( joyY / rate_div );
// bound
  paramX = constrain( paramX, paramin, paramax );
  paramY = constrain( paramY, paramin, paramax );
  if  ( ( paramX == paramin ) || ( paramX == paramax ) ||
        ( paramY == paramin ) || ( paramY == paramax )
      ) digitalWrite( 13, 1 );
  else digitalWrite( 13, 0 );
// plot  
  static int count = 0;
  // Note : the IDE plotter X range is 500 samples (fixed)
  Serial.print( joyX );   Serial.print("  ");
  Serial.print( joyY );  Serial.print("  ");
  Serial.print( paramX );   Serial.print("  ");
  Serial.print( paramY );   Serial.print("  ");
  // print also a reference pulse to keep the Y autoscale quiet
  if  ( count == 0 )
      Serial.println( -1023 );
  else if ( count == 1 )
      Serial.println( 1023 );
  else
      Serial.println( 0 );
  count = count + 1;
  if  ( count > 495 ) count = 0;  // a little less than the full plotter X range
  delay(loop_delay);        // approximate servo loop period (ms)
}
