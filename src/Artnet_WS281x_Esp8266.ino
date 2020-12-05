/*  platformio run -t upload --upload-port 2.0.0.51
//beacon interval an DTIM

--- Receiving ArtNet over WiFi using Esp8266
--- Output Artnet data to ws2812/13 using NeoPixelBus library
--- Catch one universe
--- uses GPIO3 (RX on Esp8266) GPIO2 for other boards

//WRiting to ws - ~418mcrsec
*/

#include "helpers.h"
#include <Ticker.h>

//Ethernet Settings
//const char* ssid; //SSID 
const char* ssid1 = (char*)"udp";
Ticker updateSendTicker;

long newTime = 0; // holds time for calculating time interval between packets (for DROP_PACLETS mode)
long noSignalTime = 0; // holds time for calculating cctime interval after last arrived packet (for NOSIGNAL blackout mode)
bool blackoutSetted = false; // used for avoiding blackout if no signal, when in RECORDED mode
int recordPacketsCounter = 0; // counting packets for allow saving received packet into FS (for avoiding signal noise issues)
int mycounter = -1; //counter of packets, need only for debugging and testing for printing in readWiFIUdp and readEthernetUdp methods
uint8_t writingFlag = 0; //indicates if esp is writing data to fs (when writing - 1 - DROP PACKETS is disabled, when - 0 - DROP PACKET enabled)

//Wifi Settings
const uint8_t startUniverse = settings.universe; //****************************
//IPAddress ip(2, 0, 0, UNI); //IP ADDRESS NODEMCU ****************
//IPAddress gateway(2, 0, 0, 101); //IP ADDRESS РОУТЕРА 
IPAddress ip(192, 168, 0, UNI); //IP ADDRESS NODEMCU ****************
IPAddress gateway(192, 168, 0, 101); //IP ADDRESS РОУТЕРА 
IPAddress subnet_ip(255, 255, 255, 0); //SUBNET_IP
const char* password = "esp18650"; //PASSW 

void update() {//for sending updated status to Application
  formAnswerInfo(ARTNET_PORT_OUT_UPD);
}

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
  //LittleFS.format();
  strip.Begin();
  initModes();
  FX.initFxData();////
  test2();
  ConnectWifi(ssid1);
  Serial.println();
  printf("Version: %s\n", VERSION);
  pinMode(STATUS_LED, OUTPUT);
  OTA_Func();
  recorder.setFunc(sendStartRecording, sendStopRecording);
}

void loop() { 
  //Serial.println(WiFi.getPhyMode());
  ArduinoOTA.handle();
  #ifndef NO_ARTNET
    processData();
  #endif
}

void reconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    ConnectWifi(ssid1);
  }
}

