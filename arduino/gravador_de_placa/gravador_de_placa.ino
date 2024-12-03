#include <SPI.h>
#include <MFRC522.h>

// Pinos do MFRC522
#define RST_PIN 22
#define SS_PIN 21

MFRC522 mfrc522(SS_PIN, RST_PIN); // Criar uma instância do MFRC522

void setup() {
  Serial.begin(115200); // Comunicação Serial
  SPI.begin();          // Inicia SPI
  mfrc522.PCD_Init();   // Inicia o leitor MFRC522

  Serial.println("Aproxime a tag para gravar...");
}

void loop() {
  // Verifica se uma tag foi detectada
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  byte bloco = 4; // Número do bloco onde os dados serão gravados

  // Define a chave padrão
  MFRC522::MIFARE_Key key; // Uso correto da estrutura MIFARE_Key
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

  // Dados a serem gravados (placa do veículo)
  String placa = "PWL1120"; // Número da placa a ser gravado (máx. 16 bytes)
  byte buffer[16] = {0};    // Buffer de 16 bytes
  placa.getBytes(buffer, sizeof(buffer)); // Converte a string para bytes

  // Grava os dados na tag
  status = mfrc522.MIFARE_Write(bloco, buffer, 16);
  if (status == MFRC522::STATUS_OK) {
    Serial.println("Placa " + placa + " gravada com sucesso!");
  } else {
    Serial.print("Erro ao gravar: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  }

  // Finaliza a comunicação com a tag
  mfrc522.PICC_HaltA();       // Parar a comunicação com a tag
  mfrc522.PCD_StopCrypto1();  // Finalizar a autenticação
}
