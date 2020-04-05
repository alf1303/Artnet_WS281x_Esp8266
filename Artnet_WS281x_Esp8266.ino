/*
--- Receiving ArtNet over WiFi using Esp8266
--- Receiving ArtNet over Ethernet using Enc28j60, connected to Esp8266
--- Output Artnet data to ws2812/13 using NeoPixelBus library
--- Catch one universe
--- uses GPIO3 (RX on Esp8266) GPIO2 for other boards
*/

#define FLASH_SELECT
//#define EXTERNAL_SELECT

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <NeoPixelBus.h>
#include <UIPEthernet.h>
  #ifdef FLASH_SELECT 
    #include <EasyButton.h>
    #include <EEPROM.h>
    EasyButton m_button(0);
    uint8_t ledmod = 112;
  #endif

//Button Settings
#define MODE_PIN 5 //pin for changing mode (LOW - WIFI, HIGH - LAN) with external button
#define STATUS_WIFI 0 
#define STATUS_LAN 1
#define STATUS_LED 2 // Led indicator (2 - built-in for NodeMCU)
uint8_t mode; // WIFI or LAN mode variable (0 - WIFI, 1 - LAN)


// ARTNET CODES
#define ARTNET_DATA 0x50
#define ARTNET_POLL 0x20
#define ARTNET_POLL_REPLY 0x21
#define ARTNET_PORT 6454
#define ARTNET_HEADER 17

//Ethernet Settings
const byte mac[] = { 0x44, 0xB3, 0x3D, 0xFF, 0xAE, 0x55 }; // Last same as ip **************************

//Wifi Settings
const uint8_t startUniverse = 55; //****************************
IPAddress ip(2, 0, 0, 55); //IP ADDRESS NODEMCU ****************
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

//Sets status led to show current mode (OFF - WIFI, ON - LAN) !val becouse ESP sets incorrect (inverted)
void setStatusLed(int val) { digitalWrite(STATUS_LED, !val); }

void checkStatus(){ //Reads the MODE pin and sets mode variable according
  #ifdef FLASH_SELECT //FLASH SELECT
    if (EEPROM.read(0) == 1) 
      {mode = STATUS_LAN;}
        else  // EXTERNAL button select
          {mode = STATUS_WIFI;}
  #else
      if (digitalRead(MODE_PIN) == 0) 
        {mode = STATUS_WIFI;}
          else 
            {mode = STATUS_LAN;}
  #endif
  }

#ifdef FLASH_SELECT
  void onPressed() {
    if (ledmod == 112)
      ledmod = !mode;
    else ledmod = !ledmod;
      EEPROM.write(0, ledmod);
    if(EEPROM.commit()) setStatusLed(ledmod);
  }
#endif

void setup() {
  //Serial.begin(115200);
  //delay(10);
  #ifdef FLASH_SELECT
    EEPROM.begin(10);
    m_button.begin();
    m_button.onPressed(onPressed);
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
    readUDP();
}

// connect to wifi
boolean ConnectWifi(void)
{
  WiFi.config(ip, gateway, subnet_ip);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
   // Serial.print(".");
  }
  wifiUdp.begin(ARTNET_PORT); // Open ArtNet port for WIFI
}

//connect Ethernet
void ConnectEthernet() {
  Ethernet.begin(mac,ip, dns, gateway, subnet_ip);
  ethernetUdp.begin(ARTNET_PORT); // Open ArtNet port LAN) 
  }

//Reading WiFi UDP Data (IRAM_ATTR) (ICACHE_FLASH_ATTR)
void IRAM_ATTR readWiFiUDP() {
    if (wifiUdp.parsePacket() && wifiUdp.destinationIP() == ip) {
        wifiUdp.read(hData, 18);
     if ( hData[0] == 'A' && hData[4] == 'N' && startUniverse == hData[14]) {
         uniSize = (hData[16] << 8) + (hData[17]);
         wifiUdp.read(uniData, uniSize);
         universe = hData[14];
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
         sendWS();
        }    
    }
}

//Choosing which readUPD function to use
void readUDP() {
  if (mode == STATUS_LAN) 
  {
    readEthernetUDP();
    }
  else {
    readWiFiUDP();
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
