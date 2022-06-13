
/*
 * walkGPS stage 1 : determine the duration of the sentence burst
 * by means of the serial plotter
 * Conclusion : each burst is practically continuous
 * short bursts of about 250 ms, long burst of about 450 ms
 * The burst period fluctuates around 1 second
Typical sequence of 5 seconds :
$GPGGA,133449.000,4332.7181,N,00129.2813,E,1,7,1.32,136.0,M,50.4,M,,*5D
$GPGSA,A,3,08,21,10,32,14,27,23,,,,,,1.59,1.32,0.88*05
$GPRMC,133449.000,A,4332.7181,N,00129.2813,E,0.22,213.72,130622,,,A*6C
$GPVTG,213.72,T,,M,0.22,N,0.40,K,A*3C

$GPGGA,133450.000,4332.7181,N,00129.2813,E,1,7,1.32,136.0,M,50.4,M,,*55
$GPGSA,A,3,08,21,10,32,14,27,23,,,,,,1.59,1.32,0.88*05
$GPRMC,133450.000,A,4332.7181,N,00129.2813,E,0.23,238.06,130622,,,A*6F
$GPVTG,238.06,T,,M,0.23,N,0.43,K,A*34

$GPGGA,133451.000,4332.7180,N,00129.2812,E,1,7,1.32,136.0,M,50.4,M,,*54
$GPGSA,A,3,08,21,10,32,14,27,23,,,,,,1.59,1.32,0.88*05
$GPRMC,133451.000,A,4332.7180,N,00129.2812,E,0.15,227.53,130622,,,A*65
$GPVTG,227.53,T,,M,0.15,N,0.28,K,A*32

$GPGGA,133452.000,4332.7180,N,00129.2812,E,1,7,1.32,136.0,M,50.4,M,,*57
$GPGSA,A,3,08,21,10,32,14,27,23,,,,,,1.59,1.32,0.88*05
$GPRMC,133452.000,A,4332.7180,N,00129.2812,E,0.05,277.95,130622,,,A*68
$GPVTG,277.95,T,,M,0.05,N,0.10,K,A*37

$GPGGA,133453.000,4332.7180,N,00129.2811,E,1,7,1.32,136.0,M,50.4,M,,*55
$GPGSA,A,3,08,21,10,32,14,27,23,,,,,,1.59,1.32,0.88*05
$GPGSV,3,1,12,08,85,299,27,21,62,295,27,27,52,128,22,10,38,050,44*70
$GPGSV,3,2,12,01,33,272,17,32,26,104,42,22,17,125,17,14,12,322,15*7A
$GPGSV,3,3,12,16,10,177,19,44,09,108,,23,08,042,20,03,07,213,17*7A
$GPRMC,133453.000,A,4332.7180,N,00129.2811,E,0.11,118.87,130622,,,A*66
$GPVTG,118.87,T,,M,0.11,N,0.21,K,A*39
 * longest sequence is about 75 chars, short burst 240 chars, long burst 448 chars
 * note: we have 2k bytes total RAM
 */

 #include <SoftwareSerial.h>
#define RX 7
#define TX 8
#define ANALOGPIN 5
SoftwareSerial mysoft = SoftwareSerial( RX, TX );
bool verbose = false;
bool plot = false;

void setup() {
Serial.begin(115200);
mysoft.begin(9600);

}

void loop() {
char c;
// analog processing
  if  ( plot ) {
      static long tloop = 0;
      tloop += 10;
      int val = analogRead( ANALOGPIN );
      int tt = millis() % 500;
      Serial.print(tt); Serial.print(" 1023 "); Serial.println(val);
      while ( millis() < tloop ) {}
  }

// processing input from the GPS
  while ( mysoft.available() ) {
    c = mysoft.read();
    if  ( verbose ) {
        Serial.print( c );
    }
  }

// processing PC remote conrol
  if  ( Serial.available() ) {
      c = Serial.read();
      if  ( c == 'v' ) verbose = true; 
      if  ( c == 'p' ) plot = true; 
      if  ( c == 'n' ) { verbose = false; plot = false; } 
  }
}
