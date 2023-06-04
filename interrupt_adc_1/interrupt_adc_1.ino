// ce prog implemente un oscilloscope rudimentaire sur pin A0 :
//      - la config de l'horloge ADC @ 500 kHz
//      - l'interruption ADC toutes les 13 periodes de son horloge (26 us, 38461.5 Hz)
//      - buffer circulaire de 512 samples de 8 bits
//      - display differe sur serial plotter avec ticks chaque ms
//      - trigger sur depassement athr, single sweep
//          - envoyer a pour demarrer, s pour trigger manuel

// thanks to:
// https://www.instructables.com/id/Arduino-Vocal-Effects-Box/

#define DISP_PERIOD 1000
#define QBUF (1<<9)       // power of 2

// timing variables
bool trun = 0;              // timing acquisition flag 
volatile unsigned long ticks = 0;
unsigned long next_ms = 0;
unsigned int secs = 0;
// ADC variables
volatile bool arun = 0;     // ADC acquisition run flag
volatile unsigned char abuf[QBUF];   // ADC data buffer
unsigned int wi = 0;        // write index
// trigger variables
volatile bool trig = 0;     // trigger event flag
unsigned char athr = 180;    // trigger level

ISR( ADC_vect ) {   // when new ADC value ready
if  ( arun )
    {
    unsigned char aval = ADCH;  // store 8 bit value
    abuf[ wi & (QBUF-1) ] = aval;
    if  ( aval > athr )
        trig = 1;
    wi++;
    }
else ticks += 1;
}

// set up continuous sampling of analog pin 0 with interrupt
void adc_init() {
cli();

ADCSRA = 0; ADCSRB = 0;

ADMUX |= (1 << REFS0);  // set reference voltage
ADMUX |= (1 << ADLAR);  // left align the ADC value so we can read 8 MSBs from ADCH register only
  
ADCSRA |= (1 << ADPS2) | (1 << ADPS0); //set ADC clock with prescale 32 i.e. 500kHz
ADCSRA |= (1 << ADATE); // enabble auto trigger
ADCSRA |= (1 << ADIE);  // enable interrupts when measurement complete
ADCSRA |= (1 << ADEN);  // enable ADC
ADCSRA |= (1 << ADSC);  // start ADC measurements
  
sei();
}

// afficher 2 courbes, dont une donnant 1 tick toutes les ms
void adisplay() {
  unsigned int usec = 0;
  wi += 2;  // skip 2 old samples
  for ( int i = 0; i < 500; i++ )
      {     // send 500 samples (full plotter windows)
      if  ( usec > 1000 )
          { usec -= 1000; Serial.print( 10 ); }
      else Serial.print( 0 );
      Serial.print(','); 
      Serial.println( abuf[ wi & (QBUF-1) ] );
      wi++; usec += 26;   // sample period = 26 us
      }
}

void setup() {
Serial.begin( 115200 );
Serial.println("starting");
pinMode( 13, OUTPUT );
digitalWrite( 13, 0 );
adc_init();
}

void loop() {
if  ( Serial.available() ) {
    char c = Serial.read();
    switch( c ) {
      case 'a' : arun = 1; trun = 0; digitalWrite( 13, 1 ); break;
      case 's' : arun = 0; adisplay(); digitalWrite( 13, 0 ); break;
      case 't' : arun = 0; trun = 1; digitalWrite( 13, 0 ); break;
      default  : ticks = 0; next_ms = millis() - 1; secs = 0; break;
      }
    }
if  ( trig )
    {
    delay(7);   // half buffer
    arun = 0; adisplay(); digitalWrite( 13, 0 );
    trig = 0;
    }
if  ( millis() > next_ms ) {
    if  ( trun )
        {
        Serial.print( secs );  Serial.print("\t");
        Serial.print( ticks ); Serial.print("\t");
        if  ( secs < 1 ) Serial.println();
        else Serial.println( ticks / secs );
        }
    next_ms += DISP_PERIOD;
    ++secs;
    }
}
