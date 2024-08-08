#include "WiFi.h" // 1. include the WiFi header - this library is installed automatically when you set up the ESP32.

// 2. define two constants: Wifi Network Name and Password of that WiFi network
#define WIFI_NETWORK "your router name" // fill in with your own Wifi Name
#define WIFI_PASSWORD "your password" // fill in with your own Wifi Password

// 3. define wifi timeout: how long should the ESP32 attempt to connect to your wifi. 
#define WIFI_TIMEOUT_MS 20000 // I want mine to try for 20s = 20'000 ms

// 4. create function that takes care of connecting to WiFi (keeps code clean)
void connectToWiFi(){
  Serial.println("  "); 
  Serial.println("WIFI Connection:"); 
  WiFi.mode(WIFI_STA);// set ESP32 to station mode in order to connect to existing network
  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD); // this line starts the WiFi connection process
  
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
    Serial.print(WiFi.localIP());
  }
 
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
