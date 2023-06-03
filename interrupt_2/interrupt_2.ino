// https://www.instructables.com/id/Arduino-Timer-Interrupts/

// set timer0 interrupt with 64 prescaler
void timer0_init( unsigned char period ) {
  cli();    // stop interrupts
  TCCR0A = TCCR0B = 0;  // reset all
  TCNT0  = 0;           // reset counter
  OCR0A = period - 1;   // compare match register
  TCCR0A |= (1 << WGM01); // turn on CTC mode
  TCCR0B |= (1 << CS01) | (1 << CS00);  // 64 prescaler
  TIMSK0 |= (1 << OCIE0A);  // enable timer compare interrupt
  sei();    // allow interrupts
}

// set timer1 interrupt with 1024 prescaler
void timer1_init( unsigned int period ) {
  cli();
  TCCR1A = TCCR1B = 0;
  TCNT1  = 0;
  OCR1A = period - 1;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);  // 1024 prescaler
  TIMSK1 |= (1 << OCIE1A);
  sei();
  }
  
// set timer2 interrupt with 8 prescaler
void timer2_init( unsigned char period ) {
  cli();
  TCCR2A = TCCR2B = 0;
  TCNT2  = 0;
  OCR2A = period - 1;
  TCCR2A |= (1 << WGM21);
  TCCR2B |= (1 << CS21);   // 8 prescaler
  TIMSK2 |= (1 << OCIE2A);
  sei();
  }


ISR( TIMER1_COMPA_vect ) {    // timer1 interrupt pin 13 (LED)
  static unsigned char toggle1 = 0;
  if (toggle1){
    digitalWrite(13,HIGH);
    toggle1 = 0;
  }
  else{
    digitalWrite(13,LOW);
    toggle1 = 1;
  }
}

void setup() {
pinMode( 13, OUTPUT ); 
digitalWrite(13,HIGH);
timer1_init( 15624 );
}

void loop() {
  // put your main code here, to run repeatedly:

}
