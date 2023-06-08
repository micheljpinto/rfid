#include <ESPAsyncWebServer.h>
#include <WiFi.h> 
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include "FS.h"

const char* ssid =      "default0";
const char* password =  "@hfj0601";
const char* ssid_ex =   "esp01";
const char* pass_ex=    "mich1983";
const char* usr="Admin";
const char* senha="mich1983";
String tagTemp=""; //variável temporária usada para armazenar tag vinda de webIHM
extern boolean readerDisabled;
extern String lastReadBackup;
extern long timeLastCardRead;
extern int DELAY_OPEN;
// Set LED GPIO
extern int OPEN_GATE;
extern int BT_MANUAL;
extern int LED_OK;
extern volatile bool handOpen;
String ledState;
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String toggleLed(){
  digitalWrite(OPEN_GATE,LOW);
  timeLastCardRead=millis();
  readerDisabled=1; 
  handOpen=0;
  digitalWrite(LED_OK,HIGH);
  return "ok";
}

int confirmPass(String input){
  
  StaticJsonDocument<96> doc;

  DeserializationError error = deserializeJson(doc, input);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return false;
  }
  const char* user = doc["user"]; // "michel"
  const char* pass = doc["pass"]; // "123456"
  String tag = doc["tag"]; // "38484958"
  tagTemp=tag;

  if (!(*user==*usr) || !(*pass==*senha))
  {
    //Usuário ou senha incorretos
    return 0;
  }
  else if(searchTag(tagTemp))
  {
    //Tag existente 
    return 1;
  } else {
    // Gravar tag
    return 2; 
  }    
}

void setupWifi(){

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid_ex, pass_ex);
    //WiFi.hostname("esp32");
    WiFi.begin(ssid, password);
    int count=0;
    while (WiFi.status() != WL_CONNECTED) {
       if(count==3)
         break;
       delay(1000);
       Serial.println("Connecting to WiFi..");
       count++;
    }

    // Print ESP32 Local IP Address
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.softAPIP());
}

void setupWebserver(){
   // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false);
  });
  // Route to load documents file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script.js", "text/json");
  });
  
     server.on("/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/bootstrap.min.js", "text/json");
  });

       server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/bootstrap.min.css", "text/css");
  });
  
    server.on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/jquery.min.js", "text/json");
  });
  //----------------------------------------------------------------------
 
  server.on("/lastread", HTTP_GET, [](AsyncWebServerRequest *request){  
    request->send(200, "text/json", lastReadBackup);
    });
  
  server.on("/list", HTTP_GET, [](AsyncWebServerRequest *request){  
    request->send(200, "text/json", readFile("/tags.txt"));
    });
  //PAGINA DE ADD TAG
  server.on("/addtag", HTTP_GET, [](AsyncWebServerRequest *request){  
    request->send(SPIFFS, "/addtag.html",String(), false);
    });
  //ADICIONA TAG
  server.on("/addtag", HTTP_POST, [](AsyncWebServerRequest * request){},
    NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    String stream;
    for (size_t i = 0; i < len; i++) {
      stream+=(char)data[i];
    }
    switch (confirmPass(stream))
    {
    case 0:
      Serial.println("Usuário ou senha incorretos");
      request->send(200, "javascript/text", "console.log(\"Usuário ou senha incorretos\")");
      break;

      case 1:
      Serial.println("Tag ja existe");
      request->send(200, "javascript/text", "console.log(\"Tag ja existe\")");
      break;
    default:
      Serial.println("Cadastrar tags");
      writeFile(tagTemp+",","/tags.txt",true);
      request->send(200, "javascript/text", "console.log(\"Cadastrar tags\")");
      break;
    }       
    
    Serial.println(stream);
  }); 
    
    //REMOVE TAG
    server.on("/rmtag", HTTP_POST, [](AsyncWebServerRequest * request){},
    NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    String stream;
    for (size_t i = 0; i < len; i++) {
      stream+=(char)data[i];
    }
    switch (confirmPass(stream))
    {
    case 0:
      Serial.println("Usuário ou senha incorretos");
      request->send(200, "javascript/text", "console.log(\"Usuário ou senha incorretos\")");
      break;

      case 1:
      deleteTag(tagTemp);
      request->send(200, "javascript/text", "console.log(\"Tag ja existe\")");
      Serial.println("Tag descadastrada");
      break;
    default:
      request->send(200, "javascript/text", "console.log(\"Descadastrar tag\")");
      Serial.println("Tag inexistente");
      break;
    }       
    
    Serial.println(stream);
  }); 
      
  server.on("/atuador", HTTP_GET, [](AsyncWebServerRequest *request){
    //digitalWrite(2,HIGH);
    //timeLastCardRead=millis();
    //readerDisabled=1;
    request->send(200, "text/html",toggleLed());
       
    
  });

  // Start server
  server.begin();
  }

  