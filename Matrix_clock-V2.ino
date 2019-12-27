#ifdef ESP32
#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())
#define LED_ON      HIGH
#define LED_OFF     LOW
#else
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#define ESP_getChipId()   (ESP.getChipId())
#define LED_ON      LOW
#define LED_OFF     HIGH
#endif

#include <ESP_WiFiManager.h>             //https://github.com/khoih-prog/ESP_WiFiManager
#include <ESP_DoubleResetDetector.h>      //https://github.com/khoih-prog/ESP_DoubleResetDetector

#include "Adafruit_NeoPixel.h" // https://github.com/zostay/Mysook/tree/master/device/lib/Adafruit_NeoPixel_RMT
#include <RtcDS3231.h> // https://github.com/Makuna/Rtc

#define LED_PIN 13 // Led pin
#define LED_COUNT  64 // Number of leds
#define BRIGHTNESS 2 // Led brightness 0-255


WiFiUDP ntpUDP;
RtcDS3231<TwoWire> Rtc(Wire);
NTPClient timeClient(ntpUDP, "us.pool.ntp.org", 1 * 3600);
Adafruit_NeoPixel leds(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

// SSID and PW for Config Portal
String ssid = "ESP_" + String(ESP_getChipId(), HEX);
const char* password = "brmlabcz";

// SSID and PW for your Router
String Router_SSID;
String Router_Pass;

// Number of seconds after reset during which a
// subseqent reset will be considered a double reset.
#define DRD_TIMEOUT 10

// RTC Memory Address for the DoubleResetDetector to use
#define DRD_ADDRESS 0

//DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);
DoubleResetDetector* drd;

// Onboard LED I/O pin on NodeMCU board
const int PIN_LED = 2; // D4 on NodeMCU and WeMos. GPIO2/ADC12 of ESP32. Controls the onboard LED.

// Indicates whether ESP has WiFi credentials saved from previous session, or double reset detected
bool initialConfig = false;

//####MatrixClock####
uint64_t previousMillis = 0;        // will store last time LED was updated
uint16_t interval = 250;           // interval at which to blink (milliseconds)



const uint8_t  SPRINTF_BUFFER_SIZE =     32;
char          inputBuffer[SPRINTF_BUFFER_SIZE];

//const uint8_t DH[2] = {56, 48};
//const uint8_t JH[9] = {58, 50, 42, 34, 26, 18, 10, 2, 3};
//const uint8_t DM[5] = {60, 52, 44, 36, 28};
//const uint8_t JM[9] = {62, 54, 46, 38, 30, 22, 14, 6, 7};

const uint8_t DH[2] = {56, 48};
const uint8_t JH[9] = {57, 49, 41, 33, 25, 17, 9, 1, 2};
const uint8_t DM[5] = {58, 50, 42, 34, 26};
const uint8_t JM[9] = {59, 51, 43, 35, 27, 19, 11, 3, 4};
const uint8_t DS[5] = {60, 52, 44, 36, 28};
const uint8_t JS[9] = {61, 53, 45, 37, 29, 21, 13, 5, 6};


const uint32_t colors[6] = {leds.Color(255, 0, 0),leds.Color(0, 255, 255),
                            leds.Color(0, 255, 0),leds.Color(255, 0, 255),
                            leds.Color(0, 0, 255),leds.Color(255, 255, 0) 
                            };

//####MatrixClock####

void heartBeatPrint(void)
{
  static int num = 1;

  if (WiFi.status() == WL_CONNECTED)
    Serial.print("H");        // H means connected to WiFi
  else
    Serial.print("F");        // F means not connected to WiFi

  if (num == 80)
  {
    Serial.println();
    num = 1;
  }
  else if (num++ % 10 == 0)
  {
    Serial.print(" ");
  }
}


void setup()
{

  Wire.begin();
  Serial.begin(115200);
  leds.begin(); //Initialize leds object
  leds.show(); //Sets all leds to zero
  leds.setBrightness(BRIGHTNESS); //Set brightness
  // put your setup code here, to run once:
  // initialize the LED digital pin as an output.
  pinMode(PIN_LED, OUTPUT);
  Serial.println("\nStarting");

  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);

  //Local intialization. Once its business is done, there is no need to keep it around
  ESP_WiFiManager ESP_wifiManager;

  // We can't use WiFi.SSID() in ESP32 as it's only valid after connected.
  // SSID and Password stored in ESP32 wifi_ap_record_t and wifi_config_t are also cleared in reboot
  // Have to create a new function to store in EEPROM/SPIFFS for this purpose
  Router_SSID = ESP_wifiManager.WiFi_SSID();
  Router_Pass = ESP_wifiManager.WiFi_Pass();

  //Remove this line if you do not want to see WiFi password printed
  Serial.println("Stored: SSID = " + Router_SSID + ", Pass = " + Router_Pass);

  // SSID to uppercase
  ssid.toUpperCase();

  if (Router_SSID != "")
  {
    ESP_wifiManager.setConfigPortalTimeout(60); //If no access point name has been previously entered disable timeout.
    Serial.println("Got stored Credentials. Timeout 60s");
  }
  else
  {
    Serial.println("No stored Credentials. No timeout");
    initialConfig = true;
  }

  if (drd->detectDoubleReset())
  {
    Serial.println("Double Reset Detected");
    initialConfig = true;
  }

  if (initialConfig)
  {
    Serial.println("Starting configuration portal.");
    digitalWrite(PIN_LED, LED_ON); // turn the LED on by making the voltage LOW to tell us we are in configuration mode.

    //sets timeout in seconds until configuration portal gets turned off.
    //If not specified device will remain in configuration mode until
    //switched off via webserver or device is restarted.
    //ESP_wifiManager.setConfigPortalTimeout(600);

    //it starts an access point
    //and goes into a blocking loop awaiting configuration
    if (!ESP_wifiManager.startConfigPortal((const char *) ssid.c_str(), password))
      Serial.println("Not connected to WiFi but continuing anyway.");
    else
      Serial.println("WiFi connected...yeey :)");
  }

  digitalWrite(PIN_LED, LED_OFF); // Turn led off as we are not in configuration mode.

#define WIFI_CONNECT_TIMEOUT        30000L
#define WHILE_LOOP_DELAY            200L
#define WHILE_LOOP_STEPS            (WIFI_CONNECT_TIMEOUT / ( 3 * WHILE_LOOP_DELAY ))

  unsigned long startedAt = millis();

  while ( (WiFi.status() != WL_CONNECTED) && (millis() - startedAt < WIFI_CONNECT_TIMEOUT ) )
  {
    WiFi.mode(WIFI_STA);
    WiFi.persistent (true);
    // We start by connecting to a WiFi network

    Serial.print("Connecting to ");
    Serial.println(Router_SSID);

    WiFi.begin(Router_SSID.c_str(), Router_Pass.c_str());

    int i = 0;
    while ((!WiFi.status() || WiFi.status() >= WL_DISCONNECTED) && i++ < WHILE_LOOP_STEPS)
    {
      delay(WHILE_LOOP_DELAY);
    }
  }

  Serial.print("After waiting ");
  Serial.print((millis() - startedAt) / 1000);
  Serial.print(" secs more in setup(), connection result is ");

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("connected. Local IP: ");
    Serial.println(WiFi.localIP());
  }
  else
    Serial.println(ESP_wifiManager.getStatus(WiFi.status()));


  timeClient.begin();
  delay(2000);
  timeClient.update();
  Rtc.Begin();
  RTC_Update();
}


void loop()
{
  // Call the double reset detector loop method every so often,
  // so that it can recognise when the timeout expires.
  // You can also call drd.stop() when you wish to no longer
  // consider the next reset as a double reset.
  drd->loop();

  // put your main code here, to run repeatedly
  check_status();





  if (millis() - previousMillis > interval) {
    previousMillis = millis();

    if (!RTC_Valid()) {
      RTC_Update();
    }

    RtcDateTime currTime = Rtc.GetDateTime();
    printDateTime(currTime);
    Serial.println(timeClient.getFormattedTime());

    RtcDateTime dt = Rtc.GetDateTime();;
    timeToLeds(dt.Hour(), dt.Minute(), dt.Second());


  }

}
