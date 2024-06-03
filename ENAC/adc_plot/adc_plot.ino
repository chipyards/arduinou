void setup() {
Serial.begin(115200);
Serial.println("min,ADC_0,max"); 
}

#define PERIOD 20

void loop() {
static int cnt = 1;
static int oldm = 0;
unsigned int a, m = millis();
if  ( m != oldm ) {
    if ( ( m % PERIOD ) == 0 ) {
       if  ( cnt > 0 )
           cnt++;
       if  ( cnt > 500 )
           cnt = 0;
       if  ( cnt )
           {
           a = analogRead(0);
           Serial.print("0,"); Serial.print( a ); Serial.println(",1023");
           }
       }
    oldm = m;
    }
}
