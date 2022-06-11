
  // registers for port B
  char * direction_reg = 0x24;
  char * data_reg = 0x25;


void setup() {
* direction_reg |= 0x20;
}

void loop() {
  static long t0 = 0;
  long t1, t2;
  if ( t0 == 0 )
      t0 = millis();
  else  t0 = t2;
  t1 = t0 + 200;
  t2 = t0 + 500;
* data_reg |= 0x20;       // change only 1 bit (OR forces 1)
while ( millis() != t1 ) {}
* data_reg &= (~0x20);    // change only 1 bit (AND forces 0) //0xDF;
while ( millis() != t2 ) {}
}
