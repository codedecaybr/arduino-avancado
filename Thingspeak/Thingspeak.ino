/*
  www.CodeDecay.com.br
  Licença de uso: Apache 2.0

  Neste sketch enviamos uma leitura de temperatura 
  para o Thingspeak de modo que possa ser visualizada
  através da internet.
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
 *  que os pinos 10 e 11 do arduino serão RX e TX desta interface. A comunicação serial padrão do
 *  arduino com o computador é realizada sempre com os pinos 0 e 1.
 */
SoftwareSerial ESP8266(10, 11); // RX do Arduino, TX do Arduino

char ssid[] = "casa";            // nome da rede wifi a ser utilizada
char pass[] = "meiaportuguesameiamucarela";        // senha da rede wifi a ser utilizada
int status = WL_IDLE_STATUS;     // flag de status da conexão

char server[] = "api.thingspeak.com";
String key = "36H51R1D0CWQ14W0";

// Initialize the Ethernet client object
WiFiEspClient client;

void setup()
{
  Serial.begin(9600);  // Inicializa comunic. serial arduino <-> computador
  ESP8266.begin(9600); // Inicializa comunic. serial arduino <-> ESP
  WiFi.init(&ESP8266); // "[WiFiEsp] Initializing ESP module" "[WiFiEsp] Initilization successful - 1.5.4"

  // Verifica a presença do ESP
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("ESP8266 nao foi detectado");
    while (true); //esta linha mantém o programa travado
  }


  // tenta conectar à rede wifi
  while ( status != WL_CONNECTED) {
    Serial.print("Tentando conectar a rede wifi WPA de SSID: ");
    Serial.println(ssid);
    // Conecta a rede WPA/WPA2, alterando o valor da flag status caso a conexão seja bem sucedida
    status = WiFi.begin(ssid, pass);
  }


  //Informa que a conexão foi bem sucedida e mostra informações mais detalhadas
  Serial.println("Conectado ao WiFi!");
  printWifiStatus();

  Serial.println();
}
void loop()
{
  int leitura = analogRead(0);
  String temperatura = String(leitura);
  Serial.println("Iniciando conexao com o servidor...");
  // Se conseguir conectar ao servidor, imprime na serial e faz requisição
  if (client.connect(server, 80)) {
    Serial.println("Conectado ao servidor!");
    //Vamos montar a primeira linha do cabeçalho, a unica que precisa ser alterada
    String cab = "GET /update?key=" + key + "&field1=" + temperatura + " HTTP/1.1";
    client.println(cab);
    client.println("Host: api.thingspeak.com");
    
    /*
     * Remova ou comente esta proxima linha para ver toda a resposta do servidor! 
     * Note que o corpo da mensagem contém apenas um número identificador do dado 
     * enviado ao canal do Thingspeak. Se este é o quarto dado que você envia, o corpo da mensagem irá conter '4'.
     */
     
    client.println("Connection: close");
    client.println();
  }
  
  //Se há bytes chegando do server, os lê e imprime
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // quando o server desconectar, encerrar o client
  if (!client.connected()) {
    Serial.println();
    Serial.println("Desconectando do servidor...");
    client.stop();
  }
  Serial.println("Rodando delay!");
  delay(10*1000); // Espera um minuto
}


void printWifiStatus()
{
  // Imprime o SSID da rede a qual nos conectamos
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Imprime o IP do ESP8266
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Imprime a potência do sinal
  long rssi = WiFi.RSSI();
  Serial.print("Potencia do sinal (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
