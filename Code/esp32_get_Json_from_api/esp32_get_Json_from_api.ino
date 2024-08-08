#include "WiFi.h" 
#include <HTTPClient.h>
#include <ArduinoJson.h>

// 1. Add these two libraries (you need to install them) 
#define WIFI_NETWORK "Verizon_7YV47L" 
#define WIFI_PASSWORD "put password here"

#define WIFI_TIMEOUT_MS 20000

void connectToWiFi(){
  Serial.println("  "); 
  Serial.println("WIFI Connection:"); 
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);
  
  // keeping track of how long the esp32 has been trying to connect to our WIFI 
  unsigned long startAttemptTime = millis();
  // keeps looping as long as we don't have a WiFi connecion and haven't exceeded our timeout. 
  while(WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS){
    Serial.println("    ESP 32 is trying to connect to WiFi");
    delay(100);
  }

  // after this While loop, we are either connected to Wifi, or we have timed out and are not connected. 
  if (WiFi.status() != WL_CONNECTED){
    Serial.println("    Connection to WiFi has failed"); 
  }

  if (WiFi.status() == WL_CONNECTED){
    Serial.println("    esp32 is connected to WiFi at IP Adress:  ");
    Serial.print("    ");
    Serial.println(WiFi.localIP());
    
    // 2. create HTTP client
    HTTPClient client;

    // 3. specify the URL
    client.begin("https://jsonplaceholder.typicode.com/posts/1/comments");

    // 4. place it into an integer called httpCode
    int httpCode = client.GET();

    // 5. in order to know whether the request was successful, add the below code.
    if (httpCode >0){
      String payload = client.getString();
      Serial.print("Statuscode:");
      Serial.println(String(httpCode));
      Serial.println(payload);
      
      // in order to deserialize out JSON file we need to modify the string:
      char json[500];
      payload.replace(" ", "");
      payload.replace("\n", "");
      payload.trim();
      payload.remove(0,1);
      payload.toCharArray(json, 500);

      StaticJsonDocument<200> doc;
      deserializeJson(doc, json);

      // example: retrieveing some data:
      int id = doc["id"];
      const char* email = doc["email"];
      // example: printing that data:
      Serial.print(String(id));
      Serial.print("  -  ");
      Serial.println(String(email));

      // end client connection
      client.end();

      
      }else{
        Serial.println("Error on HTTP request");
      }
    
  }
  // 6. add a delay here - otherwise ESP32 is too quick to check
  delay(10000);
 
  }

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // 5. call our WiFi function in setup:
  connectToWiFi();

}

void loop() {
  // put your main code here, to run repeatedly:

}
