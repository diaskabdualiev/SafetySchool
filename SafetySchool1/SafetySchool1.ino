#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

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
FirebaseData stream;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

const int zharyqPins[15] = {32, 33, 25, 26, 27, 14, 13, 23, 22, 21, 19, 18, 17, 16, 4};


void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  for (int i = 0; i < 15; i++) {
    pinMode(zharyqPins[i], OUTPUT);
    digitalWrite(zharyqPins[i], LOW);
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

  if (!Firebase.RTDB.beginStream(&stream, "/esp3"))
    Serial.printf("stream begin error, %s\n\n", stream.errorReason().c_str());
  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);
}


void streamTimeoutCallback(bool timeout)
{
  if(timeout){
    // Stream timeout occurred
    Serial.println("Stream timeout, resume streaming...");
  }  
}

void streamCallback(FirebaseStream data)
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
