/*
N:\Appli\jdk-22\bin\java -classpath ".;jssc-2.8.0.jar" Listeur
N:\Appli\jdk-22\bin\java -classpath ".;jssc-2.8.0.jar" Listeur
marche avec tous les JDKs, 32 ou 64 bits (ne semble pas utiliser de dll
*/
import jssc.*;

public class Listeur {
  public static void main(String[] args) {
	for(String port : jssc.SerialPortList.getPortNames()) {
		System.out.println(port);
	}
  }
}

