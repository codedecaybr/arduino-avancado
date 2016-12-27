/*
  www.CodeDecay.com.br
  Licença de uso: Apache 2.0

  Neste sketch inicializamos a comunicação 
  serial com o ESP, enviamos comandos
  e observamos as respostas.
 */

/* Iremos utilizar a biblioteca WiFiEsp para 
 *  facilitar as operações com o ESP
*/
#include "WiFiEsp.h"

/*E tambem a SoftwareSerial (nao precisa instalar) 
para estabelecer comunicação serial do arduino
com o ESP */ 
#include "SoftwareSerial.h"

/* Inicializa objeto do tipo SoftwareSerial especificando
 *  que os pinos 3 e 2 do arduino serão RX e TX desta interface. A comunicação serial padrão do
 *  arduino com o computador é realizada sempre com os pinos 0 e 1.
 */
SoftwareSerial SerialEsp(3,2); 

char ssid[] = "casa";            // your network SSID (name)
char pass[] = "meiaportuguesameiamucarela";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

char server[] = "arduino.cc";

// Initialize the Ethernet client object
WiFiEspClient client;

void setup()
{
  // initialize serial for debugging
  Serial.begin(9600);
  // initialize serial for ESP module
  SerialEsp.begin(9600);
  // initialize ESP module
  WiFi.init(&SerialEsp);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data
  Serial.println("You're connected to the network");
  
  printWifiStatus();

  Serial.println();
  Serial.println("Starting connection to server...");
  // if you get a connection, report back via serial
  if (client.connect(server, 80)) {
    Serial.println("Connected to server");
    // Make a HTTP request
    client.println("GET /asciilogo.txt HTTP/1.1");
    client.println("Host: arduino.cc");
    client.println("Connection: close");
    client.println();
  }
}

void loop()
{
  // if there are incoming bytes available
  // from the server, read them and print them
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if the server's disconnected, stop the client
  if (!client.connected()) {
    Serial.println();
    Serial.println("Disconnecting from server...");
    client.stop();

    // do nothing forevermore
    while (true);
  }
}


void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
