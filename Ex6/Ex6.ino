/*
www.CodeDecay.com.br
Licença de uso: Apache 2.0

Neste exercício vamos criar um servidor local no ESP8266
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

char ssidAP[] = "codedecay";                 // nome do access point
char senhaAP[] = "codedecay2017";         // senha do access point

/*Inicializa o objeto do tipo cliente, que usaremos para enviar as informações
para o servidor do Thingspeak*/
WiFiEspClient espCliente;
/*Inicializa o objeto do tipo servidor, que receberá requisições do seu celular
ou notebook e enviará uma página HTML simples como resposta*/
WiFiEspServer espServidor(80);

// O RingBuffer armazena a requisição recebida pelo servidor
RingBuffer buf(20);


void setup() {

  Serial.begin(9600);  // Inicializa comunic. serial Arduino <-> computador
  ESP8266.begin(9600); // Inicializa comunic. serial Arduino <-> ESP
  WiFi.init( &ESP8266 ); /* Associa a interface serial do ESP8266 com o objeto
  WiFi do Arduino, indicando que todas as conexões de rede são feitas através
  do ESP8266 conectado na interface SoftwareSerial indicada */

  // Tenta conectar à rede wifi
  Serial.print( "Tentando conectar a rede wifi WPA de SSID: " );
  Serial.println( ssidStation );
  while ( WiFi.begin(ssidStation, senhaStation) != WL_CONNECTED ) delay(10);
  // Informa que a conexão foi bem sucedida e mostra informações mais detalhadas
  printWifiStatus();

  // Tenta iniciar o Access Point
  Serial.print( "Tentando iniciar o Access Point de SSID: " );
  Serial.println( ssidAP );
  // Inicia o AP (nome, canal, senha, segurança, desligar station)
  if ( WiFi.beginAP(ssidAP, 10, senhaAP, ENC_TYPE_WPA2_PSK, false) ) {
    Serial.print("Endereco IP no AP: ");
    Serial.println( WiFi.localIP() );
    espServidor.begin(); // inicia o servidor na porta 80
  }
}

void loop() {

  // aguarda a conexao de clientes novos:
  WiFiEspClient cliente = espServidor.available();
  if ( cliente ) atenderCliente( cliente );

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

void atenderCliente(WiFiEspClient cliente) {
  buf.init(); // inicializa o buffer circular
  while ( cliente.connected() ) {
    if ( cliente.available() ) { // verifica se ainda existem bytes a serem recebidos do cliente
      buf.push( cliente.read() ); // adiciona o byte lido no buffer

      /* um request http termina com dois \r\n consecutivos
      quando atingirmos essa situacao, basta enviar a resposta */
      if ( buf.endsWith("\r\n\r\n") ) {
        enviarResposta( cliente );
        break;
      }
    }
  }
  cliente.stop(); // desconecta do cliente
}

void enviarResposta(WiFiEspClient cliente) {
  // Envia o cabeçalho da resposta:
  cliente.print("HTTP/1.1 200 OK\r\n");
  cliente.print("Content-Type: text/html\r\n");
  cliente.print("Connection: close\r\n");
  cliente.print("\r\n");
  // Envia o corpo (html) da resposta:
  cliente.print("<!DOCTYPE HTML>\r\n");
  cliente.print("<html>\r\n");
  cliente.print("<h1>Smartfan</h1>\r\n");
  cliente.print("</html>\r\n");
}
