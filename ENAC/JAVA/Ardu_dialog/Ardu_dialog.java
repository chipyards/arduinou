/*
N:\Appli\jdk-8-32\bin\javac -Xlint -classpath ".;jssc-2.8.0-arduino4.jar" Ardu_dialog.java
N:\Appli\jdk-8-32\bin\java -classpath ".;jssc-2.8.0-arduino4.jar" Ardu_dialog COM6 42
N.B. avec jdk-8 (64 bits) et jssc-2.8.0 (arduino ou pas), la dll jSSC-2.8_x86_64.dll
est bien selectionnee mais plante EXCEPTION_ACCESS_VIOLATION
jssc-2.9.6 est ok avec tous les SDK (32 ou 64), mais donne warning SLF4J: Failed to load class "org.slf4j.impl.StaticLoggerBinder"
*/

/**
Ce prog accepte un nom de port t.q. COM6 comme 1er argument, un nombre comme 2e argument
Periodiquement le nombre est emis sous forme d'un byte puis decremente,
jusqu'a atteindre la valeur zero.
Les reponses sont affichees, interpretees comme String 
*/

import jssc.*;

public class Ardu_dialog {

  public static void dump_args( String[] args )
	{
	int i = 0;
	for 	( String elem: args )
		System.out.println("Argument " + (i++) + ": " + elem);
	}

  public static void main( String[] args ) throws Exception
	{
	dump_args( args );
	// ouverture du port serie
	String portName = "COM6";
	if	( args.length >= 1 )
		portName = args[0];
	System.out.println("opening " + portName );
	SerialPort port = new SerialPort( portName );
	port.openPort();
	port.setParams( 9600, 8, 1, 0 );	// bauds, bits, stops, parity
	// traitement data a emettre
	byte val = 0;
	if	( args.length >= 2 )
		val = (byte)Integer.parseInt(args[1]);
	System.out.println("val = " + val );
	// tempo car le port peut resetter l'Arduino
	System.out.println("Wait for reset");
	Thread.sleep( 5000 );
	// dialogue
	String s;
	while	( val >= 0 )
		{
		s = port.readString();		// returns null if no data is available
		if	( s != null )
			{
			System.out.printf("%s%n", s );
			}
		port.writeByte( val-- );
		Thread.sleep( 1000 );
		}
	port.closePort();
	}
}
