#include <FastLED.h> // FastLED
#include <Wire.h>
#include <VL53L0X.h> // VL53L0X polulu
#include "ESP8266WiFi.h"
#include <ESP8266WebServer.h>
#include "html_header.h"
#include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>
#include <time.h>

#define EXT_CONFIG_SSID_PASS

#ifdef EXT_CONFIG_SSID_PASS
#include <config.h> // wifi SSID and password
#else
const char* password = "xxx";
const char* ssid     = "xxx";
#endif

// Classic web server on port 80
ESP8266WebServer server(80);

uint color_addr = 0;

struct {
  unsigned char red,green,blue;
} rgb_data;

String form;

VL53L0X sensor;
#define VL53L0X_SDA 5
#define VL53L0X_SCL 4

// distance unit: cm
#define MAX_DISTANCE 50.0
#define MIN_DISTANCE 10.0

#define LED_PIN     4 // RGB strip
#define NUM_LEDS    26
#define BRIGHTNESS  100
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
TBlendType    currentBlending;

#define UPDATES_PER_SECOND 100
#define MIN_LIT_SEC 20

float distance;
float brightness;
int mappedValue;

// time
// https://github.com/scanlime/esp8266-Arduino/blob/master/tests/Time/Time.ino

#define NORMAL_MODE 0
#define ALARM_MODE 1

int timezone = 2;
int dst = 0;
char server_response[100];
char timestr[20];
struct tm * timeinfo;

int alarm_hour = 7;
int alarm_mins = 30;
int alarm_holdoff = 60; // sec
int mode = NORMAL_MODE;

time_t holdoff_time;

time_t get_time()
{
  return  time(nullptr) - timezone * 60 * 60;
}
// Root handler: basic web page with link to settings page
void handle_root() {
  strcpy(server_response, "<html>WS2812 WiFi <a href='./out'>Control</a></html><p>");
  time_t now = get_time();
  strftime(timestr, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
  strcat(server_response,timestr);


  server.send(200, "text/html", server_response);
  delay(100);
}

// Output handler: display colorwheel and allow picking a color
void handle_outputs() {
  // Strings to strore the client output
  String RMsg;
  String GMsg;
  String BMsg;

  // Parse client output
  RMsg=server.arg("R");
  GMsg=server.arg("G");
  BMsg=server.arg("B");

  brightness= server.arg("intensity").toInt();

  // Convert to number to pass to Neopixel library
  rgb_data.red=RMsg.toInt();
  rgb_data.green=GMsg.toInt();
  rgb_data.blue=BMsg.toInt();

  // Update the form and send it back to the client
  form = "<html>";
  form = form + html_header;
  form = form + "</html>";
  // Send the form
  server.send(200, "text/html", form);

  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i].setRGB( rgb_data.red, rgb_data.green, rgb_data.blue);
  }
  FastLED.setBrightness(brightness);
  FastLED.show();

  Serial.println("RGB values updated: "+String(rgb_data.red)+","+String(rgb_data.green)+","+String(rgb_data.blue));

  // replace values in EEPROM
  EEPROM.put(color_addr,rgb_data);
  EEPROM.commit();
}

void setup() {

  // read out LED colors from the EEPROM

  EEPROM.begin(512);
  EEPROM.get(color_addr,rgb_data);

  Serial.begin(115200);
  Serial.println("RGB values loaded from EEPROM: "+String(rgb_data.red)+","+String(rgb_data.green)+","+String(rgb_data.blue));

  // connect to WiFi AP
  Serial.print("Connecting to ");
  Serial.print(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.print(WiFi.status());

  Serial.println("");

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

// get the time for a time server
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.print("\nWaiting for time");
    while (!time(nullptr)) {
      Serial.print(".");
      delay(1000);
    }
    Serial.println("");
    Serial.println("Time set:");

    time_t now = get_time();
    strftime(timestr, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
    Serial.println(timestr);

    holdoff_time = now;

  // Attach handles to web server
  // Base page
  server.on("/", handle_root);
  // Setting page
  server.on("/out", handle_outputs);
  // Start web server
  server.begin();

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );

  currentBlending = LINEARBLEND;

  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i].setRGB( rgb_data.red, rgb_data.green, rgb_data.blue);
  }

  FastLED.setBrightness(0);
  FastLED.show();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);   // turn off built-in LED

  pinMode(D0, OUTPUT);
  digitalWrite(D0, HIGH);   // turn off built-in LED

  Wire.begin(VL53L0X_SDA, VL53L0X_SCL);

  sensor.init();
  sensor.setTimeout(500);

  // Start continuous back-to-back mode (take readings as
  // fast as possible).  To use continuous timed mode
  // instead, provide a desired inter-measurement period in
  // ms (e.g. sensor.startContinuous(100)).
  sensor.startContinuous();

}

void update_light_intensity()
{
  distance =  sensor.readRangeContinuousMillimeters()/10;
  //Serial.println(distance);

  brightness= 100.0*(distance-MIN_DISTANCE)/(MAX_DISTANCE-MIN_DISTANCE);

  if ( (0 <= distance) & (distance <= MAX_DISTANCE))
  {
    FastLED.setBrightness(constrain(brightness, 0, 100));
    FastLED.show();
  }

}

void ramp_up()
{
  for (int i=0;i<=100;i++)
  {
    brightness= i;

    FastLED.setBrightness(constrain(brightness, 0, 100));
    FastLED.show();

    delay(100);
  }
}

void loop() {
  delay(50);

  time_t now = get_time();


  int hour  = (now / 3600) % 24;
  int min  = (now / 60) % 60;

  if (now > holdoff_time && hour == alarm_hour && min == alarm_mins)
  {
    mode = ALARM_MODE;
  }

  if (mode == NORMAL_MODE)
  {
    update_light_intensity();
  }
  else if (mode == ALARM_MODE)
  {
    ramp_up();
    mode = NORMAL_MODE;
    holdoff_time = now + alarm_holdoff;
  }


  // Process clients requests
  server.handleClient();

}
