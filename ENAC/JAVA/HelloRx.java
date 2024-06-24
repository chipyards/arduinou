/*
N:\Appli\jdk-8-32\bin\javac -Xlint -classpath ".;jssc-2.8.0-arduino.jar" HelloRx.java
N:\Appli\jdk-8-32\bin\java -classpath ".;jssc-2.8.0-arduino4.jar" HelloRx
N.B. avec jdk-8 (64 bits) et jssc-2.8.0 (arduino ou pas), la dll jSSC-2.8_x86_64.dll
est bien selectionnee mais plante EXCEPTION_ACCESS_VIOLATION
jssc-2.9.6 est ok avec tous les SDK (32 ou 64), mais donne warning SLF4J: Failed to load class "org.slf4j.impl.StaticLoggerBinder"
*/
					// Finalement compile Ok le 2eme cas en ajoutant throws Exception a la main()
					// https://en.wikipedia.org/wiki/Classpath
//import jssc;				// HelloRx.java:1: error: '.' expected
import jssc.*;				// HelloRx.java:10: error: unreported exception SerialPortException; must be caught or declared to be thrown
//import jscc.SerialPort;		// HelloRx.java:2: error: package jscc does not exist (pourtant c'est dans Serial.java d'arduino)
//import jscc.SerialPort.*;		// HelloRx.java:3: error: package jscc.SerialPort does not exist
//import static jscc.SerialPort.*;	// HelloRx.java:4: error: package jscc does not exist

public class HelloRx {
  public static void main(String[] args) throws Exception {

	SerialPort port = new SerialPort("COM6");
	port.openPort();
	port.setParams( 9600, 8, 1, 0 );	// bauds, bits, stops, parity
	byte[] buffer = port.readBytes(10);	// read first 10 bytes
	System.out.println("C'est " + new String( buffer ) );
	System.out.println("C'est " + "ouf2" );
	port.closePort();

	}
}
