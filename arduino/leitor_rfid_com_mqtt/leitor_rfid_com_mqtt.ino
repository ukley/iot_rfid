
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <PubSubClient.h>


// Pinos do MFRC522
#define RST_PIN 22
#define SS_PIN 21

// Credenciais Wi-Fi
const char* ssid = "inserirSSID";
const char* password = "inserirpassword";

// Configuração do broker MQTT
const char* mqtt_server = "broker.emqx.io";  // Endereço do broker MQTT
const int mqtt_port = 1883;                   // Porta do broker MQTT (padrão: 1883)
const char* placa_topic = "veiculosro/placa";   // Tópico MQTT para envio da placa


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

   // Obter o UID da tag
  String tagID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    tagID += String(mfrc522.uid.uidByte[i], HEX); // Converte cada byte do UID para hexadecimal
    if (i < mfrc522.uid.size - 1) tagID += ":";  // Adiciona ':' como separador
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

    String tag_placa_topic = tagID+"|"+placa;
    

    // Conecta ao broker MQTT
    while (!client.connected()) {
      Serial.println("Conectando ao broker MQTT...");
      if (client.connect("ESP32Client")) {
        Serial.println("Conectado ao broker MQTT!");
        client.publish(placa_topic, tag_placa_topic.c_str()); // Publica a placa no tópico configurado
        Serial.println();
      } else {
        Serial.print("Falha na conexão. Estado: ");
        Serial.println(client.state());
        delay(2000);
      }
    }      

  } else {
    Serial.print("Erro ao ler: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  }

  // Finaliza a comunicação com a tag
  mfrc522.PICC_HaltA();       // Parar a comunicação com a tag
  mfrc522.PCD_StopCrypto1();  // Finalizar a autenticação

  client.loop(); // Processa mensagens MQTT, se houver
}

