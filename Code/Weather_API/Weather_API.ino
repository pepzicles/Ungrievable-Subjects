
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

// set up your wifi name and password
const char* ssid = "Verizon_7YV47L";
const char* password = "wifi password";


// Enter your API-Key code (from Weather Website)
String openWeatherMapApiKey = "aa7947c7527e5de00129657d624e1622";

// Add the Coordinates you want to retrieve weather information from
String longitude = "40.712";
String latitude = "74.0060";

// Timer for API Call
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned: 10 minutes= (600000)
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

String jsonBuffer;

void setup() {
  Serial.begin(115200);
  
  // Setting up the WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");
}

void loop() {
  // Send an HTTP GET request
  if ((millis() - lastTime) > timerDelay) {
    // Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      // this is the API URL
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?lat=" + latitude + "&lon=" + longitude + "&appid=" + openWeatherMapApiKey;
      
      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);
      JSONVar myObject = JSON.parse(jsonBuffer);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
      // retrieve specific information like this!
    
//      Serial.print("JSON object = ");
//      Serial.println(myObject);
//      Serial.print("Temperature: ");
//      Serial.println(myObject["main"]["temp"]);
//      Serial.print("Pressure: ");
//      Serial.println(myObject["main"]["pressure"]);
//      Serial.print("Humidity: ");
//      Serial.println(myObject["main"]["humidity"]);
//      Serial.print("Wind Speed: ");
//      Serial.println(myObject["wind"]["speed"]);

Serial.println(" ----start---- ");
//Serial.println(myObject["timezone"]);
Serial.println(myObject["weather"][0]["main"]); // do like this if there is an extra set of []
Serial.println(" ----end---- ");

    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}
