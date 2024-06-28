/*
N:\Appli\jdk-8-32\bin\javac -Xlint -classpath ".;jssc-2.8.0-arduino4.jar" HelloRx2.java
N:\Appli\jdk-8-32\bin\java -classpath ".;jssc-2.8.0-arduino4.jar" HelloRx2
N.B. avec jdk-8 (64 bits) et jssc-2.8.0 (arduino ou pas), la dll jSSC-2.8_x86_64.dll
est bien selectionnee mais plante EXCEPTION_ACCESS_VIOLATION
jssc-2.9.6 est ok avec tous les SDK (32 ou 64), mais donne warning SLF4J: Failed to load class "org.slf4j.impl.StaticLoggerBinder"
*/

import jssc.*;

public class HelloRx2 {
  public static void main(String[] args) throws Exception
	{
	SerialPort port = new SerialPort("COM6");
	port.openPort();
	port.setParams( 9600, 8, 1, 0 );	// bauds, bits, stops, parity
	int cnt = 50;
	String s;
	while	( cnt-- > 0 )
		{
		s = port.readString();		// returns null if no data is available
		if	( s != null )
			{
			System.out.printf("%s%n", s );
			}
		port.writeString( "a" );
		Thread.sleep( 1000 );
		}
	port.closePort();
	}
}
