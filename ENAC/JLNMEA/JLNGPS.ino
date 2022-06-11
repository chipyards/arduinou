#include <JLNMEA.h>

#include <SoftwareSerial.h>
#define rxPin 7
#define txPin 8

// Set up a new SoftwareSerial object
SoftwareSerial mysoft = SoftwareSerial(rxPin, txPin);
int period = 50;
nmea_ctx lectx;    // we allocate the context here, it has a pointer to the variables
#define CTX (&lectx)  // and only scalar variables
gps_var lesvars[QVAR];  // we allocate the variables here
bool verbose = true;

void setup()  {
    Serial.begin(115200);
    Serial.println( "Hello GPS" );
    // Define pin modes for TX and RX
    pinMode(rxPin, INPUT);
    pinMode(txPin, OUTPUT);

    // Set the baud rate for the SoftwareSerial object
    mysoft.begin(9600);
    mysoft.listen();

    CTX->data = lesvars;  // then we make the context aware of it
    gps_var_init( CTX );  // we initialize the variables we are interested in
            for  ( int i = 0; i < CTX->qdata; i++ ) {
                vardump( CTX, i );
            }
//  Serial.print("="); Serial.println();
    Serial.print("sizeof(bool)="); Serial.println(sizeof(bool));
    Serial.print("sizeof(int)="); Serial.println(sizeof(int));
    Serial.print("sizeof(long)="); Serial.println(sizeof(long));
    Serial.print("sizeof(gps_var)="); Serial.println(sizeof(gps_var));
    Serial.print("sizeof(lesvars)="); Serial.println(sizeof(lesvars));
    Serial.print("sizeof(lectx)="); Serial.println(sizeof(lectx));
    Serial.print("sizeof(CTX)="); Serial.println(sizeof(CTX));
}

int vardump( nmea_ctx * ctx, int ivar )
{
double d;
Serial.print("var "); Serial.print(ivar); Serial.print(": ");
gps_var * v = &ctx->data[ivar];
if  ( v->stat == 1 )
  { Serial.println("undef"); return 1; }
else if ( v->stat != 0 )
  { Serial.println("invalid"); return 2; }
Serial.print(v->type); Serial.print(" ");
switch  ( v->type )
  {
  case 'a' : Serial.print((char)v->val ); break;
  case 'd' :
  case '2' :
  case '3' : Serial.print((int)v->val ); break;
  case 'f' :  d = (double)v->val;
      for ( int p = 0; p < v->frac; p++ )
        d /= 10.0;
      Serial.print( d ); break;
  }
Serial.println();
return 0;
}

void loop() {

     while  ( mysoft.available() ) {
          char c = mysoft.read();
          if  ( c >= ' ' ) {
              nmea_proc( CTX, c ); if ( verbose ) Serial.print( c );
          } else if  ( c == 13 ) {
              if  ( CTX->status == 42 ) {
                  Serial.print(" <<ok>>");
                  gps_var * v;    
                  v = &CTX->data[16]; // sat count
                  if  ( v->stat == 0 ) {
                      Serial.print(" sats="); Serial.print( (int)v->val );
                  }
                  v = &CTX->data[2]; // seconds
                  if  ( v->stat == 0 ) {
                      Serial.print(" secs="); Serial.println( v->val );
                  }
              } else {
                  invalidate_sentence( CTX );
                  if ( verbose ) Serial.println(" <<bad>>");
              }
          new_sentence(CTX);  // remet a zero le parseur mais les variables sont persistantes
          }
     }
     if ( Serial.available() ) {
        char c = Serial.read();
        if  ( c == 'n' ) verbose = false;
        if  ( c == 'v' ) verbose = true;
        if  ( c == 'd' ) {
            for  ( int i = 0; i < CTX->qdata; i++ ) {
                vardump( CTX, i );
            }

        }
     }
        
}
