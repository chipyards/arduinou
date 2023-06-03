// ce prog teste :
//      - la config de l'horloge ADC @ 500 kHz
//      - l'interruption ADC toutes les 13 periodes de son horloge
// la mesure ADC n'est pas utilisee, le prog mesure juste la frequence d'interrupt
// ( theoriquement (16MHz/32) / 13 =  38461.5 Hz )

// thanks to:
// https://www.instructables.com/id/Arduino-Vocal-Effects-Box/

#define DISP_PERIOD 1000

volatile unsigned long ticks = 0;
unsigned long next_ms = 0;
unsigned int secs = 0;

ISR( ADC_vect ) {   //when new ADC value ready
//  PORTD = ADCH;//send 8 bit value from analog pin 0 to DAC
ticks += 1;
}

// set up continuous sampling of analog pin 0 with interrupt
void adc_init() {
cli();

ADCSRA = 0; ADCSRB = 0;

ADMUX |= (1 << REFS0);  // set reference voltage
ADMUX |= (1 << ADLAR);  // left align the ADC value- so we can read highest 8 bits from ADCH register only
  
ADCSRA |= (1 << ADPS2) | (1 << ADPS0); //set ADC clock with prescale 32 i.e. 500kHz
ADCSRA |= (1 << ADATE); // enabble auto trigger
ADCSRA |= (1 << ADIE);  // enable interrupts when measurement complete
ADCSRA |= (1 << ADEN);  // enable ADC
ADCSRA |= (1 << ADSC);  // start ADC measurements
  
sei();
}

void setup() {
Serial.begin( 115200 );
Serial.println("starting");
adc_init();
}

void loop() {
if  ( Serial.available() ) {
    char c = Serial.read();
    ticks = 0; next_ms = millis() - 1; secs = 0;
    }
if  ( millis() > next_ms ) {
    Serial.print( secs );  Serial.print("\t");
    Serial.print( ticks ); Serial.print("\t");
    if  ( secs < 1 ) Serial.println();
    else Serial.println( ticks / secs );
    next_ms += DISP_PERIOD;
    ++secs;
    }
}
