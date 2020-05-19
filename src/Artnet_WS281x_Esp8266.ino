/*  platformio run -t upload --upload-port 2.0.0.51

--- Receiving ArtNet over WiFi using Esp8266
--- Receiving ArtNet over Ethernet using Enc28j60, connected to Esp8266
--- Output Artnet data to ws2812/13 using NeoPixelBus library
--- Catch one universe
--- uses GPIO3 (RX on Esp8266) GPIO2 for other boards
*/
//
#define FLASH_SELECT
//#define EXTERNAL_SELECT
#define ADV_DEBUG
#define DEBUGMODE
#define DROP_PACKETS //In this mode packets, arrived less then MIN_TIME ms are dropped
#define LAN_MODE //Comment if using only in WiFi mode (EXPERIMENTAL)
#define NO_SIG 5000 // Maximum Time for detecting that there is no signal coming
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <NeoPixelBus.h>
  #ifdef LAN_MODE
    #include "../lib/UIPEthernet/UIPEthernet.h"
    #define CS_PIN 4 //Assign GPIO4(D2) as CS pin for ENC28j60 (default was GPIO15(D8))
  #endif
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
uint8_t mode; // WIFI or LAN or AUTO mode variable (0 - WIFI, 1 - LAN, 2 - AUTO)
uint8_t autoMode; // mode for Automatic strip control
const uint8_t autoModeCount = 6; //Number of submodes in AUTO mode (Chase, White, Red, Green, Blue, Recorded for now)

//Ethernet Settings
#define UNI 70 //************************************
const byte mac[] = { 0x44, 0xB3, 0x3D, 0xFF, 0xAE, 0x70}; // Last byte same as ip **************************

//Wifi Settings
const uint8_t startUniverse = UNI; //****************************
IPAddress ip(2, 0, 0, UNI); //IP ADDRESS NODEMCU ****************
IPAddress gateway(2, 0, 0, 101); //IP ADDRESS РОУТЕРА 
IPAddress subnet_ip(255, 255, 255, 0); //SUBNET_IP
const char* ssid = "udp"; //SSID 
const char* password = "esp18650"; //PASSW 
//const char* ssid = "ANetEsp"; //SSID 
//const char* password = "ktulhu_1234"; //PASSW 

//UDP Settings
WiFiUDP wifiUdp;
  #ifdef LAN_MODE
    EthernetUDP ethernetUdp;
  #endif
uint8_t uniData[514]; uint16_t uniSize; uint8_t net = 0; uint8_t universe; uint8_t subnet = 0;
uint8_t hData[ARTNET_HEADER + 1];

// Neopixel settings
const uint16_t PixelCount = 120; // КОЛИЧЕСТВО ПОДКЛЮЧЕННЫХ ПИКСЕЛЕЙ В ЛЕНТЕ 
const uint8_t PixelPin = 2;  // make sure to set this to the correct pin, ignored for Esp8266
const int numberOfChannels = PixelCount * 3; // Total number of channels you want to receive (1 led = 3 channels)
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);

float chaseHue = 0.0f; // for CHASE submode of AUTO mode
HslColor chaseColor;  // for CHASE submode of AUTO mode

//LCD Settings
//LiquidCrystal_I2C lcd(0x27, 16, 2);

//Sets status led to show current mode (OFF - WIFI, ON - LAN) !val becouse ESP sets incorrect (inverted)
void setStatusLed(int val) { 
  digitalWrite(STATUS_LED, !val); 
#ifdef ADV_DEBUG
  printf("**** set STATUS_LED: %d\n", val);
#endif
  }

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
    if (autoMode == 5) readDataPacketFromFS();
  #else //EXTERNAL button select
      if (digitalRead(MODE_PIN) == 0) 
        {mode = STATUS_WIFI;}
          else 
            {mode = STATUS_LAN;}
  #endif
