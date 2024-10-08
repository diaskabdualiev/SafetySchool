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
#define API_KEY "AIzaSyBPRfGaCYhNKFMcwKBCFtI4TLM9csQ_8Ds"

// Insert RTDB URL
#define DATABASE_URL "https://safetyschool-19925-default-rtdb.asia-southeast1.firebasedatabase.app"


// Define Firebase Data object
FirebaseData stream;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

const int zharyqPins[15] = {32, 23, 22, 16, 4, 33, 25, 27, 14, 26, 18, 13, 19, 21 , 17};

unsigned long previous_time = 0;
unsigned long delay1 = 30000;  // 30 seconds delay

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 15; i++) {
    pinMode(zharyqPins[i], OUTPUT);
    digitalWrite(zharyqPins[i], LOW);
  }
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(zharyqPins[0], HIGH);
    delay(300);
    digitalWrite(zharyqPins[0], LOW);
    delay(400);
  }
  digitalWrite(zharyqPins[0], LOW );
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
  if (timeout) {
    // Stream timeout occurred
    Serial.println("Stream timeout, resume streaming...");
  }
}

void streamCallback(FirebaseStream data)
{
  Serial.println("Получены данные из потока...");
  Serial.println(data.streamPath());
  Serial.println(data.dataPath());
  Serial.println(data.dataType());

  // Обработка для zharyq1 до zharyq15
  if (data.dataPath().endsWith("/zharyq1")) {
    digitalWrite(zharyqPins[0], data.intData() != 0 ? HIGH : LOW);
    Serial.println("/zharyq1");
  } else if (data.dataPath().endsWith("/zharyq2")) {
    digitalWrite(zharyqPins[1], data.intData() != 0 ? HIGH : LOW);
    Serial.println("/zharyq2");
  } else if (data.dataPath().endsWith("/zharyq3")) {
    digitalWrite(zharyqPins[2], data.intData() != 0 ? HIGH : LOW);
    Serial.println("/zharyq3");
  } else if (data.dataPath().endsWith("/zharyq4")) {
    digitalWrite(zharyqPins[3], data.intData() != 0 ? HIGH : LOW);
    Serial.println("/zharyq4");
  } else if (data.dataPath().endsWith("/zharyq5")) {
    digitalWrite(zharyqPins[4], data.intData() != 0 ? HIGH : LOW);
    Serial.println("/zharyq5");
  } else if (data.dataPath().endsWith("/zharyq6")) {
    digitalWrite(zharyqPins[5], data.intData() != 0 ? HIGH : LOW);
    Serial.println("/zharyq6");
  } else if (data.dataPath().endsWith("/zharyq7")) {
    digitalWrite(zharyqPins[6], data.intData() != 0 ? HIGH : LOW);
    Serial.println("/zharyq7");
  } else if (data.dataPath().endsWith("/zharyq8")) {
    digitalWrite(zharyqPins[7], data.intData() != 0 ? HIGH : LOW);
    Serial.println("/zharyq8");
  } else if (data.dataPath().endsWith("/zharyq9")) {
    digitalWrite(zharyqPins[8], data.intData() != 0 ? HIGH : LOW);
    Serial.println("/zharyq9");
  } else if (data.dataPath().endsWith("/zharyq10")) {
    digitalWrite(zharyqPins[9], data.intData() != 0 ? HIGH : LOW);
    Serial.println("/zharyq10");
  } else if (data.dataPath().endsWith("/zharyq11")) {
    digitalWrite(zharyqPins[10], data.intData() != 0 ? HIGH : LOW);
    Serial.println("/zharyq11");
  } else if (data.dataPath().endsWith("/zharyq12")) {
    digitalWrite(zharyqPins[11], data.intData() != 0 ? HIGH : LOW);
    Serial.println("/zharyq12");
  } else if (data.dataPath().endsWith("/zharyq13")) {
    digitalWrite(zharyqPins[12], data.intData() != 0 ? HIGH : LOW);
    Serial.println("/zharyq13");
  } else if (data.dataPath().endsWith("/zharyq14")) {
    digitalWrite(zharyqPins[13], data.intData() != 0 ? HIGH : LOW);
    Serial.println("/zharyq14");
  } else if (data.dataPath().endsWith("/zharyq15")) {
    digitalWrite(zharyqPins[14], data.intData() != 0 ? HIGH : LOW);
    Serial.println("/zharyq15");
  }

  // Выводим значение
  Serial.println(data.intData());
}
void loop() {

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

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
