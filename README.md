# Gesture-controlled IoT nightlight with ESP8266WiFi

## Features

* LED color can be set on the web interface
* Brightness is controlled by holding your hand above the distance sensor. Increasing/decreasing
the distance increases/decreases the brightness. Removing your hand fixes the brightness.

Source files are in the form of a PlatformIO project. See https://platformio.org/.

## Hardware

* NodeMcu V3 Wifi module (ESP8266)
* VL53L0X Time-of-Flight (ToF) Laser Ranging Sensor
* WS2812B LED strip

## References

Color wheel, basic web server code:

https://github.com/jmbussat/WS2812_ESP8266

## Libraries used

* FastLED
* Pololulu VL53L0X
