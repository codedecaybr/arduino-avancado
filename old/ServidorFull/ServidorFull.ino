/*

*/

#include "WiFiEsp.h"
#include "SoftwareSerial.h"

SoftwareSerial ESP8266(10, 11); // RX do Arduino, TX do Arduino
WiFiEspServer servidor(80);

char ssid[] = "codedecay";                 // nome do access point
char pass[] = "codedecay2017";         // senha do access point

// use a ring buffer to increase speed and reduce memory allocation
RingBuffer buf(20);
int estadoFan = 0;

void setup() {
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);

    Serial.begin(9600);     // inicia a comunicacao serial para debug
    ESP8266.begin(9600);    // inicializa a comunicacao com o ESP8266
    WiFi.init(&ESP8266);    // initialize ESP module

    Serial.print("Tentando iniciar o Access Point ");
    Serial.println(ssid);

    if ( WiFi.beginAP(ssid, 10, pass, ENC_TYPE_WPA2_PSK) ) { // inicia o access point

        IPAddress ip = WiFi.localIP();
        Serial.print("Endereco IP: ");
        Serial.println(ip);

        servidor.begin(); // inicia o servidor na porta 80

    }
}


void loop() {
    WiFiEspClient cliente = servidor.available(); // aguarda a conexao de clientes novos

    if (cliente) { // se aconteceu a conexao
        bool recebeuGet = false;
        buf.init(); // initialize the circular buffer
        while ( cliente.connected() ) {
            if ( cliente.available() ) { // verifica se ainda existem bytes a serem recebidos do cliente
                char c = cliente.read(); // le um byte
                buf.push(c);    // adiciona o byte lido no buffer

                // um request http termina com dois \r\n consecutivos
                // quando atingirmos essa situacao, basta enviar a resposta
                if ( buf.endsWith("cmd=toggle") && !recebeuGet ) {
                  recebeuGet = true;
                  if ( estadoFan == 1 ) digitalWrite(13, LOW);
                  else digitalWrite(13, HIGH);
                  estadoFan = !estadoFan;
                }

                if ( buf.endsWith("\r\n\r\n") ) {
                    enviaResposta(cliente);
                    break;
                }
            }
        }

        delay(10);

        // encerra a conexao
        cliente.stop();
    }
}

void enviaResposta(WiFiEspClient cliente) {
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