#ifdef ADV_DEBUG
  printf("**** readedMODE: %s, readedAUTOMODE: %s\n", convertModes(readMode), convertAutoModes(readAutoMode));
  printf("**** MODE: %s, AUTOMODE: %s\n", convertModes(mode), convertAutoModes(autoMode));
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
          #ifdef ADV_DEBUG
            printf("**** nextMODE: %s\n", convertModes(ledmod));
          #endif
      }
      else {
          #ifdef ADV_DEBUG
            printf("xxxx error_FS nextMODE: %s\n", convertModes(ledmod));
          #endif
      }
    }
    else {
      autoMode++;
      if (autoMode > 5) autoMode = 0;
      EEPROM.write(1, autoMode);
      if(EEPROM.commit()) {
        #ifdef ADV_DEBUG
            printf("**** nextAUTOMODE: %s\n", convertAutoModes(autoMode));
        #endif
      }
      else {
        #ifdef ADV_DEBUG
            printf("xxxx error_FS nextAUTOMODE: %s\n", convertAutoModes(autoMode));
        #endif
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
          #ifdef ADV_DEBUG
            printf("**** _press_ nextMode: %s\n", convertModes(temp));
          #endif
        } 
        else {
          #ifdef ADV_DEBUG
            printf("xxxx _press_ error_FS nextMode: %s\n", convertModes(temp));
          #endif
        }
    }
#endif

