#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "Convidado iot"; // Substitua pela rede do laboratório (ex: iot)
const char* password = "mescla@1417"; // Substitua pela senha da rede

// IP do ESP8266 da sua dupla (preencha após descobrirem no Monitor Serial)
String IP_DO_COLEGA = "192.168.1.101"; 
// IP da dupla Thiago Macan e Martino
ESP8266WebServer server(80);

unsigned long ultimoTempo = 0;
const unsigned long intervalo = 10000; // 10 segundos em milissegundos

void setup() {
  Serial.begin(115200);
  
  // Configuração do LED Interno (GPIO 2)
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH); // Inicia apagado (o LED do ESP8266 opera com lógica invertida)

  // Conexão Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado!");
  
  // Exibe o IP que deve ser passado para o colega
  Serial.print("Seu Endereço IP: ");
  Serial.println(WiFi.localIP()); 

  // Configuração da Rota do Servidor HTTP (/toggle)
  server.on("/toggle", HTTP_GET, []() {
    int estadoAtual = digitalRead(2);
    digitalWrite(2, !estadoAtual); // Inverte o estado físico do pino
    server.send(200, "text/plain", "OK");
    Serial.println("Requisição GET recebida! LED invertido.");
  });

  server.begin();
  Serial.println("Servidor HTTP iniciado na porta 80.");
}

void loop() {
  // 1. Processa requisições de entrada (Servidor HTTP)
  server.handleClient();

  // 2. Dispara requisições de saída a cada 10s (Cliente HTTP)
  if (millis() - ultimoTempo >= intervalo) {
    if (WiFi.status() == WL_CONNECTED && IP_DO_COLEGA != "192.168.X.X") {
      WiFiClient client;
      HTTPClient http;

      String url = "http://" + IP_DO_COLEGA + "/toggle";
      Serial.println("Enviando GET para: " + url);
      
      http.begin(client, url);
      int codigoResposta = http.GET();

      if (codigoResposta > 0) {
        String payload = http.getString();
        Serial.printf("Resposta do colega: %d - %s\n", codigoResposta, payload.c_str());
      } else {
        Serial.printf("Erro na requisição: %s\n", http.errorToString(codigoResposta).c_str());
      }

      http.end();
    }
    ultimoTempo = millis(); // Reinicia o cronômetro
  }
}
