#include <Wire.h>
#include <Adafruit_PN532.h>
//#include <WiFi.h>  
//#include "SPIFFS.h"
#include <ArduinoJson.h>
//#include "FS.h"
#include "Esp.h"
//#include <ESPAsyncWebServer.h> 
#include "config/wifi.h"
#include "config/spiffs.h"

/* Usando o terminal para ler porta serial
 stty 9600 -F /dev/ttyUSB0 raw -echo
cat /dev/ttyUSB0
*/

/************CONFIG I2C***************************************************/
#define PN532_IRQ   (5)
#define PN532_RESET (4)  // Not connected by default on the NFC Shield
const int DELAY_BETWEEN_CARDS = 1000;
long timeLastCardRead = 0;
boolean readerDisabled = false;
int irqCurr;
int irqPrev;
// This example uses the IRQ line, which is available when in I2C mode.
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);
String lastRead="";
String lastReadBackup="";
char* var[2]= {"Foi encontrado um cartão","Acionada a interrupção"};

char* compareString[2]={"824051485", "4056079132"};

//**************** NFC FUNCTIONS *****************************************
void setupNFC(){
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print(F("Não foi encontrada placa PN53x"));
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print(F("Encontrado chip PN5")); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print(F("Firmware ver. ")); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  // configure board to read RFID tags
  nfc.SAMConfig();
  startListeningToNFC();

}

void startListeningToNFC() {
  // Reset our IRQ indicators
  irqPrev = irqCurr = HIGH;
  
  Serial.println("Waiting for an ISO14443A Card ...");
  nfc.startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A);
}

String handleCardDetected() {
    uint8_t success = false;
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
    uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

    // read the NFC tag's info
    success = nfc.readDetectedPassiveTargetID(uid, &uidLength);
    Serial.println(success ? "Read successful" : "Read failed (not a card?)");
   
    if (success) {
      // Display some basic information about the card
      Serial.println("Found an ISO14443A card");
      Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
      Serial.print("  UID Value: ");
      nfc.PrintHex(uid, uidLength);
      uint32_t cardid_read = uid[0];

      if (uidLength == 4)
      {
        // We probably have a Mifare Classic card ... 
        cardid_read <<= 8;
        cardid_read |= uid[1];
        cardid_read <<= 8;
        cardid_read |= uid[2];  
        cardid_read <<= 8;
        cardid_read |= uid[3]; 
        Serial.print("Seems to be a Mifare Classic card #");
        Serial.println(cardid_read);
      }
      Serial.println("");
      // The reader will be enabled again after DELAY_BETWEEN_CARDS ms will pass.
      readerDisabled = true;
      timeLastCardRead = millis();
      return String(cardid_read);
    }

    // The reader will be enabled again after DELAY_BETWEEN_CARDS ms will pass.
    readerDisabled = true;
}

//***************** LOOP AND SETUP****************************************

void setup(void) {
  pinMode(ledPin, OUTPUT);
  digitalWrite(2,LOW);
 
  Serial.begin(115200);
  while (!Serial) delay(10); // for Leonardo/Micro/Zero
  
  setupSPIFFS();
  setupWifi();
  setupWebserver(); 
  setupNFC();

}

void loop(void) {
  //Rotina para aguardar um tempo de delay após lido o ultimo cartão
  if (readerDisabled) {
    if (millis() - timeLastCardRead > DELAY_BETWEEN_CARDS) {
      readerDisabled = false;
      startListeningToNFC();
    }
  } else {

    irqCurr = digitalRead(PN532_IRQ);
    // Compara Interrupção do sistema para entrar somente quando houver leitura na fila
    if (irqCurr == LOW && irqPrev == HIGH) {
       Serial.println(var[0]);
       Serial.println(var[1]); 
       lastRead= handleCardDetected(); 

    //Varre o array de ID's cadastrados no sistema para verificar permissão ou não de acesso 
        for (byte i = 0; i < 2; i++)
        {
          if(lastRead==compareString[i]){
            Serial.printf("Abertura permitida\n");
            digitalWrite(2,HIGH);
            delay(3000);
            digitalWrite(2,LOW);
          }
        }
      lastReadBackup=lastRead;
      lastRead="";
    }

    irqPrev = irqCurr;
   }

  
}


