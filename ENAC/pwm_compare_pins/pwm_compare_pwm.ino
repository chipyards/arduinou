void setup() {
  // put your setup code here, to run once:
}

unsigned char pw = 88;

void loop() {
analogWrite( 3, pw );
analogWrite( 5, pw );
analogWrite( 6, pw );
analogWrite( 9, pw );
analogWrite( 10, pw );
analogWrite( 11, pw );
pw++;
delay(50);
}
