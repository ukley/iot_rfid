
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>


// Pinos do MFRC522
#define RST_PIN 22
#define SS_PIN 21

// Credenciais Wi-Fi
const char* ssid = "###";
const char* password = "###";

// Configuração do broker MQTT
const char* mqtt_server = "broker.emqx.io";  // Endereço do broker MQTT
const int mqtt_port = 1883;                   // Porta do broker MQTT (padrão: 1883)
const char* placa_topic = "veiculosro/placa";   // Tópico MQTT para envio da placa

// URL do endpoint Flask (ajuste conforme necessário) ngrok
String url = "https://71df-2804-2860-6023-f000-39d-2208-edfc-5296.ngrok-free.app/api/dados/placa";  // Local do seu servidor Flask


WiFiClient espClient;         // Cliente Wi-Fi
PubSubClient client(espClient); // Cliente MQTT

MFRC522 mfrc522(SS_PIN, RST_PIN); // Criar uma instância do MFRC522


void setup() {
  Serial.begin(115200); // Comunicação Serial
  SPI.begin();          // Inicia SPI
  mfrc522.PCD_Init();   // Inicia o leitor MFRC522

  // Conecta ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao Wi-Fi...");
  }
  Serial.println("Conectado ao Wi-Fi");
  Serial.println(WiFi.localIP()); // IP do dispositivo na rede

  // Configura o cliente MQTT
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  // Verifica se uma tag foi detectada
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  byte bloco = 4; // Número do bloco onde os dados estão gravados

  // Define a chave padrão
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // Autentica o bloco
  MFRC522::StatusCode status;
  status = mfrc522.PCD_Authenticate(
      MFRC522::PICC_CMD_MF_AUTH_KEY_A,
      bloco,
      &key,
      &(mfrc522.uid)
  );

  if (status != MFRC522::STATUS_OK) {
    Serial.print("Erro na autenticação: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Lê os dados do bloco
  byte buffer[18]; // Buffer para armazenar os dados
  byte bufferSize = sizeof(buffer);
  status = mfrc522.MIFARE_Read(bloco, buffer, &bufferSize);

  if (status == MFRC522::STATUS_OK) {
    String placa = "";
    for (byte i = 0; i < 7; i++) {
      placa += (char)buffer[i]; // Concatena os caracteres lidos na string
    }

    // Conecta ao broker MQTT
    while (!client.connected()) {
      Serial.println("Conectando ao broker MQTT...");
      if (client.connect("ESP32Client")) {
        Serial.println("Conectado ao broker MQTT!");
        client.publish(placa_topic, placa.c_str()); // Publica a placa no tópico configurado
        Serial.println(placa);
      } else {
        Serial.print("Falha na conexão. Estado: ");
        Serial.println(client.state());
        delay(2000);
      }
    }
  
    // Envia a placa para a API via POST
    gravarTrackingVeiculo(placa);

  } else {
    Serial.print("Erro ao ler: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  }

  // Finaliza a comunicação com a tag
  mfrc522.PICC_HaltA();       // Parar a comunicação com a tag
  mfrc522.PCD_StopCrypto1();  // Finalizar a autenticação

  client.loop(); // Processa mensagens MQTT, se houver
}

void enviarPlacaParaApi(String placa) {
    HTTPClient http;

    // URL da API com a placa como parâmetro
    String url = "https://71df-2804-2860-6023-f000-39d-2208-edfc-5296.ngrok-free.app/api/users/placa?placa="+placa;

    Serial.println(url);

    // Inicia a conexão com a API
    http.begin(url);
    http.addHeader("Content-Type", "application/json");  // Adiciona o cabeçalho

    // Faz a requisição GET
    int httpCode = http.GET();

    // Verifica o código de retorno
    if (httpCode > 0) {
        Serial.printf("HTTP Status Code: %d\n", httpCode);

        // Lê a resposta da API
        String payload = http.getString();
        Serial.println("Resposta da API:");
        Serial.println(payload);
    } else {
        Serial.printf("Erro na requisição: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end(); // Finaliza a conexão HTTP
}

void gravarTrackingVeiculo(String placa) {
    HTTPClient http;  // Objeto para gerenciar a requisição HTTP

    String url = "https://670c-2804-2860-6023-f000-39d-2208-edfc-5296.ngrok-free.app/api/dado?placa="+placa;

    // Inicia a conexão com a API
    http.begin(url);
    http.addHeader("Content-Type", "application/json");  // Adiciona o cabeçalho

    // Faz a requisição GET
    int httpCode = http.GET();

    // Verifica o código de retorno
    if (httpCode > 0) {
        Serial.printf("HTTP Status Code: %d\n", httpCode);

        // Lê a resposta da API
        String payload = http.getString();
        Serial.println("Resposta da API:");
        Serial.println(payload);
    } else {
        Serial.printf("Erro na requisição: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end(); // Finaliza a conexão HTTP  
}
