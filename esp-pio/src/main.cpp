#include <Arduino.h>
#include <ESP8266WiFi.h>

#define START_SENSOR 0
#define SENSORS 3
#define BUTTON_PIN D0
#define CONFIG_TIMEOUT_MS 60000
#define WIFI_TIMEOUT_MS 10000
#define WIFI_RECONNECT_MS 60000
#define SERVER_TIMEOUT_MS 5000
#define SERVER_RECONNECT_MS 30000


String ssid     = "VNE-N41";
String password = "34670000";
String host     = "10.21.36.131";
u16 port        = 5000;

WiFiClient client;
WiFiServer configServer(7931);

u64 lastPressTime = 0;
bool lastButtonState = false;
bool doubleClickDetected = false;

const String my_ssid = "ESP_CONIG";
const String my_password = "34670000";
const u8 config_magic[4] = {0xDE, 0xAD, 0xBE, 0xEF};

struct Sensor {
	float value;
	u64 last_time;
	u64 delay;
	float base_min; float base_max;
	float shift_min; float shift_max;
};

Sensor s[] = {
	{ 0.0, 0, 5000, -1000.0, 1000.0,  -10.0,  10.0 }, // pH sensor
	{ 0.0, 0, 1000,     0.0, 1024.0, -200.0, 200.0 }, // light sensor
	{ 0.0, 0, 1000,   -30.0,   50.0,   -1.0,   1.0 }, // T sensor
};


float getRandomPrecent() {
	return rand() / (float) RAND_MAX;
}


float getRandom(float min, float max) {
	return min + (max - min) * getRandomPrecent();
}


u64 lastWifiConnectAttempt = 0;
void connectToWifi() {
	if (millis() - lastWifiConnectAttempt < (
		WIFI_TIMEOUT_MS + WIFI_RECONNECT_MS
	) && lastWifiConnectAttempt != 0) return;
	lastWifiConnectAttempt = millis();
	Serial.print("Подключение к Wi-Fi: ");
	WiFi.begin(ssid, password);
	while (!WiFi.isConnected()) {
		auto status = WiFi.status();
		if (status == WL_NO_SSID_AVAIL) {
			Serial.println(" Ошибка\nДанный SSID не доступен\n");
			return;
		}
		if (status == WL_WRONG_PASSWORD) {
			Serial.println(" Ошибка\nПароль не верный\n");
			return;
		}
		if (millis() - lastWifiConnectAttempt > WIFI_TIMEOUT_MS) {
			Serial.println(" Таймаут\n");
			return;
		}
		Serial.print(".");
		delay(500);
	}
	Serial.println(" Готово\n");
}

u64 lastServerConnectAttempt = 0;
void connectToServer() {
	if (millis() - lastServerConnectAttempt < (
		SERVER_TIMEOUT_MS + SERVER_RECONNECT_MS
	) && lastServerConnectAttempt != 0) return;
	lastServerConnectAttempt = millis();
	Serial.print("Подключение к серверу: ");
	while (!client.connected()) {
		if (!WiFi.isConnected()) {
			Serial.println(" Ошибка\nWiFi не подключен\n");
			return;
		}
		if (millis() - lastServerConnectAttempt > SERVER_TIMEOUT_MS) {
			Serial.println(" Таймаут\n");
			return;
		}
		client.connect(host, port);
		Serial.print(".");
		delay(500);
	}
	Serial.println(" Готово\n");
}

void sendSensorData(int sensor_id, float data) {
	String body = "{\"sensor_id\":\"" + String(sensor_id + START_SENSOR) +
								"\",\"data\":" + String(data) + "}";
	int contentLength = body.length();
	String req = "POST /data HTTP/1.1\r\n";
	req += "Host: " + host + "\r\n";
	req += "Content-Type: application/json\r\n";
	req += "Content-Length: " + String(contentLength) + "\r\n";
	req += "Connection: keep-alive\r\n";
	req += "\r\n";
	req += body;

	while (!client.connected())
		client.connect(host, port);
	client.print(req);
	// while (client.available()) client.read();
	client.stop();

	Serial.print("Отправлен ");
	Serial.print(sensor_id);
	Serial.print(" со значением: ");
	Serial.println(data);
}


