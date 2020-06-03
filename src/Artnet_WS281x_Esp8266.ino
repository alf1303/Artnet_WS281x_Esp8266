/*  platformio run -t upload --upload-port 2.0.0.51
//beacon interval an DTIM

--- Receiving ArtNet over WiFi using Esp8266
--- Receiving ArtNet over Ethernet using Enc28j60, connected to Esp8266
--- Output Artnet data to ws2812/13 using NeoPixelBus library
--- Catch one universe
--- uses GPIO3 (RX on Esp8266) GPIO2 for other boards

//WRiting to ws - ~418mcrsec
*/
//#define NO_WS
//#define NO_ARTNET
#define ADV_DEBUG
#define DEBUGMODE
#define DROP_PACKETS //In this mode packets, arrived less then MIN_TIME ms are dropped
#define NO_SIG 5000 // Maximum Time for detecting that there is no signal coming
#include "helpers.h"

//Ethernet Settings
//const char* ssid; //SSID 
const char* ssid1 = (char*)"udp";
//Ticker wifi_ticker;

long newTime = 0; // holds time for calculating time interval between packets (for DROP_PACLETS mode)
long noSignalTime = 0; // holds time for calculating cctime interval after last arrived packet (for NOSIGNAL blackout mode)
bool blackoutSetted = false; // used for avoiding blackout if no signal, when in RECORDED mode
int recordPacketsCounter = 0; // counting packets for allow saving received packet into FS (for avoiding signal noise issues)
int mycounter = -1; //counter of packets, need only for debugging and testing for printing in readWiFIUdp and readEthernetUdp methods

//Wifi Settings
const uint8_t startUniverse = UNI; //****************************
IPAddress ip(2, 0, 0, UNI); //IP ADDRESS NODEMCU ****************
IPAddress gateway(2, 0, 0, 101); //IP ADDRESS РОУТЕРА 
IPAddress subnet_ip(255, 255, 255, 0); //SUBNET_IP
const char* password = "esp18650"; //PASSW 

void setStatusLed(int val) { 
  digitalWrite(STATUS_LED, !val); 
#ifdef ADV_DEBUG
  printf("**** set STATUS_LED: %d\n", val);
#endif
  }

void setup() {
  Serial.begin(115200);
  delay(10);
  LittleFS.begin();
  strip.Begin();
  test();
  initModes();
  ConnectWifi(ssid1);
  Serial.println();
  printf("Version: %s\n", VERSION);
  pinMode(STATUS_LED, OUTPUT);
  OTA_Func();
}

void loop() { 
  //Serial.println(WiFi.getPhyMode());
  ArduinoOTA.handle();
  #ifndef NO_ARTNET
    processData();
  #endif
}

// connect to wifi
boolean ConnectWifi(const char *ssid) {
  boolean state = true;
  int i = 0;
  WiFi.persistent(false);
  WiFi.config(ip, gateway, subnet_ip);
  WiFi.setPhyMode(WIFI_PHY_MODE_11G);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setOutputPower(10.0); //16.4 20max
  WiFi.enableAP(0);
  if(WiFi.SSID() == ssid && WiFi.psk() == password) {
    printf("**** Loading WiFi settings from ROM\n");
    WiFi.begin();
  }
  else {
      printf("**** Connecting with new settings\n");
      WiFi.begin(ssid, password);
  }
  Serial.printf("**** Connecting to WiFi. SSID: %s\n", ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
   if (i > 30){              // Wait 8s for connecting to WiFI
    state = false;
      break;
    }
    i++;
  }
  if(!state) {
    printf("**** Cant connect to WiFi SSID: \n");
  }
  else {
    printf("**** Connected to WiFi! SSID:");
    Serial.print(WiFi.SSID());
    printf(" IP: ");
    Serial.print(WiFi.localIP());
    Serial.println();
  }
   // Open ArtNet port for WIFI
    int res = wifiUdp.begin(ARTNET_PORT);
    #ifdef ADV_DEBUG
        if (res == 1) printf("**** Opened UDP socket (WIFI) on port :%d\n", ARTNET_PORT);
        if (res == 0) printf("xxxx error opening UDP(WIFI)(no available sockets)\n");
    #endif
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
void readWiFiUDP() {
  //printf("$$$$$$$$\n");
    if (wifiUdp.parsePacket()){//&& wifiUdp.destinationIP() == ip) {
      noSignalTime = millis(); //this will be compared with current time in processData function
      blackoutSetted = false; // allow blackout when no signal for a some time
        wifiUdp.read(hData, 18);
        //printf("Uni: %d\n", hData[14]);
     if ( hData[0] == 'A' && hData[4] == 'N' && startUniverse == hData[14]) {
       Serial.print("Source IP: ");
       Serial.println(wifiUdp.remoteIP().toString());
         uniSize = (hData[16] << 8) + (hData[17]);
         wifiUdp.read(uniData, uniSize);
         universe = hData[14];
         int dur = getTimeDuration();

#ifdef DEBUGMODE
         if(sizeof(uniData) == 514) { //*******************************************
           if(uniData[509] == 255) {
             mycounter = -1;
           }
         }
        //printf(" ** p_len: %d ** 358: %d\n", uniSize, uniData[357]);
        mycounter++;//***************************************************************
#endif
         #ifdef DROP_PACKETS
         if (dur > MIN_TIME) {
          #ifdef DEBUGMODE
            printf("%d  %d ms_wifi ** ", mycounter, dur);//************************************
          #endif
           #ifndef NO_WS
            long oldd = micros();
            sendWS();
            printf("wsTime: %lu\n", micros() - oldd);
           #endif
           
         }
         else {
          #ifdef DEBUGMODE
            printf("%d Dropped: %dms\n", mycounter, dur);
          #endif
         }
          #else 
            #ifndef NO_WS
              long oldd = micros();
              sendWS();
              printf("%d %d ms_wifi ** wsTime: %d\n", mycounter, dur, micros() - oldd);
            #endif
         #endif
      }    

      else if (hData[0] == 'C' && hData[1] == 'P' && hData[2] == UNI) {
      // Serial.println(hData[0]);
      // Serial.println(hData[1]);
      // Serial.println(hData[2]);
      // Serial.println(hData[3]);
      // Serial.println(hData[4]);
        request.command = hData[3];
        request.option = hData[4];
        request.sourceIP = wifiUdp.remoteIP();
        if(hData[3] == 'S') {
          request.mode = hData[5];
          request.autoMode = hData[6];
          request.numEff = hData[7];
          request.speed = hData[8];
          request.color = RgbColor(hData[9], hData[10], hData[11]);
        }
        formAnswer();
      }
    }
}

//Choosing which readUPD function to use
void processData() {
  switch (settings.mode) {
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
    case 3:
      break;
  }
}

void autoModeFunc() {
      switch (settings.autoMode) {
        case 0:
          setStaticColor(settings.readedRGB);
          break;
        case 1:
          chasePlayer();
          break;
        case 3:
          effectPlayer();
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