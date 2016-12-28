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
char ssidAP[] = "codedecay";                 // nome do access point
char senhaAP[] = "codedecay2017";         // senha do access point

char enderecoThingspeak[] = "api.thingspeak.com";
String keyThingspeak = "36H51R1D0CWQ14W0";

unsigned long ultimoEnvio = millis();

// Inicializa o objeto do ESP8266 como cliente.
WiFiEspClient espCliente;
WiFiEspServer servidor(80);

// O RingBuffer armazena a requisição recebida pelo servidor
RingBuffer buf(20);
int estadoFan = LOW;


void setup() {
  pinMode(13, OUTPUT); // Inicia o pino de controle do relé como saída
  digitalWrite(13, estadoFan);

  Serial.begin(9600);  // Inicializa comunic. serial Arduino <-> computador
  ESP8266.begin(9600); // Inicializa comunic. serial Arduino <-> ESP
  WiFi.init( &ESP8266 );

  // tenta conectar à rede wifi
  Serial.print( "Tentando conectar a rede wifi WPA de SSID: " );
  Serial.println( ssidStation );
  while ( WiFi.begin(ssidStation, senhaStation) != WL_CONNECTED ) delay(10);
  //Informa que a conexão foi bem sucedida e mostra informações mais detalhadas
  printWifiStatus();

  Serial.print( "Tentando iniciar o Access Point de SSID: " );
  Serial.println( ssidAP );
  if ( WiFi.beginAP(ssidAP, 10, senhaAP, ENC_TYPE_WPA2_PSK, false) ) { // inicia o access point
      Serial.print("Endereco IP: ");
      Serial.println( WiFi.localIP() );
      servidor.begin(); // inicia o servidor na porta 80
  }
}

void loop() {
  WiFiEspClient cliente = servidor.available(); // aguarda a conexao de clientes novos
  if ( cliente ) atenderCliente( cliente );

  if ( millis() > ultimoEnvio + 60000 )  {
    enviarTemperatura();
    ultimoEnvio = millis();
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
  int leitura = analogRead(0)*500.0/1023.0;
  String temperatura = String(leitura);

  // Se conseguir conectar ao servidor, faz a requisição
  if ( espCliente.connect(enderecoThingspeak, 80) ) {
    Serial.println("Conectado ao servidor!");
    //Vamos montar a primeira linha do cabeçalho, a unica que precisa ser alterada
    String cabecalho = "GET /update?key=" + keyThingspeak + "&field1=" + temperatura + " HTTP/1.1";
    espCliente.println(cabecalho);
    espCliente.println("Host: api.thingspeak.com");
    espCliente.println("Connection: close");
    espCliente.println();
  }

  espCliente.stop();
  Serial.println();
}

void atenderCliente(WiFiEspClient cliente) {
  bool recebeuGet = false;
  buf.init(); // inicializa o buffer circular
  while ( cliente.connected() ) {
      if ( cliente.available() ) { // verifica se ainda existem bytes a serem recebidos do cliente
          buf.push( cliente.read() ); // adiciona o byte lido no buffer

          if ( buf.endsWith("cmd=toggle") && !recebeuGet ) {
            recebeuGet = true;
            digitalWrite(13, !estadoFan);
            estadoFan = !estadoFan;
          }

          // um request http termina com dois \r\n consecutivos
          // quando atingirmos essa situacao, basta enviar a resposta
          if ( buf.endsWith("\r\n\r\n") ) {
              enviarResposta(cliente);
              break;
          }
      }
  }
  cliente.stop();
}

void enviarResposta(WiFiEspClient cliente) {
    cliente.print("HTTP/1.1 200 OK\r\n");
    cliente.print("Content-Type: text/html\r\n");
    cliente.print("Connection: close\r\n");
    cliente.print("\r\n");
    cliente.print("<!DOCTYPE HTML>\r\n");
    cliente.print("<html>\r\n");
    cliente.print("<h1>Smartfan</h1>\r\n");
    cliente.print("<br>\r\n");
    cliente.print("Temperatura: ");
    cliente.print(analogRead(0) * (500.0/1023.0) );
    cliente.print("<br>\r\n");
    cliente.print("Liga/Desliga: <a href=\"/?cmd=toggle\"><button type=\"button\">ON/OFF</button><a>");
    cliente.print("</html>\r\n");
}
