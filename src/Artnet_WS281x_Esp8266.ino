/*  platformio run -t upload --upload-port 2.0.0.51

--- Receiving ArtNet over WiFi using Esp8266
--- Receiving ArtNet over Ethernet using Enc28j60, connected to Esp8266
--- Output Artnet data to ws2812/13 using NeoPixelBus library
--- Catch one universe
--- uses GPIO3 (RX on Esp8266) GPIO2 for other boards
tyrtyr
*/

#define FLASH_SELECT
//#define EXTERNAL_SELECT
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <NeoPixelBus.h>
#include "../lib/UIPEthernet/UIPEthernet.h"
//#include <Wire.h> 
//#include <LiquidCrystal_I2C.h>
#include "helpers.h"
  #ifdef FLASH_SELECT 
    #include "../lib/EasyButton/src/EasyButton.h"
    #include <EEPROM.h>
    #define AUTO_LED 16 // Led indicator for autoMode (16 - built-in for NodeMCU)
    EasyButton m_button(0); //FLASH button on NodeMCU
    uint8_t ledmod = 112; //indicator for on/off STATUS_LED led ()
    uint8_t ledautomod; //indicator for on/off AUTO_LED ()
    void setPin(int state) {
      digitalWrite(AUTO_LED, !state);
    }
  #endif

//Button Settings
#define MODE_PIN 5 //pin for changing mode (LOW - WIFI, HIGH - LAN) with external button
#define STATUS_WIFI 0 
#define STATUS_LAN 1
#define STATUS_LED 2 // Led indicator (2 - built-in for NodeMCU)
uint8_t mode; // WIFI or LAN mode variable (0 - WIFI, 1 - LAN)
uint8_t autoMode; // mode for Automatic strip control
const uint8_t autoModeCount = 5; //Number of submodes in AUTO mode (Chase, White, Red, Green, Blue for now)
                                  //Change if add more submodes

//Ethernet Settings
#define IND 55 //************************************
const byte mac[] = { 0x44, 0xB3, 0x3D, 0xFF, 0xAE, 0x55}; // Last byte same as ip **************************

//Wifi Settings
const uint8_t startUniverse = IND; //****************************
IPAddress ip(2, 0, 0, IND); //IP ADDRESS NODEMCU ****************
IPAddress gateway(2, 0, 0, 101); //IP ADDRESS РОУТЕРА 
IPAddress subnet_ip(255, 255, 255, 0); //SUBNET_IP
const char* ssid = "ANetEsp"; //SSID 
const char* password = "ktulhu_1234"; //PASSW 

//UDP Settings
WiFiUDP wifiUdp;
EthernetUDP ethernetUdp;
uint8_t uniData[514]; uint16_t uniSize; uint8_t net = 0; uint8_t universe; uint8_t subnet = 0;
uint8_t hData[ARTNET_HEADER + 1];

// Neopixel settings
const uint16_t PixelCount = 120; // КОЛИЧЕСТВО ПОДКЛЮЧЕННЫХ ПИКСЕЛЕЙ В ЛЕНТЕ 
const uint8_t PixelPin = 2;  // make sure to set this to the correct pin, ignored for Esp8266
const int numberOfChannels = PixelCount * 3; // Total number of channels you want to receive (1 led = 3 channels)
NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);

float chaseHue = 0.0f; // for CHASE submode of AUTO mode
HslColor chaseColor;  // for CHASE submode of AUTO mode

//LCD Settings
//LiquidCrystal_I2C lcd(0x27, 16, 2);

//Sets status led to show current mode (OFF - WIFI, ON - LAN) !val becouse ESP sets incorrect (inverted)
void setStatusLed(int val) { digitalWrite(STATUS_LED, !val); }

void checkStatus(){ //Gets value and sets mode variable according to it
  #ifdef FLASH_SELECT //FLASH SELECT
    uint8_t readMode = EEPROM.read(0);
    uint8_t readAutoMode = EEPROM.read(1);
    if ((readMode < 0) || (readMode > 2)) // if not correct values, set default - WIFI
      mode = STATUS_WIFI;
    else  { mode = readMode;}  //Set readed value
      if (mode == 2) {setPin(1); ledautomod = 1;} // On AUTO mode led and set it indicator
      else {setPin(0); ledautomod = 0;} // Off AUTO mode led and set it indicator
    if (readAutoMode >= autoModeCount)  autoMode = 0; // Set default if readed incorrect
    else autoMode = readAutoMode; 
  #else //EXTERNAL button select
      if (digitalRead(MODE_PIN) == 0) 
        {mode = STATUS_WIFI;}
          else 
            {mode = STATUS_LAN;}
  #endif
  }

