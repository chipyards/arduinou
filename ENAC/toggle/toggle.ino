void setup() {
pinMode( 2, INPUT_PULLUP ); // pushbutton wired to ground
pinMode( 13, OUTPUT );      // board LED
digitalWrite( 13, 0 );
}

void loop() {
  static bool oldbut = false;
  static bool toggle = false;
  bool newbut = digitalRead( 2 );
  if  ( ( newbut != oldbut ) && ( newbut ) ) {
      if  ( toggle ) {
          toggle = false;
      } else {
          toggle = true;
      }
  }
oldbut = newbut;
digitalWrite( 13, toggle );

delay( 30 );
}