// connect to wifi
boolean ConnectWifi(const char *ssid) {
  boolean state = true;
  int i = 0;
  WiFi.persistent(false);
  WiFi.config(ip, gateway, subnet_ip);
  WiFi.setPhyMode(WIFI_PHY_MODE_11G);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setOutputPower(17.0); //16.4 20max
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
        wifiUdp.read(hData, 5);
        if(hData[0] == 'A') {
          wifiUdp.read(hData1, 13);
        }
        //printf("0 - %d, 1 - %d, 2 - %d", hData[0], hData[1], hData[2]);
     //if ( hData[0] == 'A' && hData[4] == 'N' && startUniverse == hData[14]) {
       if ( hData[0] == 'A' && hData1[9] == settings.universe) {
         uniSize = (hData1[11] << 8) + (hData1[12]);
         wifiUdp.read(uniData, uniSize);
         universe = hData1[9];
         //int dur = getTimeDuration();

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
            //printf("%d  %d ms_wifi ** ", mycounter, dur);//************************************
          #endif
           #ifndef NO_WS
            long oldd = micros();
            recorder.writePacket(uniData, uniData[509], uniData[510], uniData[511]);
            if (settings.mode == STATUS_WIFI) sendWS();
            //printf("wsTime: %lu\n", micros() - oldd);
           #endif
         }
         else {
           if (writingFlag) {
              recorder.writePacket(uniData, uniData[509], uniData[510], uniData[511]);
              if (settings.mode == STATUS_WIFI) sendWS();
           } else {
              #ifdef DEBUGMODE
                printf("%d Dropped: %dms\n", mycounter, dur);
              #endif
           }

         }
          #else 
            #ifndef NO_WS
              //long oldd = micros();
              recorder.writePacket(uniData, uniData[509], uniData[510], uniData[511]);
             if (settings.mode == STATUS_WIFI) sendWS(); //*****************************************
             if (settings.mode == STATUS_FIXT) sendWS_addressed(); //*****************************************
              //printf("%d %d ms_wifi ** wsTime: %lu\n", mycounter, dur, micros() - oldd);
            #endif
         #endif
      }    
      else if (hData[0] == 'C' && hData[2] == UNI) {
        //printf("taddammmm\n");
        if(!compareIpAddresses(sourceIP, wifiUdp.remoteIP())) {
          sourceIP = wifiUdp.remoteIP();
          saveIpToFs();
        }
        request.command = hData[3];
        request.option = hData[4];
        //request.sourceIP = wifiUdp.remoteIP();
        if(hData[3] == 'S') {
          if(hData[4] == 'S') {
            wifiUdp.read(hData1, 13);
            wifiUdp.read(hData2, 17);
            request.mode = hData1[0];
            request.autoMode = hData1[1];
            request.numEff = hData1[2];
            request.fxSpeed = speedToDouble(hData1[3]);
            request.color = RgbColor(hData1[4], hData1[5], hData1[6]);
            request.dimmer = hData1[7];
            //request.save = hData[12];
            request.mask = hData1[8];
            request.universe = hData1[9];
            request.address = hData1[10] + hData1[11];
            request.reverse = hData1[12];
            request.pixelCount = hData2[0];
            request.startPixel = hData2[1];
            request.endPixel = hData2[2];
            request.segment = hData2[3];
            ///////////////////
            request.fxColor = RgbColor(hData2[7], hData2[8], hData2[9]);
            request.strobe = hData2[10];
            request.fxSize = hData2[11];
            request.fxParts = hData2[12];
          if(request.fxParts != settings.fxParts) {
            FX.needRecalculate = true;
          }
          request.fxFade = hData2[13];
          request.fxParams = hData2[14];
          settings.fxParams = request.fxParams;
          request.fxSpread = hData2[15];
          request.fxWidth = hData2[16];
          printf("** udp read, recSize: %d, width: %d\n", request.fxSize, hData2[16]);
          if(request.fxSpread != settings.fxSpread) {
            FX.needRecalculate = true;
          }
          request.pixelCount = hData2[0] + (hData2[4]<<8);
          request.startPixel = hData2[1] + (hData2[5]<<8);
          request.endPixel = hData2[2] + (hData2[6]<<8);
            //printf("SetSettings: hdata15: %d, hdata16: %d, addr: %d, reverse: %d, pC: %d, stP: %d, eP: %d, seg: %d\n", hData[15], hData[16], hData[15] + hData[16], hData[17], hData2[0], hData2[1], hData2[2], hData2[3]);
          }
          //if name settings
        if(hData[4] == 'N') {
          uint8_t nameSize = wifiUdp.read();
          if(nameSize > 0) {
            settings.name = new char[nameSize+1];
            wifiUdp.read(settings.name, nameSize);
            settings.name[nameSize] = '\0';
            saveNameToFs(false);
          }
        }

         //if playlist settings
        if(hData[4] == 'L') {
          printf("PL set receive\n");
          uint8_t plSize = wifiUdp.read();
          uint8_t low = wifiUdp.read();
          uint8_t high = wifiUdp.read();
          playlistPeriod = low + (high<<8);
          delay(10);
          playlist = new ledsettings_t[plSize];
          playlist_temp = playlist;
          printf("plSize: %d, plPeriod: %d\n", plSize, playlistPeriod);
          for(int i = 0; i < plSize; i++) {
            ledsettings_t set;
            set.dimmer = wifiUdp.read();
            set.color.R = wifiUdp.read();
            set.color.G = wifiUdp.read();
            set.color.B = wifiUdp.read();
            set.fxColor.R = wifiUdp.read();
            set.fxColor.G = wifiUdp.read();
            set.fxColor.B = wifiUdp.read();
            set.strobe = wifiUdp.read();
            set.fxNumber = wifiUdp.read();
            set.fxSpeed = speedToDouble(wifiUdp.read());
            set.fxSize = wifiUdp.read();
            set.fxParts = wifiUdp.read();
            set.fxFade = wifiUdp.read();
            set.fxParams = wifiUdp.read();
            set.fxSpread = wifiUdp.read();
            set.fxWidth = wifiUdp.read();
            set.fxReverse = (set.fxParams)&1;
            set.fxAttack = (set.fxParams>>1)&1;
            set.fxSymm = (set.fxParams>>2)&1;
            set.fxRnd = (set.fxParams>>3)&1;
            set.fxRndColor = (set.fxParams>>7)&1;
            *playlist = set;
            playlist++;
          }
          playlist = playlist_temp;
          settings.playlistSize = plSize;
          savePlaylist();
        }
          
        }
        processRequest();
      }
    }
}

