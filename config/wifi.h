#include <ESPAsyncWebServer.h>
#include <WiFi.h> 
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include "FS.h"


const char* ssid =      "wifimich";
const char* password =  "mich1983";
const char* ssid_ex =   "esp01";
const char* pass_ex=    "mich1983";

extern String lastReadBackup;
// Set LED GPIO
const int ledPin = 2;
String ledState;
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String toggleLed(const String& vr){
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


void setupWifi(){

    WiFi.mode(WIFI_MODE_APSTA);

    WiFi.softAP(ssid_ex, pass_ex);
    WiFi.hostname("esp3201");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }
    // Print ESP32 Local IP Address
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.softAPIP());
}
void setupWebserver(){
   // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, toggleLed);
  });
  // Route to load documents file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
    server.on("/style2.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style2.css", "text/css");
  });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script.js", "text/json");
  });
  //----------------------------------------------------------------------

  // Route to set GPIO to HIGH
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, HIGH);    
    request->send(SPIFFS, "/index.html", String(), false, toggleLed);
  });
  // Route to set GPIO to LOW
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, LOW);    
    request->send(SPIFFS, "/index.html", String(), false, toggleLed);
  });

  server.on("/lastread", HTTP_GET, [](AsyncWebServerRequest *request){  
    request->send(200, "text/json", "{read:"+ lastReadBackup+"}");
    });
  server.on("/addtag", HTTP_GET, [](AsyncWebServerRequest *request){  
    request->send(SPIFFS, "/addtag.html",String(), false);
    });

    server.on("/addtag", HTTP_POST, [](AsyncWebServerRequest * request){},
      NULL,
      [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      String stream;
      for (size_t i = 0; i < len; i++) {
        stream+=(char)data[i];
      }
      ;
      request->send(200, "text",String({"status:OK"}));
      Serial.println(stream);
    }); 

  // Start server
  server.begin();
  }

  