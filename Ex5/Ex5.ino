/*
www.CodeDecay.com.br
Licença de uso: Apache 2.0
Neste exercício vamos adicionar a função de toggle ao ventilador
*/

/* Iremos utilizar a bibiloteca WiFIEsp para facilitar as operações com o ESP */
#include "SoftwareSerial.h"

/*E tambem a SoftwareSerial para estabelecer comunicação serial do Arduino
com o ESP */
#include "WiFiEsp.h"


/* Inicializa objeto do tipo SoftwareSerial especificando que os pinos 10 e 11
do Arduino serão RX e TX desta interface. A comunicação serial padrão do
Arduino com o computador é realizada sempre com os pinos 0 e 1. */
SoftwareSerial ESP8266(10, 11);

/*Inicializa o objeto do tipo cliente, que usaremos para enviar as informações
para o servidor do Thingspeak*/
RingBuffer buf(20);

// Inicializar servidor no ESP que receberá conexões na porta 80 (HTTP)
WiFiEspServer espServidor(80);

void setup() {

  Serial.begin(9600);  // Inicializa comunic. serial Arduino <-> computador
  ESP8266.begin(9600); // Inicializa comunic. serial Arduino <-> ESP

  /* Associa a interface serial do ESP8266 com o objeto
  WiFi do Arduino, indicando que todas as conexões de rede são feitas através
  do ESP8266 conectado na interface SoftwareSerial indicada */
  WiFi.init( &ESP8266 );

  // Conecta à rede WiFi
  WiFi.begin("code", "codecode");

  // Mostra nome da rede à qual o modulo esta conectado
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Mostra o IP local do ESP na rede WiFi
  IPAddress ip = WiFi.localIP();
  Serial.print("End. IP: ");
  Serial.println(ip);
  
  //Mostra a potencia do sinal WiFi da rede
  Serial.print("Potencia do sinal (RSSI): ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  Serial.println();

  // Incia o AP (nome, canal, senha, segurança, desligar station)
  WiFi.beginAP("WiFi-Douglas", 2, "12345678", ENC_TYPE_WPA_PSK, false);

  // Mostra o endereço IP do AP, que usamos para conectar localmente ao servidor
  Serial.print("Endereco IP do AP: ");
  Serial.println(WiFi.localIP());

  //Inicializa servidor
  espServidor.begin();
  
}

void loop() {
  
  // aguarda a conexao de clientes novos:
  WiFiEspClient cliente = espServidor.available();
  if (cliente) atenderCliente(cliente);
}

// Esta função processa as requisições recebidas
void atenderCliente ( WiFiEspClient cliente){
  buf.init(); //inicializa o buffer circular

  while(cliente.connected()){
    if(cliente.available()){ // verifica se ainda existem bytes a serem recebidos do cliente
      buf.push(cliente.read()); // adiciona o byte lido ao buffer
    }

    /* um request http termina com dois \r\n consecutivos.
      quando atingirmos essa situacao, basta enviar a resposta */
    if(buf.endsWith("\r\n\r\n")){
      enviarResposta(cliente);
      break;
    }
  }
  cliente.stop(); //desconecta do cliente
}

void enviarResposta(WiFiEspClient cliente){
  // Envia o cabeçalho da resposta HTTP
  cliente.print("HTTP/1.1 200 OK\r\n");
  cliente.print("Content-Type: text/html\r\n");
  cliente.print("Connection: close\r\n");
  cliente.print("\r\n");
  // Envia o corpo da mensagem HTTP
  cliente.print("<!DOCTYPE HTML>\r\n"); // não é uma tag HTML! É uma declaração ao navegador sobre a versão de html da página
  cliente.print("<html>\r\n"); // tag html que diz ao navegador que este arquivo é no formato html
  cliente.print("<h1>Smartfan</h1>\r\n"); //tag HTTP de título
  cliente.print("<br>\r\n"); // tag HTTP para pular linha
  cliente.print("</html>\r\n");
}