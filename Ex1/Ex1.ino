/*
  www.CodeDecay.com.br
  Licença de uso: Apache 2.0

  Neste sketch inicializamos a comunicação 
  serial com o ESP, enviamos comandos
  e observamos as respostas.
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
 *  que os pinos 3 e 2 do arduino serão RX e TX desta interface. A comunicação serial padrão do
 *  arduino com o computador é realizada sempre com os pinos 0 e 1.
 */
SoftwareSerial SerialEsp(3,2); 

void setup()
{
  Serial.begin(9600);    // Inicializa com. serial arduino <-> computador
  SerialEsp.begin(9600); // Inicializa com. serial arduino <-> ESP
  WiFi.init(&SerialEsp); // 
}
 
void loop()
{
  int numRedes = WiFi.scanNetworks(); // Aqui o codigo da biblioteca se encarrega de enviar AT+CWLAP para o ESP e organizar a resposta.
  Serial.println("Numero de redes WiFi detectadas: ");
  Serial.println(numRedes); // -1 significa nenhuma rede encontrada

  /*
   * O objeto Wifi contém as informações de todas as redes localizadas,
   * e estas informações podem ser extraídas passando o numero da rede
   * para os métodos SSID (para extrair nome, SSID, da rede) ou RSSI
   * para extrair a intensidade do sinal:
   */

   for (int numRede = 0; numRede < numRedes; numRede++)
   {
    Serial.print(numRede); // imprime o numero da rede em questao
    Serial.print(" ");
    Serial.print(WiFi.SSID(numRede)); //imprime o SSID da rede de numero numRede
    Serial.print(" ");
    Serial.print(WiFi.RSSI(numRede)); // imprime intensidade do sinal da rede
    Serial.println(" dBm");
   }
   delay(5000);
    
}
