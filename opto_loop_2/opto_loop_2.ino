/*
The opto-loop project : phase 1 : open loop tuning

Actuator : a LED connected to D11 (a PWM-capable digital pin)
  - negative terminal to ground
  - positive terminal to D11 via a 470 Ohm resistor
Sensor : a LDR connected to A5 (analog input)
  - terminal 1 to +5V
  - terminal 2 to A5, with a pulldown resistor to GND (typ more than 10k)

Manual control : a potentiometer or joystick connected to A1

Output to plotter :
  - the set point (from the potentiometer)
  - the actuator signal (driving the LED through PWM)
  - the sensor signal (from the ADC)

*/

// the setup routine runs once when you press reset:
void setup() {

// initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  Serial.println("SetPoint Actuator Sensor PlotRef");
}

int integrator = 0;
const int LOG_TAU = 3;

// the loop routine runs over and over again forever:
void loop() {
// read the inputs
  // int valx = analogRead(A0);
  int valy = analogRead(A1);
  int valset = map( valy, 0, 1023, 0, 600 );
  int valLDR = analogRead(A5);
// compute the control
  int error = valset - valLDR;
  integrator += ( error >> LOG_TAU );
  integrator = constrain( integrator, 0, 255 ); 
  int PW = (error>>1) + integrator; 
// drive the LED @ 11 with valX , range 0 - 255
  PW = constrain( PW, 0, 255 );
  analogWrite( 11, PW );
  
  static int count = 0;
  // print out the signals
  // print also a reference pulse to keep the Y autoscale quiet
  // Note : the IDE plotter X range is 500 samples (fixed)
  Serial.print( valset );   Serial.print("  ");
  Serial.print( PW << 2 );  Serial.print("  ");
  Serial.print( valLDR );   Serial.print("  ");
  Serial.println( count?0:1023 );
  count = count + 1;
  if  ( count > 495 ) count = 0;  // a little less than the full plotter X range
  delay(20);        // approximate servo loop period (ms)
}
