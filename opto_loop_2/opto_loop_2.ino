/*
The opto-loop project : phase 1 : open loop tuning

Actuator : a LED connected to D11 (a PWM-capable digital pin)
  - negative terminal to ground
  - positive terminal to D11 via a 470 Ohm resistor
Sensor : a LDR connected to A5 (analog input)
  - terminal 1 to +5V
  - terminal 2 to A5, with a pulldown resistor to GND (typ more than 10k)

Manual control : a joystick connected to A1, with integral control and deadband

Output to plotter :
  - the set point (from the potentiometer)
  - the actuator signal (driving the LED through PWM)
  - the sensor signal (from the ADC)

*/

const int loop_frequency = 40;  // should be a divider of 1000
const int loop_delay = 1000 / loop_frequency; // approximately

int centerX, centerY;
int paramX = 1, paramY = 1;

const int deadspan = 100;
const int paramin = 0;   // to be adjusted according to application needs
const int paramax = 700;
// integration rate :
// from 0 to paramax, it takes (rate_div*paramax)/(loop_frequency*joydev) seconds
const int rate_div = 64;


// the setup routine runs once when you press reset:
void setup() {
  pinMode( 13, OUTPUT );
  centerX = analogRead(A0);
  centerY = analogRead(A1);

// initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  // plot labels (Note : keep the AVR reset while starting the plotter)
  Serial.println("SetPoint Actuator Sensor PlotRef");
  centerX = analogRead(A0);
  centerY = analogRead(A1);
}

// a function to remove the joystick deadband
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

// the integrator for the servo PI algorithm
int integrator = 0;
const int LOG_TAU = 3;

// the loop routine runs over and over again forever:
void loop() {
// read the joystick inputs
  // int joyX = analogRead(A0) - centerX;
  int joyY = analogRead(A1) - centerY;
  // joyX = killdead( joyX );
  joyY = killdead( joyY );  
// integrate
  // paramX += ( joyX / rate_div );
  paramY += ( joyY / rate_div );
// bound
  // paramX = constrain( paramX, paramin, paramax );
  paramY = constrain( paramY, paramin, paramax );
  if  ( ( paramX == paramin ) || ( paramX == paramax ) ||
        ( paramY == paramin ) || ( paramY == paramax )
      ) digitalWrite( 13, 1 );
  else digitalWrite( 13, 0 );

// read the sensor inputs
  int valLDR = analogRead(A5);
  
// compute the control (the set value aka command is paramY)
  int error = paramY - valLDR;
  integrator += error;
  // integrator contents are bounded in order to provide quick recovery in case of saturation of the servo
  integrator = constrain( integrator, 0, ( 255 << LOG_TAU ) ); 
  int PW = ( error >> 2 ) + ( integrator >> LOG_TAU );  
// drive the LED @ 11 with valX , range 0 - 255
  PW = constrain( PW, 0, 255 );
  analogWrite( 11, PW );
  
  static int count = 0;
  // print out the signals
  // print also a reference pulse to keep the Y autoscale quiet
  // Note : the IDE plotter X range is 500 samples (fixed)
  Serial.print( paramY );   Serial.print("  ");
  Serial.print( PW << 2 );  Serial.print("  ");
  Serial.print( valLDR );   Serial.print("  ");
  Serial.println( count?0:1023 );
  count = count + 1;
  if  ( count > 495 ) count = 0;  // a little less than the full plotter X range
  delay(loop_delay);        // approximate servo loop period (ms)
}
