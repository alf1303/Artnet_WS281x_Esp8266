#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <NeoPixelBus.h>
#include <LittleFS.h>
#include "recorder.h"
#define VERSION "v_0.6.4"

#define UNI 27 // change this for setting node universe and last byte of IP Address************************************

//#define NO_WS
//#define NO_ARTNET
#define ADV_DEBUG
#define DEBUGMODE
//#define DROP_PACKETS //In this mode packets, arrived less then MIN_TIME ms are dropped
#define NO_SIG 5000 // Maximum Time for detecting that there is no signal coming
#ifdef DROP_PACKETS 
#define MIN_TIME 15 // Minimum time duration between 2 packets for allowing show packets (in milliseconds) 
#endif

//Button Settings
#define STATUS_WIFI 0
#define STATUS_LAN 1
#define STATUS_AUTO 2
#define STATUS_FIXT 3
#define AUTO_STAT 0
#define AUTO_CHASE 1
#define AUTO_RECORDED 2
#define STATUS_LED 2 // Led indicator (2 - built-in for NodeMCU)
#define FILE_MODES "/modes"
extern WiFiUDP wifiUdp;
typedef struct {
    uint8_t mode; // WIFI or LAN or AUTO mode variable (0 - WIFI, 1 - LAN, 2 - AUTO, 3 - FIXTURE MODE)
    uint8_t autoMode; // mode for Automatic strip control
    uint8_t speed; //speed for playing effects from FS
    RgbColor readedRGB; //color for static automode
    uint8_t chaseNum; //number of internal chase
    uint8_t dimmer; //intensity
    //uint8_t recordedEffNum; //number of recorded effect
} settings_t;
extern settings_t settings;
extern settings_t temp_set;

extern Recorder recorder;
extern uint8_t writingFlag;

typedef struct {
    char command;
    char option;
    uint8_t mode;
    uint8_t autoMode;
    uint8_t numEff;
    uint8_t speed;
    RgbColor color;
    uint8_t dimmer;
    IPAddress sourceIP;
    //bool save;
    uint8_t mask;
} request_t;
extern request_t request;

typedef struct {
    uint8_t dimmer;
    uint8_t shutter;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t effect;
    uint8_t speed;
} fixture_t;
extern fixture_t fixtureData;

// ARTNET CODES
#define ARTNET_DATA 0x50
#define ARTNET_POLL 0x20
#define ARTNET_POLL_REPLY 0x21
#define ARTNET_PORT 6454
#define ARTNET_PORT_OUT 6455
#define ARTNET_PORT_OUT_REC 6456
#define ARTNET_PORT_OUT_UPD 6457
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
extern RgbColor highlite;
extern RgbColor before_highlite;
extern bool _highlite;

char* convertModes(int mod); //Converts digital values to String names for General mode
char* convertAutoModes(int automod); //Converts digital values to String names for Auto modes
void chaserColor(int speed);
void setStaticColor(RgbColor);
void setStaticColorDimmed(uint8_t dimmer, RgbColor col);
void test();
void OTA_Func();
void chasePlayer(uint8_t chaseNum, uint8_t speed, uint8_t dimmer); //for playing internal effects
void effectPlayer(); //for playing effects from FS
void initModes();
void formAnswerInfo(int port);
void processRequest();
void processGetCommand();
void processSetCommand();
void setHighliteMode();
void unsetHighliteMode();
void fillSettingsFromFs(settings_t* set);
void saveSettingsToFs();
void showStrip();
void setReset();
void setRemoteColor();
void sendWSread(uint8_t* data, uint8_t dimmer);
void sendStartRecording();
void sendStopRecording();
void fillFixtureData();