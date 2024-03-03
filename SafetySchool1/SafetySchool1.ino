#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "SafetySchool"
#define WIFI_PASSWORD "SafetySchool"

// Insert Firebase project API Key
#define API_KEY "AIzaSyCI9EJ6dAA-XN8wYl8Y2fiSZNJGX5AEqmM"

// Insert RTDB URL
#define DATABASE_URL "https://safetyschool-5bb16-default-rtdb.asia-southeast1.firebasedatabase.app"


// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN_VENTILATION, OUTPUT);
  pinMode(RELAY_PIN_HEATER, OUTPUT);
  // Ensure relays are OFF initially
  digitalWrite(RELAY_PIN_VENTILATION, LOW);
  digitalWrite(RELAY_PIN_HEATER, LOW);

  dht.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

Firebase.setStreamCallback(fbdo, streamCallback, streamTimeoutCallback);

if (!Firebase.beginStream(fbdo, "/test/data"))
{
  // Could not begin stream connection, then print out the error detail
  Serial.println(fbdo.errorReason());
}

void streamTimeoutCallback(bool timeout)
{
  if(timeout){
    // Stream timeout occurred
    Serial.println("Stream timeout, resume streaming...");
  }  
}

void streamCallback(StreamData data)
{

  // Print out all information

  Serial.println("Stream Data...");
  Serial.println(data.streamPath());
  Serial.println(data.dataPath());
  Serial.println(data.dataType());

  // Print out the value
  // Stream data can be many types which can be determined from function dataType

  if (data.dataTypeEnum() == firebase_rtdb_data_type_integer)
      Serial.println(data.to<int>());
  else if (data.dataTypeEnum() == firebase_rtdb_data_type_float)
      Serial.println(data.to<float>(), 5);
  else if (data.dataTypeEnum() == firebase_rtdb_data_type_double)
      printf("%.9lf\n", data.to<double>());
  else if (data.dataTypeEnum() == firebase_rtdb_data_type_boolean)
      Serial.println(data.to<bool>()? "true" : "false");
  else if (data.dataTypeEnum() == firebase_rtdb_data_type_string)
      Serial.println(data.to<String>());
}

void loop() {

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    
  }
}
