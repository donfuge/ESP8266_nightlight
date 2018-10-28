# Gesture-controlled IoT nightlight with ESP8266WiFi


## Hardware

* NodeMcu V3 Wifi module (ESP8266)
* VL53L0X Time-of-Flight (ToF) Laser Ranging Sensor
* WS2812B LED strip


## Features

* LED color can be set on the web interface
* Brightness is controlled by holding your hand above the distance sensor. Increasing/decreasing
the distance increases/decreases the brightness. Removing your hand fixes the brightness.


## Comments

Source files are in the form of a PlatformIO project. See https://platformio.org/.


The LED color is saved to EEPROM on each color update. Therefore, changing the color on the interface in
a quickly swept fashion is not recommended. Right now the color wheel interface uses the onClick event,
and does not support swiping or mouse dragging, so this is not a problem.

Using a HC-SR04 ultrasound distance sensor module has proven to be problematic, especially in confined environments,
where multiple reflections can occur. The VL53L0X laser sensor has shown a much better performance.

## References

Color wheel, basic web server code:

https://github.com/jmbussat/WS2812_ESP8266

## Libraries used

* FastLED
* Pololulu VL53L0X