#ifdef FLASH_SELECT
  void onPressed() {
    if (mode != 2) {
      if (ledmod == 112)
        ledmod = !mode;
      else ledmod = !ledmod;
        EEPROM.write(0, ledmod);
      if(EEPROM.commit()) {
        setStatusLed(ledmod);
      }
    }
    else {
      autoMode++;
      if (autoMode > 4) autoMode = 0;
      EEPROM.write(1, autoMode);
      if(EEPROM.commit()) {
      }
    }
  }

  void onPressedForDuration3s() {
        uint8_t temp;
        ledautomod = !ledautomod;
        if (ledautomod) { temp = 2; }
        else { temp = 0; }
        EEPROM.write(0, temp);
        if(EEPROM.commit()) {
          if (temp == 2) setPin(1);
          if (temp == 0) setPin(0);
        } 
    }
#endif

void setup() {
  Serial.begin(115200);
  delay(10);
  
  #ifdef FLASH_SELECT
    EEPROM.begin(10);
    m_button.begin();
    m_button.onPressed(onPressed);
    m_button.onPressedFor(3000, onPressedForDuration3s);
    //Wire.begin(D2, D3);
    //lcd.begin();
    //lcd.backlight();
    pinMode(AUTO_LED, OUTPUT);
  #endif
  pinMode(STATUS_LED, OUTPUT);
  pinMode(MODE_PIN, INPUT);
  checkStatus(); //Set mode by changing value of variable <mode>
  if (mode == STATUS_WIFI) 
    {ConnectWifi();}
      else 
        {ConnectEthernet();}
  setStatusLed(mode);
  strip.Begin();
  OTA_Func();
}

void loop() { 
  #ifdef FLASH_SELECT
    m_button.read();
  #endif
  ArduinoOTA.handle();
    processData();
}

// connect to wifi
boolean ConnectWifi(void)
{
  boolean state = true;
  int i = 0;
  WiFi.config(ip, gateway, subnet_ip);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
   // Serial.print(".");
   if (i > 16){              // Wait 8s for connecting to WiFI
    state = false;
      break;
    }
    i++;
  }
  if (state) wifiUdp.begin(ARTNET_PORT); // Open ArtNet port for WIFI
  return state;
}

//connect Ethernet
void ConnectEthernet() {
  Ethernet.begin(mac,ip, dns, gateway, subnet_ip);
  ethernetUdp.begin(ARTNET_PORT); // Open ArtNet port LAN) 
  }

long neww = 0;
int testTime() {
 long old = neww;
        neww = millis();
        int res = neww - old;
        return res;
        }

//Reading WiFi UDP Data (IRAM_ATTR) (ICACHE_FLASH_ATTR)
void IRAM_ATTR readWiFiUDP() {
    if (wifiUdp.parsePacket() && wifiUdp.destinationIP() == ip) {
        wifiUdp.read(hData, 18);
     if ( hData[0] == 'A' && hData[4] == 'N' && startUniverse == hData[14]) {
         uniSize = (hData[16] << 8) + (hData[17]);
         wifiUdp.read(uniData, uniSize);
         universe = hData[14];
         Serial.print(testTime());
         Serial.print("  ");
         sendWS();
        }    
    }
}

//Reading Ethernet UDP Data (IRAM_ATTR) (ICACHE_FLASH_ATTR)
void IRAM_ATTR readEthernetUDP() {
    if (ethernetUdp.parsePacket() /*&& ethernetUdp.destinationIP() == ip*/) {
        ethernetUdp.read(hData, 18);
     if ( hData[0] == 'A' && hData[4] == 'N' && startUniverse == hData[14]) {
         uniSize = (hData[16] << 8) + (hData[17]);
         ethernetUdp.read(uniData, uniSize);
         universe = hData[14];
         Serial.print(testTime());
         Serial.print("  ");
         sendWS();
        }    
    }
}

void autoModeFunc() {
  if (autoMode == 0) {
    chaserColor();
    }
    else {
      switch (autoMode) {
        case 1:
          setStaticColor(white);
          break;
        case 2:
          setStaticColor(red);
          break;
         case 3:
          setStaticColor(green);
          break;
         case 4:
          setStaticColor(blue);
          break;
        }
    }
  }

//Choosing which readUPD function to use
void processData() {
  switch (mode) {
    case 0:
      readWiFiUDP();
      break;
    case 1:
      readEthernetUDP();
      break;
    case 2:
      autoModeFunc();
      break;
    }
  }

void sendWS() {
    for (int i = 0; i < PixelCount; i++)
    {
        RgbColor color(uniData[i * 3], uniData[i * 3 + 1], uniData[i * 3 + 2]);
        strip.SetPixelColor(i, color);
    } 
    strip.Show(); 
}

//OTA - Flashing over Air
void OTA_Func() {
    ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  }

    void chaserColor() {
        chaseColor = HslColor (chaseHue, 1.0f, 0.4f);
        for (int i = 0; i < PixelCount; i++) strip.SetPixelColor(i, chaseColor);
        strip.Show();
        chaseHue = chaseHue + 0.005f;
        if (chaseHue >= 1.0) chaseHue = 0;
        delay(100);
    }

  void setStaticColor(RgbColor color) {
    for (int i = 0; i < PixelCount; i++) {
      strip.SetPixelColor(i, color);
      strip.Show();
    }
  }
