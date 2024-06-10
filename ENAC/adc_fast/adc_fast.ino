// ADC "single sweep" oscilloscope

// this function gets 500 samples, then plots them
// sample period is in ms, the total duration is printed last
// seems to work up to 1 Ksample/s  
void timed_run( int period )
{
  int cnt = 0;
  int val[500];
  unsigned long int t, start_date, stop_date, next_date;
  next_date = start_date = millis();
  while ( cnt < 500 )
    {
    t = millis();
    if  ( t > next_date )
      {
       val[cnt] = analogRead( 0 );
       cnt++;
       next_date += period;
      }
    }
  stop_date = millis();
  for ( int i = 0; i < 500; i++ )
      Serial.println( val[i] );
  Serial.print("took_"); Serial.println( stop_date - start_date );
}

// this function gets 500 samples, together with a reference square signal
// for creating timestamps with a period 20 times the specified sampling period
// then plots them
// sample period is in ms, the total duration is printed last
// seems to work up to 1 Ksample/s  
void timed_run_ticks( int period )
{
  int cnt = 0; char tickle = 0; int tick_period = period * 20;
  int val[500]; char tick_val[500];   // 1500 bytes of RAM (out of 2k)!
  unsigned long int t, start_date, stop_date, next_date, next_tick;
  next_date = next_tick = start_date = millis();
  while ( cnt < 500 )
    {
    t = millis();
    if  ( t > next_date )
      {
        val[cnt] = analogRead( 0 );
       next_date += period;
       if ( t > next_tick )
          {
            tickle ^= 1;
            next_tick += tick_period;
          }
        tick_val[cnt] = tickle;
       cnt++;
      }
    }
  stop_date = millis();
  for ( int i = 0; i < 500; i++ )
      {
      Serial.print( tick_val[i] << 10 ); Serial.print(" "); Serial.println( val[i] );
      }
  Serial.print("took_"); Serial.println( stop_date - start_date );
}

// this function gets 500 samples "as fast as possible", then plots them
// the total duration is printed last
// observed period is close to 0.11ms with Uno board
void fast_run()
{
  int cnt = 0;
  int val[500];
  unsigned long int start_date, stop_date;
  start_date = millis(); 
  while ( cnt < 500 )
    {
         val[cnt] = analogRead( 0 );
       cnt++;
    }
  stop_date = millis();
  for ( int i = 0; i < 500; i++ )
      Serial.println( val[i] );
  Serial.print("took_"); Serial.println( stop_date - start_date );
}

// this function gets 500 samples, together with a reference square signal
// for creating timestamps with a period of 5 ms
// then plots them, the total duration is printed last
void fast_run_ticks()
{
  int cnt = 0; char tickle = 0;
  int val[500]; char tick_val[500];   // 1500 bytes of RAM (out of 2k)!
  unsigned long int t, start_date, stop_date, next_tick;
  start_date = next_tick = millis();
  while ( cnt < 500 )
    {
      t = millis();
         val[cnt] = analogRead( 0 );
      if ( t > next_tick )
          {
            tickle ^= 1;
            next_tick += 5;
          }
      tick_val[cnt] = tickle;
       cnt++;
    }
  stop_date = millis();
  for ( int i = 0; i < 500; i++ )
      {
      Serial.print( tick_val[i] << 10 ); Serial.print(" "); Serial.println( val[i] );
      }
  Serial.print("took_"); Serial.println( stop_date - start_date );
}


void setup() {
//  Serial.begin(115200);
  Serial.begin(1200);     // 1200 Baud is good for observing RX UART pin - send f followed by 6 bytes
                          // good characters : one one: SP, '@'; two ones: '0', 'A'; three ones: '1', 'a'
                          // four ones : '<', 'l'; five ones: '>', 'n'; six ones : '~'  
  analogWrite( 9, 128 );  // 2040us reference signal
  Serial.print("test_"); Serial.println( analogRead(0) );
  delay(50);
}

void loop() {
if  ( Serial.available() )
    {
    switch( Serial.read() )
      {
        case '1' :
          timed_run(1); break;
        case 't' :
          timed_run_ticks(1); break;
        case '2' :
          timed_run(2); break;
        case 'f' :
          fast_run(); break;
        case 'T' :
          fast_run_ticks(); break;
      }
    }
}