void stopUpdate() {
  if(updateSendTicker.active()) {
        updateSendTicker.detach();
      }
}

void startUpdate() {
  if(!updateSendTicker.active()) {
    updateSendTicker.attach(2, update);
  }
}

//Choosing which readUPD function to use
void processData() {
  switch (settings.mode) {
    case 0:
      stopUpdate();
      readWiFiUDP();
      if (noSignalTime == 0) noSignalTime = millis();
      if (((millis() - noSignalTime) > NO_SIG) && !blackoutSetted) {
        setStaticColor(black);
        blackoutSetted = true;
      }
      //sendWS();
      break;
    case 1:
      stopUpdate();
      #ifdef LAN_MODE
        readEthernetUDP();
      #endif
      if (noSignalTime == 0) noSignalTime = millis();
      if ((millis() - noSignalTime) > NO_SIG) setStaticColor(black);
      break;
    case 2:
      startUpdate();
      autoModeFunc();
      readWiFiUDP();
      break;
    case 3:
      stopUpdate();
      readWiFiUDP();
      if (noSignalTime == 0) noSignalTime = millis();
      if (((millis() - noSignalTime) > NO_SIG) && !blackoutSetted) {
        setStaticColor(black);
        blackoutSetted = true;
      }
      //sendWS_addressed();
      break;
    case 4:
    stopUpdate();
      readWiFiUDP();
      if(uniData[5] != fixtureData.effect) recorder.tryStopReading();
      fillFixtureData();
      if(fixtureData.effect == 0) {
        RgbColor dmxColor(fixtureData.red, fixtureData.green, fixtureData.blue);
        setStaticColorDimmed(fixtureData.dimmer, dmxColor);
      }
      else{
        chasePlayer(fixtureData.effect, fixtureData.speed, fixtureData.dimmer);
      }
      break;
  }
}

void autoModeFunc() {
      switch (settings.autoMode) {
        case 0:
          processFx();////
          outToStrip();
          processPlaylist(); ////
          break;
        case 1:
          chasePlayer(settings.fxNumber, settings.fxSpeed, settings.dimmer);
          break;
        case 3:
          effectPlayer();
          break;
      }
}

void stopFX() {
  if(fxTicker.active()) fxTicker.detach();
        if(fxFadeTicker.active()) fxFadeTicker.detach();
        FX.clearFxData();
        FX.fxRunning = false;
        FX.needRecalculate = true;
        FX.rndShouldGo = -1;
        FX.prevIndex = -1;
        FX.lastPixel = 0;
        delay(40);
}

void processFx() {
  switch(settings.fxNumber) {
    case 0: //Stopped FX
      if(FX.fxRunning) {
        stopFX();
        FX.fxRunning = false;
        FX.needRecalculate = true;
        if(FX.previousFxNum != 0) FX.previousFxNum = 0;
        //printf("Stopped FX...\n");
      }
      break;
    case 1: //Sinus FX
      if(!fxTicker.active()) {
        stopFX();
        fxTicker.attach_ms(1000/FX.fps, sinus);
        FX.fxRunning = true;
        FX.previousFxNum = 1;
      }
      if(fxTicker.active() && FX.previousFxNum != 1) {
        stopFX();
        fxTicker.attach_ms(1000/FX.fps, sinus);
        FX.previousFxNum = 1;
        FX.fxRunning = true;
      }
      break;
    case 2: //Cyclon FX
      if(FX.previousFxNum != 2) {
        stopFX();
        setupAnimations();
        FX.fxRunning = true;
        FX.previousFxNum = 2;
      }
      if(FX.speedChanged) {
        FX.animations.ChangeAnimationDuration(1, (uint16_t)((7 - speedNormal(settings.fxSpeed, 0.15, 7))*2000 + 20));
        FX.speedChanged = false;
      }
      FX.animations.UpdateAnimations();
      break;
    case 3: //Fade FX
      if(FX.previousFxNum != 3) {
        //printf("R: %d, G:%d, B:%d, fxSize: %d, fxParts: %d, fxSpread: %d, fxSpeed: %f\n", settings.fxColor.R, settings.fxColor.G, settings.fxColor.B, settings.fxSize, settings.fxParts, settings.fxSpread, settings.fxSpeed);
        stopFX();
        setupAnimationsCyclon();
        FX.fxRunning = true;
        FX.previousFxNum = 3;
      }
      if(FX.speedChanged) {
        FX.animations2.ChangeAnimationDuration(1, (uint16_t)((5 - speedNormal(settings.fxSpeed, 0.13, 5.0))*2000 + 20));
        FX.speedChanged = false;
      }
      FX.animations2.UpdateAnimations();
      break;
    case 4: //RGB FX
      if(!fxTicker.active()) {
        stopFX();
        fxTicker.attach_ms(1000/FX.fps, sinusRGB);
        FX.fxRunning = true;
        FX.previousFxNum = 4;
      }
      if(fxTicker.active() && FX.previousFxNum != 4) {
        stopFX();
        fxTicker.attach_ms(1000/FX.fps, sinus);
        FX.previousFxNum = 4;
        FX.fxRunning = true;
      }
      break;
    
    default:
      //printf("***Wrong fxNumber\n");
      break;
  }
}

