#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <NeoPixelBus.h>

#define UNI 21 // change this for setting node universe and last byte of IP Address************************************
#define PixelCount 120  //Change for setting pixel count *****************************************
// ARTNET
#define ARTNET_PORT 6454
#define ARTNET_HEADER 17

// Neopixel settings
#define colorSaturation 200 //brightness for AUTO mode
extern uint8_t PixelPin;  // make sure to set this to the correct pin, ignored for Esp8266
extern NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip;
//NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart1800KbpsMethod> strip(PixelCount, PixelPin);
extern RgbColor red;
extern RgbColor green;
extern RgbColor blue;
extern RgbColor white;
extern RgbColor black;

void test();
void OTA_Func();
