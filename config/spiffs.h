//#include <ESPAsyncWebServer.h>
//#include <WiFi.h> 
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include "FS.h"



//************CONFIG SPIFFS***********************************************
#define FORMAT_SPIFFS_IF_FAILED true
uint32_t chipId = 0;
String tempSPIFFS="";
char* configFile="/config.txt";

//*************** SPIFFS FUNCTIONS****************************************
  void chipid() {
  //Serial.begin(115200);
  Serial.printf("\n\n---Start---\n");
  Serial.print(F("Chip Revision: "));
  Serial.print(ESP.getChipRevision());
  Serial.printf("\nCpuFreqMHz(): %lu", (unsigned long)ESP.getCpuFreqMHz());
  Serial.printf("\nSdkVersion: %s", ESP.getSdkVersion());
  Serial.printf("\nFlashChipSize: %lu", (unsigned long)ESP.getFlashChipSize());
  Serial.println("");
  Serial.print(ESP.getEfuseMac());

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

String readFile(char *pathFile) {
  Serial.print("- Reading file: ");
  Serial.println(pathFile);
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
    readConfig(readFile(configFile));
}
