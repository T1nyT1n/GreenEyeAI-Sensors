
## Подготовка новой ESP
(если обычный CH340 драйвер не ставится)
1. подключить ESP к устройству и установить драйвер:
    * скачать по ссылке [ссылке](https://driver.ru/_1345906378748621e92fd482e55f/Скачать-Silicon-Labs-CP210x-USB-to-UART-Bridge-Universal-Windows-Драйвер-v.11.4.0-для-Windows-бесплатно) или с гитхаба файл `CP210x_Universal_Windows_Driver.zip` и распаковать его.
    * открыть `Диспетчер устройств`, найти эту ESP и обновить драйвер выбрав папку с распакованным драйвером.

2. залить прошивку используя `ESP8266Flasher.exe` в папке `ESP8266 Firmware` и файл `Ai-Thinker_ESP8266_DOUT_8Mbit_v1.5.4.1-a_20171130.bin` *(там выбрать порт COM платы и файл)

## Настройка среды Arduino IDE
1. установить [Arduino IDE](https://github.com/arduino/arduino-ide/releases/latest)
2. установить библиотеки
    * `ESP8266 Weather Station` от ThingPulse (включая зависимости)
    * `Adafruit BMP085 Library` от Adafruit (включая зависимости)
    * `ESP8266 and ESP32 Oled Driver for SSD1306 display` от ThingPulse, Fabrice Weinberg
3. выбрать плату `NodeMCU 1.0 (ESP-12E Module)` в настройках IDE и порт на котором ESP находится.
4. вписать ссылку `http://arduino.esp8266.com/stable/package_esp8266com_index.json` в настройках IDE `Дополнительные ссылки для менеджера плат`

## Настройка среды VS code + PlatformIO
1. установить [VS code](https://code.visualstudio.com/)
2. установить расширение PlatformIO IDE
3. создать проект
4. записать в файл `plarformio.ini`:
```ini
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino
upload_port = COM3
monitor_speed = 115200
monitor_dtr = 0
monitor_rts = 0
```
5. указать в `upload_port` нужный COM порт
