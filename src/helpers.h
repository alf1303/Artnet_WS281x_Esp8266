#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <NeoPixelBus.h>
#define VERSION "v_0.6.3"

#define UNI 33 // change this for setting node universe and last byte of IP Address************************************

#define DROP_PACKETS //In this mode packets, arrived less then MIN_TIME ms are dropped

extern WiFiUDP wifiUdp;

// ARTNET CODES
//#define ARTNET_DATA 0x50
//#define ARTNET_POLL 0x20
//#define ARTNET_POLL_REPLY 0x21
#define ARTNET_PORT 6454
#define ARTNET_HEADER 17

//UDP Settings
extern uint8_t uniData[514]; 
extern uint16_t uniSize; 
//extern uint8_t net = 0; 
extern uint8_t universe; 
//uint8_t subnet = 0;
extern uint8_t hData[ARTNET_HEADER + 1];

// Neopixel settings
#define colorSaturation 200 //brightness for AUTO mode
extern const uint16_t PixelCount = 120; // КОЛИЧЕСТВО ПОДКЛЮЧЕННЫХ ПИКСЕЛЕЙ В ЛЕНТЕ 
extern const uint8_t PixelPin = 2;  // make sure to set this to the correct pin, ignored for Esp8266
extern const int numberOfChannels = PixelCount * 3; // Total number of channels you want to receive (1 led = 3 channels)
extern NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip;
//NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart1800KbpsMethod> strip(PixelCount, PixelPin);
extern HslColor chaseColor;  // for CHASE submode of AUTO mode
extern RgbColor red;
extern RgbColor green;
extern RgbColor blue;
extern RgbColor white;
extern RgbColor black;

char* convertModes(int mod); //Converts digital values to String names for General mode
char* convertAutoModes(int automod); //Converts digital values to String names for Auto modes
void test();
void OTA_Func();
