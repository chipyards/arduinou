void setup() {
pinMode( 2, INPUT_PULLUP );
pinMode( 13, OUTPUT );
digitalWrite( 13, 0 );
Serial.begin( 9600 );
}

void loop() {
  int pot = analogRead( 0 );
  static bool oldbut = false;
  static bool plotting = false;
  bool newbut = digitalRead( 2 );
  if  ( ( newbut != oldbut ) && ( newbut ) ) {
      if  ( plotting ) {
          plotting = false;
      } else {
          plotting = true;
      }
  }
oldbut = newbut;
digitalWrite( 13, plotting );

delay( 30 );
}
