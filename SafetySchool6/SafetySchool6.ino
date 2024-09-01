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

// DHT Sensor configuration
#define DHTPIN 33 // Adjust according to your ESP32 pin layout
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);

// Relay pins
#define RELAY_PIN_PUMP 25 // Change to your pump relay pin

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
float h = 0;
float t = 0;
float minTemp = 4000;
bool signupOK = false;

unsigned long previous_time = 0;
unsigned long delay1 = 30000;  // 30 seconds delay

unsigned long previostime = 0;


#include <ESP32Servo.h>

Servo myservo;  // Создаем объект сервопривода

const int trigPin = 14;
const int echoPin = 27;
const int servoPin = 13;

long duration;
int distance;
const int openPosition = 100;   // Угол открытия сервопривода
const int closePosition = 0; // Угол закрытия сервопривода
unsigned long previousMillis = 0; 
const long interval = 5000;   // Интервал времени для автоматического закрытия (5 секунд)


void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN_PUMP, OUTPUT);
  pinMode(26, OUTPUT);
  // Ensure relays are OFF initially
  digitalWrite(RELAY_PIN_PUMP, LOW);

  dht.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(26, HIGH);
    delay(300);
    digitalWrite(26, LOW);
    delay(300);
  }
  digitalWrite(26, LOW);
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

  myservo.attach(servoPin);  // Подключаем сервопривод к указанному пину
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(115200);      // Начинаем серийную связь на скорости 115200 бод
  myservo.write(closePosition); // Закрываем мусорку при запуске
}

void controlClimate(float currentTemperature) {

  if (currentTemperature != 0) {
    if (currentTemperature <= minTemp) {
      // Activate both ventilation and heater
      digitalWrite(RELAY_PIN_PUMP, HIGH);
      Serial.println("PUMP ON");
    } else {
      // Deactivate both systems
      digitalWrite(RELAY_PIN_PUMP, LOW);
      Serial.println("ELSE PUMP OFF");
    }
  }
}

void loop() {

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    if (Firebase.RTDB.getFloat(&fbdo, "/esp7/minvlazh")) {
      minTemp = fbdo.floatData();
    } else {
      Serial.println("Failed to read min temperature threshold");
      return;
    }




    // Send temperature and humidity data to Firebase
    Firebase.RTDB.setFloat(&fbdo, "/esp7/temp", t);
    Firebase.RTDB.setFloat(&fbdo, "/esp7/vlazh", h);

    // Control climate based on current temperature

  }


  if (millis() - previostime >= 2000) {
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

    previostime = millis();
    int sensorValue = analogRead(32); // Считываем значение с аналогового пина 34
    Serial.print("Значение сенсора: ");
    Serial.print(sensorValue);
    Firebase.RTDB.setFloat(&fbdo, "/esp7/datchikPochvy", sensorValue);
    controlClimate(sensorValue);
  }
    unsigned long currentMillis = millis();

  // Отправляем импульс
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Читаем длительность эхо-сигнала
  duration = pulseIn(echoPin, HIGH);
  
  // Рассчитываем расстояние
  distance = duration * 0.034 / 2;
  
  // Проверяем, находится ли что-то в пределах 10 см
  if (distance <= 10) {
    // Если да, открываем мусорку
    myservo.write(openPosition);
    previousMillis = currentMillis;  // Сброс таймера
  } else if (currentMillis - previousMillis >= interval) {
    // Если прошло более 5 секунд с момента последнего открытия, закрываем мусорку
    myservo.write(closePosition);
  }

  // Выводим расстояние для отладки
  Serial.print("Distance: ");
  Serial.println(distance);
  
  unsigned long current_time = millis();
  if ((WiFi.status() != WL_CONNECTED) && (current_time - previous_time >= delay1)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WIFI network");
    WiFi.disconnect();
    WiFi.reconnect();
    previous_time = current_time;
  }
}
