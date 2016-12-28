/*
www.CodeDecay.com.br
Licença de uso: Apache 2.0

Neste sketch enviamos uma leitura de temperatura para o Thingspeak de modo que
possa ser visualizada através da Internet.
*/

/*Iremos utilizar a biblioteca WiFiEsp para facilitar as operações com o ESP */
#include "WiFiEsp.h"

/*E tambem a SoftwareSerial para estabelecer comunicação serial do Arduino
com o ESP */
#include "SoftwareSerial.h"

/* Inicializa objeto do tipo SoftwareSerial especificando que os pinos 10 e 11
do Arduino serão RX e TX desta interface. A comunicação serial padrão do
Arduino com o computador é realizada sempre com os pinos 0 e 1. */
SoftwareSerial ESP8266(10, 11); // RX do Arduino, TX do Arduino

char ssidStation[] = "Claudia";            // nome da rede wifi a ser utilizada
char senhaStation[] = "140897hr";        // senha da rede wifi a ser utilizada

char enderecoThingspeak[] = "api.thingspeak.com";
String keyThingspeak = "36H51R1D0CWQ14W0";

// Inicializa o objeto do ESP8266 como cliente.
WiFiEspClient cliente;

void setup() {
  Serial.begin(9600);  // Inicializa comunic. serial Arduino <-> computador
  ESP8266.begin(9600); // Inicializa comunic. serial Arduino <-> ESP
  WiFi.init( &ESP8266 );

  // tenta conectar à rede wifi
  Serial.print( "Tentando conectar a rede wifi WPA de SSID: " );
  Serial.println( ssidStation );
  while ( WiFi.begin(ssidStation, senhaStation) != WL_CONNECTED ) delay(10);
  //Informa que a conexão foi bem sucedida e mostra informações mais detalhadas
  printWifiStatus();
}

void loop() {
  enviarTemperatura();
  delay(10*1000); // Espera um minuto
}

void printWifiStatus() {
  // Imprime o SSID da rede a qual nos conectamos
  Serial.print("SSID: ");
  Serial.println( WiFi.SSID() );

  // Imprime o IP do ESP8266
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println( ip );

  // Imprime a potência do sinal
  Serial.print("Potencia do sinal (RSSI):");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  Serial.println();
}

void enviarTemperatura() {
  int leitura = analogRead(0);
  String temperatura = String(leitura);

  // Se conseguir conectar ao servidor, faz a requisição
  if ( cliente.connect(enderecoThingspeak, 80) ) {
    Serial.println("Conectado ao servidor!");
    //Vamos montar a primeira linha do cabeçalho, a unica que precisa ser alterada
    String cabecalho = "GET /update?key=" + keyThingspeak + "&field1=" + temperatura + " HTTP/1.1";
    cliente.println(cabecalho);
    cliente.println("Host: api.thingspeak.com");
    cliente.println("Connection: close");
    cliente.println();
  }

  cliente.stop();
  Serial.println();
}
