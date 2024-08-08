/*
ACCESS ALL DATA:
  https://data.cdc.gov/resource/9hdi-ekmb.json

NOTES ABOUT THE API:
  - Data starts at very first death from covid-19 and cycles through all weeks until today. 
*/

// CERAMIC HEATER
#include "CeramicHeater.h"
#define ESP32 

// pin definitions for ESP32
#define ENABLE_PIN  12
#define ERROR_PIN   27
#define TEMP_PIN    34

// debug prints
#define DEBUG       0
// target temperature (max set point is 84)
#define TARGET_TEMP 84
// hysterisis for temperature control
#define DELTA_TEMP  4
// temp considered "off"
#define OFF_TEMP    30

CeramicHeater heater(ENABLE_PIN, ERROR_PIN, TEMP_PIN);


//API
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoHttpClient.h>

#include <ArduinoJson.h> // to de-serialize the JSON file


// ACCESS INTERNET
//const char* ssid = "Verizon_7YV47L";
//const char* password = "hotly-oak7-sock";
const char* ssid = "BFADT-IoT";
const char* password = "bfaisthebest";

// URL with parameter to only access data with social vulnerability index of All US Counties
String serverName = "https://data.cdc.gov/resource/9hdi-ekmb.json?social_vulnerability_index=All%20U.S.%20Counties";

// STORING TIME INFORMATION
int api_week_to_check = 0;  // starting at the beginning of the pandemic
int api_year_to_check = 2020; // starting at the beginning of the pandemic
int death_counts_this_week = 0;     // start at 0

void setup() {
  Serial.begin(115200); 
  // Ceramic Heater
  heater.setup(TARGET_TEMP, DELTA_TEMP, OFF_TEMP, DEBUG);

  // INITIALIZING WIFI CONNECTION
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
  Serial.println("");
}

void loop() {

  if (death_counts_this_week == 0){
    Serial.println("All deaths of the previous week have been processed - accessing new data now.");

    // increase the week 
    if (api_year_to_check == 2020 && api_week_to_check == 53){
      api_year_to_check = 2021;
      api_week_to_check = 1;
      }
    else if (api_year_to_check == 2021 && api_week_to_check == 52){
      api_year_to_check = 2022;
      api_week_to_check = 1;
      }
    else if (api_year_to_check == 2022 && api_week_to_check == 52){
      api_year_to_check = 2023;
      api_week_to_check = 1;
      }
    else{
      api_week_to_check += 1;
      }
    
    Serial.print("Now we are checking for week  ");
    Serial.print(api_week_to_check);
    Serial.print("  in year  ");
    Serial.print(api_year_to_check);
    Serial.println("");

    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      // this serverPath will give us our week's reading of all US Counties
      String serverPath = serverName + "&mmwr_week=" + api_week_to_check + "&mmwr_year=" + api_year_to_check;

      http.begin(serverPath.c_str());
      int httpResponseCode = http.GET(); // Send HTTP GET request
      
      if (httpResponseCode>0) {
/* 
  to get response code if API is not working:
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);
*/
        String data = http.getString(); // all data is stored here

        StaticJsonDocument<512> doc;
        deserializeJson(doc,data); // let's deserialize it so we can access all data inside

        death_counts_this_week = doc[0]["covid_19_deaths"]; // accessing the "total deaths" data point

        Serial.print("Total Deaths in this week are  ");
        Serial.println(death_counts_this_week);
        Serial.println("");
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
  }

  //HERE THE HEATING ELEMENT COMES TO PLAY:
  if (death_counts_this_week > 0){
    
    //One cycle of the ceramic heater: Turn hot, let cool down, representing one death

    // cycle the heater
      heater.cycle();
      // increment the local counter
      Serial.print("   cycles this week: ");
      Serial.println(death_counts_this_week);
      // keep track of the total cycles
      heater.incrementCycleCount();
      Serial.print("   total covid deaths processed: ");
      Serial.println(heater.getCycleCount());
      Serial.println("");

    // then decrease one death from the total amount of deaths.
    death_counts_this_week -= 1;
    }
}
