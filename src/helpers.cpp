#include "helpers.h"
uint8_t mode; // WIFI or LAN or AUTO mode variable (0 - WIFI, 1 - LAN, 2 - AUTO)
uint8_t autoMode; // mode for Automatic strip control
//NEOPIXEL Variables
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);
HslColor chaseColor;  // for CHASE submode of AUTO mode
float chaseHue = 0.0f; // for CHASE submode of AUTO mode
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);

//UDP Settings
uint8_t uniData[514]; 
uint16_t uniSize; 
//extern uint8_t net = 0; 
uint8_t universe; 
//uint8_t subnet = 0;
uint8_t hData[ARTNET_HEADER + 1];

char* convertModes(int mod) {
    switch (mod)
    {
    case 0:
        return (char *)"WIFI";
        break;
    case 1:
        return (char *)"LAN";
        break;
    case 2:
        return (char *)"AutoMode";
        break;
    default:
        return (char *)"ErrorMode";
        break;
    }
}

char* convertAutoModes(int automod)  {
    switch (automod)
    {
    case 0:
        return (char *)"CHASE";
        break;
    case 1:
        return (char *)"WHITE";
        break;
    case 2:
        return (char *)"RED";
        break;
    case 3:
        return (char *)"GREEN";
        break;
    case 4:
        return (char *)"BLUE";
        break;
    default:
        return (char *)"ErrorMode";
        break;
    }
}

void test() {
  RgbColor redd = RgbColor(30, 0, 0);
  RgbColor grenn = RgbColor(0, 30, 0);
  RgbColor bluee = RgbColor(0, 0, 30);
  for(int i = 0; i < PixelCount; i++) {
    strip.SetPixelColor(i, redd);
  }
  strip.Show();
  delay(500);
  for(int i = 0; i < PixelCount; i++) {
    strip.SetPixelColor(i, grenn);
  }
  strip.Show();
  delay(500);
  for(int i = 0; i < PixelCount; i++) {
    strip.SetPixelColor(i, bluee);
  }
  strip.Show();
  delay(100);
    for(int i = 0; i < PixelCount; i++) {
    strip.SetPixelColor(i, black);
  }
  strip.Show();
  delay(500);
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

  
bool ssid_selector(uint8_t uni) {
  if (uni <= 30) {
    return false;
  }
  else {
    return true;
  }
}
