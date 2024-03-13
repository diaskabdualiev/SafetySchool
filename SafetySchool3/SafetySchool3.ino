#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ESP32Servo.h>

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

unsigned long previous_time = 0;
unsigned long delay1 = 30000;  // 30 seconds delay

Servo myservo;  // создаем объект сервопривода
int servoPin = 32; // GPIO пин к которому подключен сервопривод

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
//    digitalWrite(zharyqPins[0], HIGH);
//    delay(300);
//    digitalWrite(zharyqPins[0], LOW);
//    delay(400);
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
  
  if (!Firebase.RTDB.beginStream(&stream, "/esp1"))
    Serial.printf("stream begin error, %s\n\n", stream.errorReason().c_str());
  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);

  myservo.attach(servoPin);  // ассоциируем сервопривод с пином
  myservo.write(0); // Инициализируем сервопривод в исходном положении (0 градусов)
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
  if (data.dataPath().endsWith("/door")) {
    myservo.write(data.intData() != 0 ? 90 : 0);
//    digitalWrite(zharyqPins[0], data.intData() != 0 ? HIGH : LOW);
    Serial.print("/servo door: ");
    Serial.println(data.intData());
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