void setup() {
  Serial.begin(115200);
  delay(10);
    #ifdef LAN_MODE
      UIPEthernet.init(CS_PIN); // Configures ESP8266 to use custom userdefined CS pin
    #endif
  #ifdef FLASH_SELECT
    EEPROM.begin(530);
    m_button.begin();
    m_button.onPressed(onPressed);
    m_button.onPressedFor(3000, onPressedForDuration3s);
    //Wire.begin(D2, D3); //D2 is using for CS pin Enc28j60
    //lcd.begin();
    //lcd.backlight();
    pinMode(AUTO_LED, OUTPUT);
  #endif
  pinMode(STATUS_LED, OUTPUT);
  pinMode(MODE_PIN, INPUT);
  checkStatus(); //Set mode by changing value of variable <mode>
    #ifdef LAN_MODE
      if (mode == STATUS_LAN) 
        {ConnectEthernet();}
          else 
            {ConnectWifi();}
    #else
      ConnectWifi();
    #endif
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
boolean ConnectWifi(void) {
  boolean state = true;
  int i = 0;
  WiFi.config(ip, gateway, subnet_ip);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.begin(ssid, password);
  Serial.printf("Connecting to WiFi. SSID: %s\n", ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
   if (i > 10){              // Wait 8s for connecting to WiFI
    state = false;
      break;
    }
    i++;
  }
  if (state) {
    int res = wifiUdp.begin(ARTNET_PORT);
    #ifdef ADV_DEBUG
        if (res == 1) printf("**** Opened UDP socket (WIFI) on port :%d\n", ARTNET_PORT);
        if (res == 0) printf("xxxx error opening UDP(WIFI)(no available sockets)\n");
    #endif
  } // Open ArtNet port for WIFI
  #ifdef ADV_DEBUG
    if(WiFi.status() == 3) { 
      Serial.print("**** Connected. IP: ");
      Serial.print(WiFi.localIP());
      Serial.println();
      }
      else  {
        Serial.print("xxxx error connection to WIFI: ");
        Serial.print(WiFi.status());
        Serial.println();
      }
  #endif
  return state;
}

#ifdef LAN_MODE
//connect Ethernet
void ConnectEthernet() {
  #ifdef ADV_DEBUG
    printf("Connecting to LAN\n");
  #endif
  Ethernet.begin(mac,ip, dns, gateway, subnet_ip);
  int res = ethernetUdp.begin(ARTNET_PORT); // Open ArtNet port LAN) 
      #ifdef ADV_DEBUG
        if (res == 1) printf("**** Opened UDP socket (LAN) on port :%d\n", ARTNET_PORT);
        if (res == 0) printf("xxxx error opening UDP (LAN)(no available sockets)\n");
      #endif
}
#endif

//#ifdef DROP_PACKETS
//Return duration between current time and time in variable "newTime"
int getTimeDuration() { 
 long old = newTime;
        newTime = millis();
        int res = newTime - old;
        return res;
}
//#endif

//Reading WiFi UDP Data (IRAM_ATTR) (ICACHE_FLASH_ATTR)
void IRAM_ATTR readWiFiUDP() {
    if (wifiUdp.parsePacket() && wifiUdp.destinationIP() == ip) {
      noSignalTime = millis(); //this will be compared with current time in processData function
      blackoutSetted = false; // allow blackout when no signal for a some time
        wifiUdp.read(hData, 18);
     if ( hData[0] == 'A' && hData[4] == 'N' && startUniverse == hData[14]) {
         uniSize = (hData[16] << 8) + (hData[17]);
         wifiUdp.read(uniData, uniSize);
         universe = hData[14];
         int dur = getTimeDuration();

#ifdef DEBUGMODE
         if(sizeof(uniData) == 514) { //*******************************************
           if(uniData[509] == 255) {
             mycounter = 0;
           }
         }

        printf("%d  %d ms_wifi", mycounter, dur);//************************************
        //printf(" ** p_len: %d ** 358: %d\n", uniSize, uniData[357]);
        mycounter++;//***************************************************************
#endif
         #ifdef DROP_PACKETS
         if (dur > MIN_TIME) sendWS();
          #else 
          sendWS();
         #endif

        //Recording to FS
        if (uniData[511] == 201) recordPacketsCounter++;
          else recordPacketsCounter = 0;
        if (recordPacketsCounter > 25) {
          writeDataPacketToFS();
          setRecordedMode();
          recordPacketsCounter = 0;
        }

        //Receiving packet from remote for recording
        if (uniData[510] == 175) {
          noSignalTime = 0; // awoid blackout
          blackoutSetted = true; //avoid blackout
          autoMode = 5; //switch to RECORDED mode for showing received packet
        }
        }    
    }
}

#ifdef LAN_MODE
//Reading Ethernet UDP Data (IRAM_ATTR) (ICACHE_FLASH_ATTR)
void IRAM_ATTR readEthernetUDP() {
    if (ethernetUdp.parsePacket() /*&& ethernetUdp.destinationIP() == ip*/) {
      noSignalTime = millis();
        ethernetUdp.read(hData, 18);
     if ( hData[0] == 'A' && hData[4] == 'N' && startUniverse == hData[14]) {
         uniSize = (hData[16] << 8) + (hData[17]);
         ethernetUdp.read(uniData, uniSize);
         universe = hData[14];
         int dur = getTimeDuration();

#ifdef DEBUGMODE
        if(sizeof(uniData) == 514) { //*******************************************
           if(uniData[509] == 255) {
             mycounter = 0;
           }
         }
         Serial.printf("%d  %d ms_lan\n", mycounter, dur);//***********************************
         mycounter++;//********************************************************************
#endif
          #ifdef DROP_PACKETS
         if (dur > MIN_TIME) sendWS();
          #else 
          sendWS();
         #endif
        }    
    }
}
#endif

//Choosing which readUPD function to use
void processData() {
  switch (mode) {
    case 0:
      readWiFiUDP();
      if (noSignalTime == 0) noSignalTime = millis();
      if (((millis() - noSignalTime) > NO_SIG) && !blackoutSetted) {
        setStaticColor(black);
        blackoutSetted = true;
      }
      break;
    case 1:
      #ifdef LAN_MODE
        readEthernetUDP();
      #endif
      if (noSignalTime == 0) noSignalTime = millis();
      if ((millis() - noSignalTime) > NO_SIG) setStaticColor(black);
      break;
    case 2:
      autoModeFunc();
      readWiFiUDP();
      break;
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
        case 5:
          sendWS();
          break;
        }
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

void readDataPacketFromFS() {
  for(int i = 0; i < 511; i++) {
    uniData[i] = EEPROM.read(i+11);
  }
}

void writeDataPacketToFS() {
  for(int i = 0; i < 511; i++) {
    EEPROM.write(i+11, uniData[i]);
  }
  EEPROM.commit();
}

void setRecordedMode() {
  EEPROM.write(0, 2);
  EEPROM.write(1, 5);
  if (EEPROM.commit()) setPin(1);
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
