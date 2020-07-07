#include "helpers.h"

//Ethernet Settings
//const char* ssid; //SSID 
const char* ssid1 = (char*)"udp";
const char* password = "esp18650"; //PASSW 



//Wifi Settings
const uint8_t startUniverse = UNI; //****************************
IPAddress ip(2, 0, 0, UNI); //IP ADDRESS NODEMCU ****************
IPAddress gateway(2, 0, 0, 101); //IP ADDRESS РОУТЕРА 
IPAddress subnet_ip(255, 255, 255, 0); //SUBNET_IP

void setup() {
  Serial.begin(115200);
  delay(10);
  strip.Begin();
  test();
  ConnectWifi(ssid1);
  OTA_Func();
}

void loop() { 
  //Serial.println(WiFi.getPhyMode());
  ArduinoOTA.handle();
    readWiFiUDP();
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
  return state;
}


//Reading WiFi UDP Data (IRAM_ATTR) (ICACHE_FLASH_ATTR)
void readWiFiUDP() {
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

void sendWS() {
    for (int i = 0; i < PixelCount; i++)
    {
        RgbColor color(uniData[i * 3], uniData[i * 3 + 1], uniData[i * 3 + 2]);
        strip.SetPixelColor(i, color);
    } 
    strip.Show();
}
