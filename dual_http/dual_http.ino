#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include "DHT.h"

#define USE_SERIAL Serial
#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);


WiFiMulti wifiMulti;

const char* ssid = "POCO X3";
const char* password = "shovan12345";

const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "password";

AsyncWebServer server(80);

void handleRoot(AsyncWebServerRequest *request) {
//  server.send(200, "text/html", "<form action=\"/wificred\"><label for=\"ssid\">SSID:</label><input type=\"text\" id=\"ssid\" name=\"ssid\"><br><label for=\"password\">PASSWORD:</label><input type=\"text\" id=\"password\" name=\"password\"><br> <input type=\"submit\" value=\"Submit\"></form>");
  request->send_P(200, "text/html", "<form action=\"/wificred\"><label for=\"ssid\">SSID:</label><input type=\"text\" id=\"ssid\" name=\"ssid\"><br><label for=\"password\">PASSWORD:</label><input type=\"text\" id=\"password\" name=\"password\"><br> <input type=\"submit\" value=\"Submit\"></form>");
}

void handleNotFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }
//  Serial.println("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  file.close();
//  Serial.println(fileContent);
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

// Replaces placeholder with stored values
String processor(const String& var){
  //Serial.println(var);
  if(var == "ssid"){
    return readFile(LittleFS, "/inputssid.txt");
  }
  else if(var == "password"){
    return readFile(LittleFS, "/inputpswd.txt");
  }
  return String();
}

void setup() {

    USE_SERIAL.begin(115200);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

   wifiMulti.addAP("POCO X3", "shovan12345");
    if(!LittleFS.begin(true)){
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }

   // Wait for connection
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", HTTP_GET, handleRoot);

  server.on("/hello", HTTP_GET, [](AsyncWebServerRequest *request) {
//    server.send(200, "text/plain", "Hi!!! ur friendly esp here!!");
    request->send_P(200, "text/plain", "Hi!!! ur friendly esp here!!");
  });

  server.on("/wificred", HTTP_GET, [](AsyncWebServerRequest *request) {
    String inputSSID;
    String inputPswd;
    if (request->hasParam(PARAM_INPUT_1)) {
      inputSSID = request->getParam(PARAM_INPUT_1)->value();
      writeFile(LittleFS, "/inputssid.txt", inputSSID.c_str());
    }
    if (request->hasParam(PARAM_INPUT_2)) {
      inputPswd = request->getParam(PARAM_INPUT_2)->value();
      writeFile(LittleFS, "/inputpswd.txt", inputPswd.c_str());
    }
  Serial.println(inputSSID);
  Serial.println(inputPswd);
    
    request->send(200, "text/html", "you provided following data <br> SSID: "+inputSSID+"<br>Password: " + inputPswd);
  });


  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

    USE_SERIAL.printf("starting dht sensor");
    dht.begin();

//    if((wifiMulti.run() == WL_CONNECTED)) {
//      
//      }


}

float* measureDHT() {
    delay(5000);
    static float dataArr[2];
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    dataArr[0] = -1;
    dataArr[0] = -1;
    return dataArr;
    }

    // Compute heat index in Fahrenheit (the default)
    float hif = dht.computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);

    dataArr[0] = hif;
    dataArr[1] = hic;

    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C "));
    Serial.print(f);
    Serial.print(F("°F  Heat index: "));
    Serial.print(hic);
    Serial.print(F("°C "));
    Serial.print(hif);
    Serial.println(F("°F"));
    return dataArr;
}

void loop() {
//    float* dataArr = measureDHT();
    // wait for WiFi connection
    // float dataArr[2];
    // dataArr[0] = 1.1;
    // dataArr[1] = 1.2;
    if((wifiMulti.run() == WL_CONNECTED)) {
        float* dataArr = measureDHT();

        delay(500);

        HTTPClient http;

        USE_SERIAL.print("[HTTP] begin...\n");
        // configure traged server and url
        //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
        //uncomment below for get okkkkkk
        //http.begin("http://192.168.1.69:8000/data/11"); //HTTP
        //uncomment for post okkk
        http.begin("http://192.168.1.65:8000/update/12");

        http.addHeader("Content-Type", "application/json");
        
        // String httpRequestData = "{\"temp\":\"18\", \"humidity\":\"0.5\"}";
        char *httpRequestDataC = (char*)malloc(40 * sizeof(char));

        sprintf(httpRequestDataC, "{\"temp\":\"%f\",\"humidity\":\"%f\"}", dataArr[0], dataArr[1]);
        String httpRequestData = httpRequestDataC;
        
        USE_SERIAL.printf("[HTTP] GET...\n %s", httpRequestData.c_str());

        USE_SERIAL.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        //int httpCode = http.GET();

        // start post request
        int httpCode = http.POST(httpRequestData);

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                USE_SERIAL.println(payload);
            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }
    String yourInputSSID = readFile(LittleFS, "/inputssid.txt");
    Serial.print("*** Your inputssid: ");
    Serial.println(yourInputSSID);
    //  
    String yourInputPassword = readFile(LittleFS, "/inputpswd.txt");
    Serial.print("*** Your inputpassword: ");
    Serial.println(yourInputPassword);

    delay(5000);
}
