#include <WiFi.h>
//#include <WiFiClient.h>
//#include <WebServer.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "POCO X3";
const char* password = "shovan12345";

const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "password";


//WebServer server(80);
AsyncWebServer server(80);


const int led = 13;

void handleRoot(AsyncWebServerRequest *request) {
//  server.send(200, "text/html", "<form action=\"/wificred\"><label for=\"ssid\">SSID:</label><input type=\"text\" id=\"ssid\" name=\"ssid\"><br><label for=\"password\">PASSWORD:</label><input type=\"text\" id=\"password\" name=\"password\"><br> <input type=\"submit\" value=\"Submit\"></form>");
  request->send_P(200, "text/html", "<form action=\"/wificred\"><label for=\"ssid\">SSID:</label><input type=\"text\" id=\"ssid\" name=\"ssid\"><br><label for=\"password\">PASSWORD:</label><input type=\"text\" id=\"password\" name=\"password\"><br> <input type=\"submit\" value=\"Submit\"></form>");
}

void handleNotFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
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
    }
    if (request->hasParam(PARAM_INPUT_2)) {
      inputPswd = request->getParam(PARAM_INPUT_2)->value();
    }
  Serial.println(inputSSID);
  Serial.println(inputPswd);
    
    request->send(200, "text/html", "you provided following data <br> SSID: "+inputSSID+"<br>Password: " + inputPswd);
  });


  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
//  server.handleClient();
//  delay(2);//allow the cpu to switch to other tasks
}