void resetPlaylist() {
      playlist_counter = 0;
      playlist_temp = playlist;
}

void processPlaylist() {
  if(settings.playlistMode && settings.playlistSize > 0) {
    //printf("plmode: %d, plSize: %d\n", settings.playlistMode, settings.playlistSize);
    if((millis() - playlistLastTime) > playlistPeriodMs) {
      if(playlist_counter >= settings.playlistSize){
          while(playlist_counter > 0) {
            playlist_temp--;
            playlist_counter--;
          }
          //printf("playlistCounter: %d\n", playlist_counter);
         // printf("%p, %p\n", playlist_temp, playlist);
        }
        else {
          copyPlaylistSettings(settings, *playlist_temp);
          //printf("cur item: %p\n", playlist_temp);
          playlistLastTime = millis();
          playlist_counter++;
          playlist_temp++;
        }
    }
  }
}

void outToStrip() {
    if(!isFading) {
      setStaticColorDimmed(settings.dimmer, settings.color);
    }
    else {
      setStaticColorDimmedFaded();
    }
}

void sendWS() {
      for (int i = 0; i < settings.pixelCount; i++)
    {
        RgbColor color(uniData[i * 3], uniData[i * 3 + 1], uniData[i * 3 + 2]);
        if(i < settings.startPixel || i > settings.endPixel) {
            color = black;
          }
        if(settings.reverse == 1) {
          strip.SetPixelColor(settings.pixelCount - i - 1, color);
          //printf("pixel: %d, R: %d, G: %d, B: %d\n", PixelCount - i - 1, color.R, color.G, color.B);
        }
        else {
          strip.SetPixelColor(i, color);
          //printf("pixel: %d, R: %d, G: %d, B: %d\n", i, color.R, color.G, color.B);
        }
    } 
    //strip.Show(); 
    showStrip();
}

void sendWS_addressed() {
  int k = 1;
  int addr = settings.address - 1;
  RgbColor colorAddr(uniData[addr], uniData[addr + 1], uniData[addr + 2]);
  //printf("addr: %d, unidata[addr]: %d\n", addr, uniData[addr]);
    for (int i = 0; i < settings.pixelCount; i++)
    {
        //printf("i: %d, addr: %d, color: %d %d %d\n", i, addr, colorAddr.R, colorAddr.G, colorAddr.B);
      if(k == settings.segment + 1) {
        addr = addr + 3;
        k = 1;
        //color.R = uniData[addr];
        //color.G = uniData[addr + 1];
        //color.B = uniData[addr + 2];
        colorAddr = RgbColor(uniData[addr], uniData[addr + 1], uniData[addr + 2]);
        if(i < settings.startPixel || i > settings.endPixel) {
            colorAddr = black;
          }
      }
        
        if(settings.reverse == 1) {
          strip.SetPixelColor(settings.pixelCount - i - 1, colorAddr);
        }
        else {
          strip.SetPixelColor(i, colorAddr);
        }
        k = k + 1;
    } 
    //strip.Show(); 
    showStrip();
}

void sendWSread(uint8_t* dataa, uint8_t dimmer) {
  uint8_t re, gr, bl;
  float koeff = dimmer*1.0/255;
    for (int i = 0; i < settings.pixelCount; i++)
    {
      re = *dataa++;
      gr = *dataa++;
      bl = *dataa++;
        RgbColor color(re*koeff, gr*koeff, bl*koeff);
        if(i < settings.startPixel || i > settings.endPixel) {
            color = black;
          }
        strip.SetPixelColor(i, color);
    } 
    //strip.Show(); 
    showStrip();
}