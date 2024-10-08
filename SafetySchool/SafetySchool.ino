#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>

// Rakhat
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "SafetySchool"
#define WIFI_PASSWORD "SafetySchool"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBPRfGaCYhNKFMcwKBCFtI4TLM9csQ_8Ds"

// Insert RTDB URL
#define DATABASE_URL "https://safetyschool-19925-default-rtdb.asia-southeast1.firebasedatabase.app"

// DHT Sensor configuration
#define DHTPIN 25 // Adjust according to your ESP32 pin layout
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);

// Relay pins
#define RELAY_PIN_VENTILATION 32 // Change to your ventilation relay pin
#define RELAY_PIN_HEATER 33 // Change to your heater relay pin

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
float h = 0;
float t = 0;
float maxTemp, minTemp;
bool signupOK = false;

unsigned long previous_time = 0;
unsigned long delay1 = 30000;  // 30 seconds delay

unsigned long previostime = 0;

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

void controlClimate(float currentTemperature) {

  if(currentTemperature != 0){
    if (currentTemperature >= maxTemp) {
      // Activate ventilation system
      digitalWrite(RELAY_PIN_VENTILATION, HIGH);
      digitalWrite(RELAY_PIN_HEATER, LOW);
      Serial.println("Ventilation ON");
    } else if (currentTemperature <= minTemp) {
      // Activate both ventilation and heater
      digitalWrite(RELAY_PIN_VENTILATION, HIGH);
      digitalWrite(RELAY_PIN_HEATER, HIGH);
      Serial.println("Heater and Ventilation ON");
    } else {
      // Deactivate both systems
      digitalWrite(RELAY_PIN_VENTILATION, LOW);
      digitalWrite(RELAY_PIN_HEATER, LOW);
      Serial.println("Heater and Ventilation OFF");
    }
  }
}

void loop() {

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

      if (Firebase.RTDB.getFloat(&fbdo, "/esp4/cab1/maxtemp")) {
    maxTemp = fbdo.floatData();
  } else {
    Serial.println("Failed to read max temperature threshold");
    return;
  }

  if (Firebase.RTDB.getFloat(&fbdo, "/esp4/cab1/mintemp")) {
    minTemp = fbdo.floatData();
  } else {
    Serial.println("Failed to read min temperature threshold");
    return;
  }

    
   

    // Send temperature and humidity data to Firebase
    Firebase.RTDB.setFloat(&fbdo, "/esp4/cab1/temp", t);
    Firebase.RTDB.setFloat(&fbdo, "/esp4/cab1/vlazh", h);

    // Control climate based on current temperature

  }


  if(millis() - previostime >= 2000){
    h = dht.readHumidity();
    t = dht.readTemperature();
  
    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
  
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" Humidity: ");
    Serial.println(h);
    controlClimate(t);
    previostime = millis();
  }
  
  unsigned long current_time = millis();
  if ((WiFi.status() != WL_CONNECTED) && (current_time - previous_time >= delay1)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WIFI network");
    WiFi.disconnect();
    WiFi.reconnect();
    previous_time = current_time;
  }
}
