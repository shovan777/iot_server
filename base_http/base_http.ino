/**
 * BasicHTTPClient.ino
 *
 *  Created on: 24.05.2015
 *
 */

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>
#include "DHT.h"

#define USE_SERIAL Serial
#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);


WiFiMulti wifiMulti;

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

    delay(5000);
}
