#include "helpers.h"

WiFiUDP wifiUdp;
settings_t settings = {
  mode : 0, // WIFI or LAN or AUTO mode variable (0 - WIFI, 1 - LAN, 2 - AUTO, 3 - FIXTURE MODE)
  autoMode : 0, // mode for Automatic strip control
  speed : 0, //speed for playing effects from FS
  readedRGB : blue, //color for static automode
  chaseNum : 0, //number of internal chase
  //recordedEffNum : 0 //number of recorded effect
};
settings_t temp_set;
request_t request;

//NEOPIXEL Variables
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);
RgbColor highlite(150, 150, 150);
RgbColor before_highlite;
bool _highlite = false;
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

void initModes() {
  if(!LittleFS.exists(FILE_MODES)) {
    printf("**** Mode_file not exists, creating...\n");
    File f = LittleFS.open(FILE_MODES, "w+");
    if (!f) {
      printf("**** Open mode_file for writing fails\n");
    } 
    else {
      printf("**** mode_file opened. Writing...\n");
      f.write(settings.mode);
      f.write(settings.autoMode);
      f.write(0);
      f.write(settings.speed);
      f.write(settings.readedRGB.R);
      f.write(settings.readedRGB.G);
      f.write(settings.readedRGB.B);
      delay(50);
      f.close();
    }
  }
  else {
    printf("Reading values from mode_file\n");
    File f = LittleFS.open(FILE_MODES, "r");
    printf("File length: %d\n", f.size());
    uint8_t temp[7];
    f.read(temp, 7);
    f.close();
    fillSettingsFromFs(&settings);
    printf("Writed!\n");
  }
}

void saveSettingsToFs() {
  File f = LittleFS.open(FILE_MODES, "w");
  if(!f) {
      printf("**** Open mode_file for writing fails\n");
  }
  else {
    f.write(settings.mode);
    f.write(settings.autoMode);
    f.write(settings.chaseNum);
    f.write(settings.speed);
    f.write(settings.readedRGB.R);
    f.write(settings.readedRGB.G);
    f.write(settings.readedRGB.B);
    delay(50);
    f.close();
  }
  formAnswerInfo();
}

void processRequest() {
  printf("Command: %c, Option: %c, Mode: %d, Automode: %d\n", request.command, request.option, request.mode, request.autoMode);
  printf("R: %d, G: %d, B: %d, save: %d\n", request.color.R, request.color.G, request.color.B, request.save);
  Serial.println(request.sourceIP.toString());
  switch (request.command)
  {
  case 'G': //Remote command for getting some info
    processGetCommand();
    break;
  case 'S': //Remote command for setting some settings
    processSetCommand();
    break;
  
  default:
    printf("**** Remote command unknown\n");
    break;
  }
}

void processGetCommand() {
  switch (request.option)
  {
  case 'S': //option for getting some settings
    formAnswerInfo();
    break;
  
  default:
    printf("**** Remote option unknown\n");
    break;
  }
}

void processSetCommand() {
  switch (request.option)
  {
  case 'S': //option for setting some settings to esp
    setRemoteColor();
    break;
  case 'H': //option for setting highlite mode
    setHighliteMode();
    break;
  case 'h': //option for unsetting highlight mode
    unsetHighliteMode();
    break;
  case 'R':
    setReset();
    break;
  case 'C':
  /*****/
    break;
  default:
    printf("**** Unknown set option\n");
    break;
  }
}

void setHighliteMode() {
  before_highlite = strip.GetPixelColor(1);
  setStaticColor(highlite);
  _highlite = true;
}

void unsetHighliteMode() {
  _highlite = false;
  setStaticColor(black);
}

void setReset() {
  ESP.restart();
}

void setRemoteColor() {
  printf("Setting remote settings\n");
  settings.mode = request.mode;
  settings.autoMode = request.autoMode;
  settings.chaseNum = request.numEff;
  settings.speed = request.speed;
  settings.readedRGB = request.color;
  if(request.save) {
    saveSettingsToFs();
  }
}

void formAnswerInfo() {
  fillSettingsFromFs(&temp_set);
  wifiUdp.beginPacket(request.sourceIP, ARTNET_PORT_OUT);
  wifiUdp.write("CP");
  wifiUdp.write(UNI);
  wifiUdp.write(VERSION);
  wifiUdp.write(temp_set.mode);
  wifiUdp.write(settings.mode);
  wifiUdp.write(temp_set.autoMode);
  wifiUdp.write(settings.autoMode);
  wifiUdp.write(temp_set.chaseNum);
  wifiUdp.write(settings.chaseNum);
  wifiUdp.write(temp_set.speed);
  wifiUdp.write(settings.speed);
  wifiUdp.write(temp_set.readedRGB.R);
  wifiUdp.write(temp_set.readedRGB.G);
  wifiUdp.write(temp_set.readedRGB.B);
  wifiUdp.write(settings.readedRGB.R);
  wifiUdp.write(settings.readedRGB.G);
  wifiUdp.write(settings.readedRGB.B);
  wifiUdp.endPacket();
}

void fillSettingsFromFs(settings_t* temp_set) {
  File f = LittleFS.open(FILE_MODES, "r");
  uint8_t temp[7];
  f.read(temp, 7);
  *temp_set = {
    mode : temp[0],
    autoMode : temp[1],
    speed: temp[3],
    readedRGB : RgbColor(temp[4], temp[5], temp[6]),
    chaseNum : temp[2],
  };
  f.close();
}

void chasePlayer() {
  switch (settings.chaseNum)
  {
  case 0:
    chaserColor(settings.speed);
    break;
  
  default:
    break;
  }
}

void effectPlayer() {

}

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
    case 3:
        return (char*) "FixtMode";
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
        return (char *)"STATIC";
        break;
    case 1:
        return (char *)"CHASE";
        break;
    case 2:
        return (char *)"RECORDED";
        break;
    default:
        return (char*)"AutoModError";
        break;
    }
}

void showStrip() {
  if (!_highlite) {
    strip.Show();
  }
}

void test() {
  RgbColor redd = RgbColor(30, 0, 0);
  RgbColor grenn = RgbColor(0, 30, 0);
  RgbColor bluee = RgbColor(0, 0, 30);
  for(int i = 0; i < PixelCount; i++) {
    strip.SetPixelColor(i, redd);
  }
  //strip.Show();
  showStrip();
  delay(500);
  for(int i = 0; i < PixelCount; i++) {
    strip.SetPixelColor(i, grenn);
  }
  //strip.Show();
  showStrip();
  delay(500);
  for(int i = 0; i < PixelCount; i++) {
    strip.SetPixelColor(i, bluee);
  }
  //strip.Show();
  showStrip();
  delay(100);
    for(int i = 0; i < PixelCount; i++) {
    strip.SetPixelColor(i, black);
  }
  //strip.Show();
  showStrip();
  delay(500);
}

    void chaserColor(int speed) {
        chaseColor = HslColor (chaseHue, 1.0f, 0.4f);
        for (int i = 0; i < PixelCount; i++) strip.SetPixelColor(i, chaseColor);
        //strip.Show();
        showStrip();
        chaseHue = chaseHue + 0.005f;
        if (chaseHue >= 1.0) chaseHue = 0;
        delay(260 - speed);
    }

  void  setStaticColor(RgbColor color) {
    for (int i = 0; i < PixelCount; i++) {
      strip.SetPixelColor(i, color);
      //strip.Show();
      showStrip();
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