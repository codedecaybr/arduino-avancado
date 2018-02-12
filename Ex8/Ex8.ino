/*
www.CodeDecay.com.br
Licença de uso: Apache 2.0

Neste exercício vamos criar um servidor local no ESP8266 que será capaz de
receber um comando GET para ligar e desligar o ventilador
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

char ssidStation[] = "gauss";            // nome da rede wifi a ser utilizada
char senhaStation[] = "coxinha10";        // senha da rede wifi a ser utilizada

char ssidAP[] = "codedecay";                 // nome do access point
char senhaAP[] = "codedecay2017";         // senha do access point

char enderecoThingspeak[] = "api.thingspeak.com"; // endereço do Thingspeak
String keyThingspeak = "36H51R1D0CWQ14W0"; // a chave do seu canal

/*Inicializa o objeto do tipo cliente, que usaremos para enviar as informações
para o servidor do Thingspeak*/
WiFiEspClient espCliente;
/*Inicializa o objeto do tipo servidor, que receberá requisições do seu celular
ou notebook e enviará uma página HTML simples como resposta*/
WiFiEspServer espServidor(80);

// O RingBuffer armazena a requisição recebida pelo servidor
RingBuffer buf(20);
// Armazena o estado atual do fan:
int estadoFan = LOW;

void setup() {
  pinMode(12, OUTPUT); // Inicia o pino de controle do relé como saída
  digitalWrite(12, estadoFan);

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

  // verifica se o último envio foi feito há mais de 60000 segundos:
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

void atenderCliente(WiFiEspClient cliente) {
  buf.init(); // inicializa o buffer circular
  while ( cliente.connected() ) {
    if ( cliente.available() ) { // verifica se ainda existem bytes a serem recebidos do cliente
      buf.push( cliente.read() ); // adiciona o byte lido no buffer

      /* verifica se a requisição recebida contém um parâmetro cmd=toggle: */
      if ( buf.endsWith("GET /?cmd=toggle") ) {
        if (estadoFan == HIGH){
          digitalWrite(12, LOW);
          estadoFan = LOW;
        } else {
          digitalWrite(12, HIGH);
          estadoFan = HIGH;
        }
      }

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
  cliente.print("<br>\r\n");
  cliente.print("Temperatura: ");
  cliente.print(analogRead(0) * (500.0/1023.0) );
  cliente.print("<br>\r\n");
  cliente.print("Liga/Desliga: <a href=\"/?cmd=toggle\"><button type=\"button\">ON/OFF</button><a>");
  cliente.print("</html>\r\n");
}
