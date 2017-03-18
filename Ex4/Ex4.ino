/*
www.CodeDecay.com.br
Licença de uso: Apache 2.0

Neste exercício vamos enviar uma leitura de temperatura para o Thingspeak,
de modo que ela possa ser visualizada através da Internet.
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

char enderecoThingspeak[] = "api.thingspeak.com"; // endereço do Thingspeak
String keyThingspeak = "36H51R1D0CWQ14W0"; // a chave do seu canal

/*Armazena o instante em que a placa enviou a requisição mais recente para
o servidor do Thingspeak*/
unsigned long ultimoEnvio = millis();

/*Inicializa o objeto do tipo cliente, que usaremos para enviar as informações
para o servidor do Thingspeak*/
WiFiEspClient espCliente;

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
}

void loop() {
  // verifica se o último envio foi feito há mais de 60000 milisegundos:
  if ( millis() > ultimoEnvio + 60000 )  {
    enviarTemperatura(); // envia a temperatura para o Thingspeak
    ultimoEnvio = millis(); // atualiza o momento em que a ultima requisição foi enviada.
  }
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
  // realiza a leitura analógica e a conversão para graus Celsius:
  int leitura = analogRead(0)*500.0/1023.0;
  // converte o resultado da conta anterior para texto:
  String temperatura = String(leitura);

  // Se conseguir conectar ao servidor, faz a requisição:
  if ( espCliente.connect(enderecoThingspeak, 80) ) {
    Serial.println("Conectado ao servidor!");
    // Vamos montar a primeira linha do cabeçalho, a única que precisa ser alterada:
    String cabecalho = "GET /update?key=" + keyThingspeak + "&field1=" + temperatura + " HTTP/1.1";
    espCliente.println(cabecalho);
    // Vamos montar as outras linhas do cabeçalho, que são estáticas:
    espCliente.println("Host: api.thingspeak.com");
    espCliente.println("Connection: close");
    espCliente.println();
  }
  espCliente.stop(); // Encerra a conexão com o servidor
  Serial.println();
}
