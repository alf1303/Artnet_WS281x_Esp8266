#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <NeoPixelBus.h>
//#include "Ticker.h"
//extern Ticker wifi_ticker;
#ifdef DROP_PACKETS 
#define MIN_TIME 15 // Minimum time duration between 2 packets for allowing show packets (in milliseconds) 
#endif

#ifdef LAN_MODE
    #include "../lib/UIPEthernet/UIPEthernet.h"
    #define CS_PIN 4 //Assign GPIO4(D2) as CS pin for ENC28j60 (default was GPIO15(D8))
  #endif
//#include <Wire.h> 
//#include <LiquidCrystal_I2C.h>
  #ifdef FLASH_SELECT 
    #include "../lib/EasyButton/src/EasyButton.h"
    #include <EEPROM.h>
    #define AUTO_LED 16 // Led indicator for autoMode (16 - built-in for NodeMCU)
    EasyButton m_button(0); //FLASH button on NodeMCU
    uint8_t ledmod = 112; //indicator for on/off STATUS_LED led ()
    uint8_t ledautomod; //indicator for on/off AUTO_LED ()
    //Set AUTO mode led
    void setPin(int state) {
      digitalWrite(AUTO_LED, !state);
    }
  #endif

//Button Settings
#define MODE_PIN 5 //pin for changing mode (LOW - WIFI, HIGH - LAN) with external button
#define STATUS_WIFI 0 
#define STATUS_LAN 1
#define STATUS_LED 2 // Led indicator (2 - built-in for NodeMCU)
extern uint8_t mode; // WIFI or LAN or AUTO mode variable (0 - WIFI, 1 - LAN, 2 - AUTO)
extern uint8_t autoMode; // mode for Automatic strip control
const uint8_t autoModeCount = 6; //Number of submodes in AUTO mode (Chase, White, Red, Green, Blue, Recorded for now)

// ARTNET CODES
#define ARTNET_DATA 0x50
#define ARTNET_POLL 0x20
#define ARTNET_POLL_REPLY 0x21
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
const uint16_t PixelCount = 120; // КОЛИЧЕСТВО ПОДКЛЮЧЕННЫХ ПИКСЕЛЕЙ В ЛЕНТЕ 
const uint8_t PixelPin = 2;  // make sure to set this to the correct pin, ignored for Esp8266
const int numberOfChannels = PixelCount * 3; // Total number of channels you want to receive (1 led = 3 channels)
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
void chaserColor();
void setStaticColor(RgbColor);
void test();
void OTA_Func();
//bool ssid_selector(uint8_t uni);