#include <Wire.h>
#include <Adafruit_PN532.h>
#include <WiFi.h>  
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include "FS.h"
#include "SPIFFS.h"
#include "Esp.h"
#include <ESPAsyncWebServer.h> 

//************CONFIG I2C***************************************************
// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
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
//************CONFIG DATABASE*********************************************

//************CONFIG SPIFFS***********************************************
#define FORMAT_SPIFFS_IF_FAILED true
uint32_t chipId = 0;
String tempSPIFFS="";
//************CONFIG CONNECTION ******************************************
const char* ssid = "wifimich";
const char* password = "mich1983";

// Set LED GPIO
const int ledPin = 2;
String ledState;
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

//************ WEBSERVER FUNCTIOS ****************************************
String processor(const String& vr){
  Serial.println(vr);
  if(vr == "STATE"){
    if(digitalRead(ledPin)){
      ledState = "ON";
    }
    else{
      ledState = "OFF";
    }
    Serial.print(ledState);
    return ledState;
  }
  return String();
}


//**************** NFC FUNCTIONS *****************************************

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

//**************** SERVER FUNCTIONS***************************************
void configNFC(){
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

void setupWebserver(){
  WiFi.mode(WIFI_MODE_APSTA);
 
  WiFi.softAP("esp", password);
 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.softAPIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
  // Route to set GPIO to HIGH
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, HIGH);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  // Route to set GPIO to LOW
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, LOW);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/lastread", HTTP_GET, [](AsyncWebServerRequest *request){  
    request->send(200, "json/text", "{read:"+ lastReadBackup+"}");
    });

/*     server.on(//Write received status datas in actuator
      "/writeatuador",
      HTTP_POST,
      [](AsyncWebServerRequest * request){},
      NULL,
      [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      String stream;
      for (size_t i = 0; i < len; i++) {
        stream+=(char)data[i];
      }
      ;
      request->send(200,"text/html",parserJsonActuatorWrite(stream));
    });  */

  // Start server
  server.begin();
  }
//************************************************************************

//*************** SPIFFS FUNCTIONS****************************************
  void chipid() {
  //Serial.begin(115200);
  Serial.printf("\n\n---Start---\n");
  Serial.print("Chip Revision: ");
  Serial.print(ESP.getChipRevision());
  Serial.printf("\nCpuFreqMHz(): %lu", (unsigned long)ESP.getCpuFreqMHz());
  Serial.printf("\nSdkVersion: %s", ESP.getSdkVersion());
  Serial.printf("\nFlashChipSize: %lu", (unsigned long)ESP.getFlashChipSize());
  Serial.println("");
  Serial.printf("%s",(unsigned long)ESP.getEfuseMac());

}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void listFiles(String path) {
  Serial.println("- Listing files: " + path);
  SPIFFS.begin(true);
  File root = SPIFFS.open(path);
  if (!root) {
    Serial.println("- Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("- Not a directory: " + path);
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("- Dir: ");
      Serial.println(file.name());
    } else {
      Serial.print("- File: ");
      Serial.print(file.name());
      Serial.print("\tSize: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

bool writeFile(String values, String pathFile, bool appending) {
  char *mode = "w"; //open for writing (creates file if it doesn't exist). Deletes content and overwrites the file.
  if (appending) mode = "a"; //open for appending (creates file if it doesn't exist)
  Serial.println("- Writing file: " + pathFile);
  Serial.println("- Values: " + values);
  SPIFFS.begin(true);
  File wFile = SPIFFS.open(pathFile, mode);
  if (!wFile) {
    Serial.println("- Failed to write file.");
    return false;
  } else {
    wFile.println(values);
    Serial.println("- Written!");
  }
  wFile.close();
  return true;
}

String readFile(String pathFile) {
  Serial.println("- Reading file: " + pathFile);
  SPIFFS.begin(true);
  File rFile = SPIFFS.open(pathFile, "r");
  String values;
  if (!rFile) {
    Serial.println("- Failed to open file.");
  } else {
    while (rFile.available()) {
      values += rFile.readString();
    }
    Serial.println("- File values: " + values);
  }
  rFile.close();
  return values;
}

bool deleteFile(String pathFile) {
  Serial.println("- Deleting file: " + pathFile);
  SPIFFS.begin(true);
  if (!SPIFFS.remove(pathFile)) {
    Serial.println("- Delete failed.");
    return false;
  } else {
    Serial.println("- File deleted!");
    return true;
  }
}

void renameFile(String pathFileFrom, String pathFileTo) {
  Serial.println("- Renaming file " + pathFileFrom + " to " + pathFileTo);
  SPIFFS.begin(true);
  if (!SPIFFS.rename(pathFileFrom, pathFileTo)) {
    Serial.println("- Rename failed.");
  } else {
    Serial.println("- File renamed!");
  }
}

bool formatFS() {
  Serial.println("- Formatting file system...");
  SPIFFS.begin(true);
  if (!SPIFFS.format()) {
    Serial.println("- Format failed.");
    return false;
  } else {
    Serial.println("- Formatted!");
    return true;
  }
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("- message appended");
    } else {
        Serial.println("- append failed");
    }
    file.close();
}

void testFileIO(fs::FS &fs, const char * path){
    Serial.printf("Testing file I/O with %s\r\n", path);

    static uint8_t buf[512];
    size_t len = 0;
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }

    size_t i;
    Serial.print("- writing" );
    uint32_t start = millis();
    for(i=0; i<2048; i++){
        if ((i & 0x001F) == 0x001F){
          Serial.print(".");
        }
        file.write(buf, 512);
    }
    Serial.println("");
    uint32_t end = millis() - start;
    Serial.printf(" - %u bytes written in %u ms\r\n", 2048 * 512, end);
    file.close();

    file = fs.open(path);
    start = millis();
    end = start;
    i = 0;
    if(file && !file.isDirectory()){
        len = file.size();
        size_t flen = len;
        start = millis();
        Serial.print("- reading" );
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            if ((i++ & 0x001F) == 0x001F){
              Serial.print(".");
            }
            len -= toRead;
        }
        Serial.println("");
        end = millis() - start;
        Serial.printf("- %u bytes read in %u ms\r\n", flen, end);
        file.close();
    } else {
        Serial.println("- failed to open file for reading");
    }
}

void readConfig(String value){
  //FUNÇÃO DE LEITURA COM APLICAÇÃO DE JSON
    StaticJsonDocument<128> doc;

    DeserializationError error = deserializeJson(doc, value);

    if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
    }

    int total_tags = doc["total_tags"]; // 2000
    long tag_master = doc["tag_master"]; // 1351824120
    const char* ssid = doc["ssid"]; // "default0"
    const char* password = doc["password"]; // "@hfj0601"
    Serial.println(total_tags);
    Serial.println(tag_master);
    Serial.println(ssid);
    Serial.println(password);

}

void setupSPIFFS(){
 if(!SPIFFS.begin(true)){
        Serial.println("Montagem de partição Falhou!");
        return;
    }
    chipid();
    readFile( "/config.tx");
}


//***************** LOOP AND SETUP****************************************

void setup(void) {
  pinMode(ledPin, OUTPUT);
  digitalWrite(2,LOW);
 
  Serial.begin(115200);
  while (!Serial) delay(10); // for Leonardo/Micro/Zero
  
  setupSPIFFS();
  setupWebserver(); 
  configNFC();

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


