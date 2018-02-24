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

// O RingBuffer armazena a requisição recebida pelo servidor
RingBuffer buf(20);

// Inicializar servidor no ESP que receberá conexões na porta 80 (HTTP)
WiFiEspServer espServidor(80);

// Estado atual do ventilador. Esta variável será alterada pela função atenderCliente
int estadoVent = LOW;

char enderecoThingspeak[] = "api.thingspeak.com"; // endereço do Thingspeak
String keyThingspeak = "AM8WLDRCL1S4GFOD"; // a chave do seu canal

/*Armazena o instante em que a placa enviou a requisição mais recente para
o servidor do Thingspeak*/
unsigned long ultimoEnvio = millis();

/*Inicializa o objeto do tipo cliente, que usaremos para enviar as informações
para o servidor do Thingspeak*/
WiFiEspClient espCliente;

void setup() {
  /* Usaremos o pino 12 para ligar ou desligar o relé */
  pinMode(12, OUTPUT);

  Serial.begin(9600);  // Inicializa comunic. serial Arduino <-> computador
  ESP8266.begin(9600); // Inicializa comunic. serial Arduino <-> ESP

  /* Associa a interface serial do ESP8266 com o objeto
  WiFi do Arduino, indicando que todas as conexões de rede são feitas através
  do ESP8266 conectado na interface SoftwareSerial indicada */
  WiFi.init( &ESP8266 );

  // Conecta à rede WiFi
  WiFi.begin("casa", "meiaportuguesameiamucarela");

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

  // verifica se o último envio foi feito há mais de 60000 segundos:
  if ( millis() > ultimoEnvio + 60000 )  {
    enviarTemperatura(); // envia a temperatura para o Thingspeak
    ultimoEnvio = millis(); // atualiza o momento em que a ultima requisição foi enviada.
  }
}

// Esta função processa as requisições recebidas
void atenderCliente ( WiFiEspClient cliente){
  buf.init(); //inicializa o buffer circular

  while(cliente.connected()){
    if(cliente.available()){ // verifica se ainda existem bytes a serem recebidos do cliente
      buf.push(cliente.read()); // adiciona o byte lido ao buffer

      /* verificamos se a requisição orecebida contém um parâmetro cmd=toggle */
      if (buf.endsWith("GET /?cmd=toggle")){
        if(estadoVent == HIGH){
          estadoVent = LOW;
          digitalWrite(12, estadoVent);
        } else {
          estadoVent = HIGH;
          digitalWrite(12, estadoVent);
        }
      }
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
  cliente.print("Liga/Desliga: <a href=\"/?cmd=toggle\"><button type=\"button\">ON/OFF</button><a>"); // tag html 'a' para identificar um link, href é um atributo desta tag. button é outra tag que modifica a apresentação do link
  cliente.print("</html>\r\n");
}

void enviarTemperatura(){
  // realiza a leitura analógica e a conversão para graus Celsius:
  int leitura = analogRead(0)*500.0/1023.0;
  // converte o resultado da conta anterior para texto:
  String temperatura = String(leitura);
  // Se conseguir conectar ao servidor, faz a requisição:
  if ( espCliente.connect(enderecoThingspeak, 80) ) {
    Serial.println("Conectado ao servidor do Thingspeak!");
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