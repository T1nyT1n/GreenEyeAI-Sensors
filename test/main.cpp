#include <Arduino.h>
#include <ESP8266WiFi.h>

// флаг для вывода
#define DEBUG

typedef unsigned char u8;
typedef   signed char i8;

typedef unsigned short u16;
typedef   signed short i16;

typedef unsigned int u32;
typedef   signed int i32;

// typedef unsigned long u64;
// typedef   signed long i64;

#define pin D0

const char* ssid = "VNE-N41";
const char* password = "34670000";

WiFiClient client;
const char *host = "10.21.36.131";
const u16 port = 13000;

bool connectToWifi(u64 delay_ms, u8 attempts) {
  WiFi.begin(ssid, password);
  u8 counter = 0;
  while (!WiFi.isConnected()) {
    if (counter >= attempts) {
      Serial.println("Wifi is not connected!");
      return false;
    }
    Serial.println("Wifi reconecting...");
    delay(delay_ms);
    counter++;
  }; return true;
}

bool connectToServer(u64 delay_ms, u8 attempts) {
  if (!WiFi.isConnected()) {
    Serial.println("Wifi is not connected!");
    return false;
  }
  u8 counter = 0;
  while (!client.connected()) {
    client.connect(host, port);
    if (counter >= attempts) {
      Serial.println("Server connection failed");
      return false;
    }
    Serial.println("Server reconecting...");
    delay(delay_ms);
    counter++; 
  }; return true;
}

u64 last_time_wifi = 0;
u64 last_time_server = 0;
u8 mode = 2;
void getMode() {
  // можно было бы добавить переподключение, но мне лень...
  if (!WiFi.isConnected()) {
    if (millis() - last_time_wifi > 30000) {
      connectToWifi(500, 20);
      last_time_wifi = millis();
    }
    mode = 2;
    return;
  };

  if (!client.connected()) { 
    if (millis() - last_time_server > 5000) {
      connectToServer(500, 5);
      last_time_server = millis();
    }
    mode = 2;
    return;
  };

  // отправка "волшебного" байта
  client.write(0xAF);
  client.flush();
  // чтение "волшебного" байта
  u8 byte = client.read();
  if (byte != 0xFA) return;
  // чтение заданного режима
  mode = client.read();
}

void setup() {
  Serial.begin(115200);
  pinMode(pin, OUTPUT);
  connectToWifi(500, 20);
  connectToServer(500, 5);
}

u64 last_time_mode = 0;
u64 last_time_led = 0;
bool flag = false;
void loop() {
  if (millis() - last_time_mode > 500) {
    getMode();
    last_time_mode = millis();
  }

  if (mode > 1) {
    if (millis() - last_time_led > 500) {
      flag = !flag;
      digitalWrite(pin, flag);
      last_time_led = millis();
    }
  } else {
    digitalWrite(pin, mode & 1);
  }
}
