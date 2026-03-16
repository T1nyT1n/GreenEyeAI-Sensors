#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <cstdlib>  // для rand(), srand()
#include <ctime>    // для time()

// флаг для вывода
#define DEBUG
#define START_SENSOR 0

typedef unsigned char u8;
typedef   signed char i8;

typedef unsigned short u16;
typedef   signed short i16;

typedef unsigned int u32;
typedef   signed int i32;

// typedef unsigned long u64;
// typedef   signed long i64;


const char* ssid = "VNE-N41";
const char* password = "34670000";


WiFiClient client;
const char *host = "10.21.36.131";
const u16 port = 5000;


typedef struct Sensor {
  float value;
  u64 last_time;
  u64 delay;
  float base_min; float base_max;
  float shift_min; float shift_max;
};

Sensor s[] = {
  { 0.0, 0,  100, -1000.0, 1000.0,  -10.0,  10.0 }, // pH sensor
  { 0.0, 0, 1000,     0.0, 1024.0, -200.0, 200.0 }, // light sensor
  { 0.0, 0,  100,   -30.0,   50.0,   -1.0,   1.0 }, // T sensor
};

auto size = sizeof(s) / sizeof(Sensor);


void connectToWifi() {
  Serial.print("Подключение к Wi-Fi: ");
  digitalWrite(D0, LOW);
  digitalWrite(D1, LOW);
  WiFi.begin(ssid, password);
  while (!WiFi.isConnected()) {
    auto status = WiFi.status();
    if (status == WL_NO_SSID_AVAIL) {
      Serial.println(" Ошибка\nДанный SSID не доступен");
      return;
    }
    if (status == WL_WRONG_PASSWORD) {
      Serial.println(" Ошибка\nПароль не верный");
      return;
    }
    Serial.print(".");
    delay(500);
  }
  Serial.println(" Готово");
  digitalWrite(D0, HIGH);
}


void connectToServer() {
  Serial.print("Подключение к серверу: ");
  digitalWrite(D1, LOW);
  while (!client.connected()) {
    // проверяем, действительно ли соединение
    // и выходим если оборвалось во премя
    // подключения к серверу
    if (!WiFi.isConnected()) {
      Serial.println(" Ошибка\nWiFi не подключен");
      return;
    }
    client.connect(host, port);
    Serial.print(".");
    delay(500); 
  }
  Serial.println(" Готово");
  digitalWrite(D1, HIGH);
}


float getRandom(float min, float max) {
  float x = rand() / (float) RAND_MAX;
  return min + (max - min) * x;
}


void sendSensorData(int sensor_id, float data) {
  String body = "{\"sensor_id\":" + String(sensor_id) +
                ",\"data\":" + String(data) + "}";

  int contentLength = body.length();

  String req = "POST /data HTTP/1.1\r\n";
  req += "Host: " + String(host) + "\r\n";
  req += "Content-Type: application/json\r\n";
  req += "Content-Length: " + String(contentLength) + "\r\n";
  req += "\r\n";
  req += body;

  client.print(req);
}


void setup() {
  srand(time(nullptr));
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);

  for (u32 i = 0; i < size; i++)
    s[i].value = getRandom(s[i].base_min, s[i].base_max);
}


void loop() {
  if (!WiFi.isConnected()) connectToWifi();
  if (!client.connected()) connectToServer();

  // проходим по каждому виртуальному сенсору
  for (u32 i = 0; i < size; i++) {
    auto sensor = s[i];
    if (millis() - sensor.last_time < sensor.delay) continue;
    sensor.value += getRandom(sensor.shift_min, sensor.shift_max);
    sendSensorData(i, sensor.value);
  }
}