void enterConfigMode() {
	Serial.println("\n=== РЕЖИМ КОНФИГУРАЦИИ (1 минута) ===");
	Serial.print("Точка доступа: ");
	Serial.print(my_ssid);
	Serial.print(" / ");
	Serial.println(my_password);
	Serial.println("Подключитесь и отправьте конфигурационный пакет на порт " + String(7931));

	configServer.begin();
	u32 startTime = millis();
	bool configured = false;

	while (millis() - startTime < CONFIG_TIMEOUT_MS && !configured) {
		WiFiClient configClient = configServer.accept();
		if (configClient) {
			configClient.setTimeout(2000);

			u8 magic[4];
			if (configClient.readBytes(magic, 4) != 4) {
				configClient.stop(); continue; }
			if (memcmp(magic, config_magic, 4) != 0) {
				configClient.stop(); continue; }

			u8 ssid_len;
			if (configClient.readBytes(&ssid_len, 1) != 1) {
				configClient.stop(); continue; }
			if (ssid_len > 32) {
				configClient.stop(); continue; }
				
			u8 ssid_buf[ssid_len+1];
			if (configClient.readBytes(ssid_buf, ssid_len) != ssid_len) {
				configClient.stop(); continue; }
			ssid_buf[ssid_len] = 0;

			u8 pass_len;
			if (configClient.readBytes(&pass_len, 1) != 1) {
				configClient.stop(); continue; }
			if (pass_len > 64) {
				configClient.stop(); continue; }

			u8 pass_buf[pass_len+1];
			if (configClient.readBytes(pass_buf, pass_len) != pass_len) {
				configClient.stop(); continue; }
			pass_buf[pass_len] = 0;

			u8 host_len;
			if (configClient.readBytes(&host_len, 1) != 1) {
				configClient.stop(); continue; }
			if (host_len > 64) {
				configClient.stop(); continue; }

			u8 host_buf[host_len+1];
			if (configClient.readBytes(host_buf, host_len) != host_len) {
				configClient.stop(); continue; }
			host_buf[host_len] = 0;

			u8 port_bytes[2];
			if (configClient.readBytes(port_bytes, 2) != 2) {
				configClient.stop(); continue; }

			ssid = (char*)ssid_buf;
			password = (char*)pass_buf;
			host = (char*)host_buf;
			port = (port_bytes[0] << 8) | port_bytes[1];

			Serial.println("Новые параметры получены:");
			Serial.println("SSID: " + ssid);
			Serial.println("PASSWORD: " + password);
			Serial.println("HOST: " + host);
			Serial.println("PORT: " + String(port));

			configClient.stop();
			configured = true;
		}
		delay(10);
	}
	configServer.stop();
	if (!configured) {
		Serial.println("Таймаут конфигурации, продолжаем работу со старыми параметрами");
	} else {
		Serial.println("Переподключение к новой сети...");
		WiFi.disconnect();
		delay(1000);
		connectToWifi();
		if (WiFi.isConnected()) {
			connectToServer();
		}
	}
	Serial.println("=== ВЫХОД ИЗ РЕЖИМА КОНФИГУРАЦИИ ===\n");
}

void checkDoubleClick() {
	bool currentButtonState = digitalRead(BUTTON_PIN);
	if (currentButtonState && !lastButtonState) {
		auto x = millis() - lastPressTime;
		if (x > 100 && x < 500) {
			doubleClickDetected = true;
			lastPressTime = 0;
		} else {
			lastPressTime = millis();
		}
	}
	lastButtonState = currentButtonState;
}

void setup() {
	srand(micros());
	Serial.begin(115200);
	Serial.println();
	pinMode(D0, INPUT);
	WiFi.mode(WIFI_AP_STA);
	WiFi.softAP(my_ssid, my_password);
	Serial.print("IP точки доступа: ");
	Serial.println(WiFi.softAPIP());
	for (u32 i = 0; i < SENSORS; i++) {
		s[i].value = getRandom(s[i].base_min, s[i].base_max);
	}
}

void loop() {
	checkDoubleClick();
	if (doubleClickDetected) {
		doubleClickDetected = false;
		enterConfigMode();
	}

	if (!WiFi.isConnected()) {
		connectToWifi(); return; }
	// if (!client.connected()) {
	// 	connectToServer(); return; }

	for (u32 i = 0; i < SENSORS; i++) {
		if (millis() - s[i].last_time < s[i].delay) continue;
		s[i].value += getRandom(s[i].shift_min, s[i].shift_max);
		sendSensorData(i, s[i].value);
		s[i].last_time = millis();
	}
}