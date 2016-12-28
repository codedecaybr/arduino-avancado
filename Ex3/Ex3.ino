/*
www.CodeDecay.com.br
Licença de uso: Apache 2.0

Neste exercício vamos conectar o ESP8266 a uma rede WiFi existente.
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

void setup() {

  Serial.begin(9600);  // Inicializa comunic. serial Arduino <-> computador
  ESP8266.begin(9600); // Inicializa comunic. serial Arduino <-> ESP
  WiFi.init( &ESP8266 );

  // Tenta conectar à rede wifi
  Serial.print( "Tentando conectar a rede wifi WPA de SSID: " );
  Serial.println( ssidStation );
  while ( WiFi.begin(ssidStation, senhaStation) != WL_CONNECTED ) delay(10);
  // Informa que a conexão foi bem sucedida e mostra informações mais detalhadas
  printWifiStatus();
}

void loop() {
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
