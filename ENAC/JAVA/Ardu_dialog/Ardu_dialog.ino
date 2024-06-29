void setup() {
Serial.begin( 9600 );
Serial.print( "RESET " );
}

void loop() {
if  ( Serial.available() )
    {
    char val = Serial.read();
    char tbuf[32];
    snprintf( tbuf, sizeof(tbuf), "%d = 0x%02x  ", val, val );
    Serial.print( tbuf );
    }
delay( 50 );
}
