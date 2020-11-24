#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <NeoPixelBus.h>
#include <LittleFS.h>
#include <Ticker.h>
#include <fxController.h>
#include "recorder.h"

#define VERSION "v_1.7.4"

#define FILE_MODES "/modes"
#define NAME_FILE "/namefile"
//#define SSID_FILE "/ssidfile"
//#define PASS_FILE "/passfile"
#define PLAYLIST_FILE "/playlist"

#define UNI 29 // change this for setting node universe and last byte of IP Address************************************
#define UNIVERSE 19 //actual universe for receiving DMX

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
extern WiFiUDP wifiUdp;

extern settings_t settings;
extern settings_t temp_set;

extern Recorder recorder;
extern uint8_t writingFlag;

extern FxController FX;
extern Ticker fxTicker;
extern Ticker fxFadeTicker;

typedef struct {
    char command;
    char option;
    uint8_t mode;
    uint8_t autoMode;
    uint8_t numEff;
    double fxSpeed;
    RgbColor color;
    uint8_t dimmer;
    IPAddress sourceIP;
    //bool save;
    uint8_t mask;
    uint8_t universe;
    uint16_t address;
    uint8_t reverse;
    uint8_t pixelCount;
    uint8_t startPixel;
    uint8_t endPixel;
    uint8_t segment;
    uint8_t playlistSize;
    bool playlistMode;
    RgbColor fxColor;
    uint8_t strobe;
    uint8_t fxSize;
    uint8_t fxParts;
    uint8_t fxFade;
    uint8_t fxParams;
    uint8_t fxSpread;
    uint8_t fxWidth;
    boolean fxReverse;
    boolean fxAttack;
    boolean fxSymm;
    boolean fxRnd;
    boolean fxRndColor;
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

extern boolean isFading;

//UDP Settings
extern uint8_t uniData[514]; 
extern uint16_t uniSize; 
//extern uint8_t net = 0; 
extern uint8_t universe; 
//uint8_t subnet = 0;
extern uint8_t hData[5];
extern uint8_t hData1[13];
extern uint8_t hData2[17];

// Neopixel settings
#define colorSaturation 200 //brightness for AUTO mode
//const uint16_t PixelCount = 120; // КОЛИЧЕСТВО ПОДКЛЮЧЕННЫХ ПИКСЕЛЕЙ В ЛЕНТЕ 
const uint8_t PixelPin = 2;  // make sure to set this to the correct pin, ignored for Esp8266
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

extern ledsettings_t *playlist; //array, containing data for settings to be played
extern ledsettings_t *playlist_temp; //temp array
extern NeoPixelAnimator animations; //cyclon effect
extern NeoPixelAnimator animations2; //fade effect
extern uint8_t playlistPeriod;
extern unsigned long playlistPeriodMs; 
extern unsigned long playlistLastTime; //for changing playlist items while playing
extern uint8_t playlist_counter; //for changing playlist items while playing

char* convertModes(int mod); //Converts digital values to String names for General mode
char* convertAutoModes(int automod); //Converts digital values to String names for Auto modes
void chaserColor(int speed);
void setStaticColor(RgbColor);
void setStaticColorDimmed(uint8_t dimmer, RgbColor col);
void setStaticColorDimmedFaded();
void test();
void test2();
void OTA_Func();
void chasePlayer(uint8_t fxNumber, uint8_t speed, uint8_t dimmer); //for playing internal effects
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
void sendWS_addressed();
void sendStartRecording();
void sendStopRecording();
void fillFixtureData();

void loadSettingsFromFs();

void saveNameToFs(bool first);

double speedToDouble(uint8_t speed);
uint8_t speedToInt(double speed);

void processFx();
void setupAnimations();
void setupAnimationsCyclon();
void moveAnim(const AnimationParam& param);
void fadeAnim(const AnimationParam& param);
void animCyclon(const AnimationParam& param);
void sinus();
void sinusRGB();

void savePlaylist();
void loadPlaylist();
void processPlaylist();
void copyPlaylistSettings(settings_t &set, ledsettings_t &plset);