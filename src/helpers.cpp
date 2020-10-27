#include "helpers.h"

WiFiUDP wifiUdp;
settings_t settings = {
  mode : 0, // WIFI or LAN or AUTO mode variable (0 - WIFI, 1 - LAN, 2 - AUTO, 3 - FIXTURE MODE)
  autoMode : 0, // mode for Automatic strip control
  speed : 0, //speed for playing effects from FS
  readedRGB : blue, //color for static automode
  chaseNum : 0, //number of internal chase
  dimmer : 255,
  //recordedEffNum : 0 //number of recorded effect
  universe: UNI,
  address: UNI%21*24 + 1,
  reverse: 0,
  pixelCount: 120,
  startPixel: 0,
  endPixel: 120,
  segment: 15
};
settings_t temp_set;
request_t request;
fixture_t fixtureData;

 Recorder recorder = Recorder(settings.pixelCount, &writingFlag);

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
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(settings.pixelCount, PixelPin);

boolean isFading = false;
uint8_t fred = 0;
uint8_t fgreen = 0;
uint8_t fblue = 0;
uint8_t ffdim = 0;
int fade_ms = 600;
uint8_t fade_frame_dur = 30;

//UDP Settings
uint8_t uniData[514]; 
uint16_t uniSize; 
//extern uint8_t net = 0; 
uint8_t universe; 
//uint8_t subnet = 0;
uint8_t hData[ARTNET_HEADER + 1];
uint8_t hData2[4];

void initModes() {
  if(!LittleFS.exists(FILE_MODES)) {
    printf("**** Mode_file not exists, creating...\n");
    File f = LittleFS.open(FILE_MODES, "w+");
    if (!f) {
      printf("**** Open mode_file for writing fails\n");
    } 
    else {
      uint8_t addr_low = 0;
      uint8_t addr_high = 0;
      if(settings.address > 255) {
        addr_low = 255;
        addr_high = settings.address%255;
      }
      else {
        addr_low = settings.address;
        addr_high = 0;
      }
      printf("**** mode_file opened. Writing...\n");
      f.write(settings.mode);
      f.write(settings.autoMode);
      f.write(0);
      f.write(settings.speed);
      f.write(settings.readedRGB.R);
      f.write(settings.readedRGB.G);
      f.write(settings.readedRGB.B);
      f.write(settings.dimmer);
      f.write(settings.universe);
      f.write(addr_low);
      f.write(addr_high);
      f.write(settings.reverse);
      f.write(settings.pixelCount);
      f.write(settings.startPixel);
      f.write(settings.endPixel);
      f.write(settings.segment);
      delay(50);
      f.close();
    }
  }
  else {
    printf("Reading values from mode_file\n");
    fillSettingsFromFs(&settings);
    printf("Readed!\n");
  }
}

void saveSettingsToFs() {
  File f = LittleFS.open(FILE_MODES, "w");
  if(!f) {
      printf("**** Open mode_file for writing fails\n");
  }
  else {
    uint8_t addr_low = 0;
    uint8_t addr_high = 0;
    if(settings.address > 255) {
      addr_low = 255;
      addr_high =  settings.address%255;
    }
    else {
      addr_low = settings.address;
      addr_high = 0;
    }
    f.write(settings.mode);
    f.write(settings.autoMode);
    f.write(settings.chaseNum);
    f.write(settings.speed);
    f.write(settings.readedRGB.R);
    f.write(settings.readedRGB.G);
    f.write(settings.readedRGB.B);
    f.write(settings.dimmer);
    f.write(settings.universe);
    f.write(addr_low);
    f.write(addr_high);
    f.write(settings.reverse);
    f.write(settings.pixelCount);
    f.write(settings.startPixel);
    f.write(settings.endPixel);
    f.write(settings.segment);
    delay(50);
    f.close();
  }
  printf("Saved Settings\n");
}

void fillSettingsFromFs(settings_t* temp_set) {
  File f = LittleFS.open(FILE_MODES, "r");
  uint8_t temp[16];
  f.read(temp, 16);
  *temp_set = {
    mode : temp[0],
    autoMode : temp[1],
    speed: temp[3],
    readedRGB : RgbColor(temp[4], temp[5], temp[6]),
    chaseNum : temp[2],
    dimmer : temp[7],
    universe : temp[8],
    address : temp[9] + temp[10],
    reverse: temp[11],
    pixelCount: temp[12],
    startPixel: temp[13],
    endPixel: temp[14],
    segment: temp[15]
  };
  f.close();
}

void formatFS() {
  LittleFS.format();
  ESP.restart();
}

void processRequest() {
  printf("Receive: \n");
  printf("  Command: %c, Option: %c, Mode: %d, Automode: %d\n", request.command, request.option, request.mode, request.autoMode);
  printf("  R: %d, G: %d, B: %d, D: %d, mask: %d\n", request.color.R, request.color.G, request.color.B, request.dimmer, request.mask);
  printf("  pCount: %d, startPix: %d, endPix: %d, reverse: %d, seg: %d\n", request.pixelCount, request.startPixel, request.endPixel, request.reverse, request.segment);
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
    formAnswerInfo(ARTNET_PORT_OUT);
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
  case 'F':
    formatFS();
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
  if (settings.mode != request.mode || settings.autoMode != request.autoMode || settings.chaseNum != request.numEff) {
    recorder.tryStopReading();
  }
  switch (request.mask)
  {
  case 1:
    settings.dimmer = request.dimmer;
    break;
  case 2:
    settings.readedRGB = request.color;
    break;
  case 4:
    settings.speed = request.speed;
    break;
  case 8:
    settings.mode = request.mode;
    if(settings.mode == 0) {
      settings.universe = UNI;
    }
    else if(settings.mode == 3) {
      settings.universe = UNIVERSE;
    }
    settings.autoMode = request.autoMode;
    settings.chaseNum = request.numEff;
    break;
  case 15:
    settings.dimmer = request.dimmer;
    settings.readedRGB = request.color;
    settings.speed = request.speed;
    settings.mode = request.mode;
    settings.autoMode = request.autoMode;
    settings.chaseNum = request.numEff;
    break;
  case 32:
    //settings.universe = request.universe;
    settings.address = request.address;
    settings.reverse = request.reverse;
    settings.segment = request.segment;
    saveSettingsToFs();
    break;
  case 64:
    settings.pixelCount = request.pixelCount;
    settings.startPixel = request.startPixel;
    settings.endPixel = request.endPixel;
    //saveSettingsToFs();
    break;
  case 128:
    isFading = true;
    fred = request.color.R;
    fgreen = request.color.G;
    fblue = request.color.B;
    ffdim = request.dimmer;
    break;
  case 255:
    saveSettingsToFs();
    break;
  default:
    printf("**** Unknown mask\n");
    break;
  }
    formAnswerInfo(ARTNET_PORT_OUT_UPD);
}

void setDmxAddress() {

}

void formAnswerInfo(int port) {
  fillSettingsFromFs(&temp_set);
  wifiUdp.beginPacket(request.sourceIP, port);
  wifiUdp.write("CP"); //0-1
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
  wifiUdp.write(settings.readedRGB.B); //23
  wifiUdp.write(temp_set.dimmer); //24
  wifiUdp.write(settings.dimmer); //25
  wifiUdp.write(settings.universe); //26
  if(settings.address - 255 > 0) wifiUdp.write(255); //27
  else wifiUdp.write(0); //27
  wifiUdp.write(settings.address); //28
  wifiUdp.write(settings.reverse); //29
  wifiUdp.write(settings.pixelCount); //30
  wifiUdp.write(settings.startPixel); //31
  wifiUdp.write(settings.endPixel); //32
  wifiUdp.write(settings.segment); //33
  wifiUdp.endPacket();
  printf("Answer: a: %d, u: %d, r: %d, p: %d, st: %d, end: %d, seg: %d\n", settings.address, settings.universe, settings.reverse, settings.pixelCount, settings.startPixel, settings.endPixel, settings.segment);
}

void chasePlayer(uint8_t chaseNum, uint8_t speed, uint8_t dimmer) {
  if (chaseNum == 1) {
    chaserColor(speed);
  }
  else if(chaseNum >= 11 && chaseNum <= 19) {
    recorder.setFile(chaseNum);
    if(LittleFS.exists(recorder.filename)){
      sendWSread(recorder.readPacket(chaseNum, speed), dimmer);
    }
  }
  else {
    printf("**** error chasenum\n");
  }
}

void effectPlayer() {

}

void sendStartRecording() {
  printf("sendStartRecording\n");
  wifiUdp.beginPacket(request.sourceIP, ARTNET_PORT_OUT_REC);
  wifiUdp.write("CP");
  wifiUdp.write(UNI);
  wifiUdp.write('r');
  wifiUdp.endPacket();
}

void sendStopRecording() {
  printf("sendStopRecording\n");
size_t f_size = 1;
  char size_string[7];
  //size_string = (char*)"xxxxxxx";
  if (LittleFS.exists(recorder.filename)) {
    File f = LittleFS.open(recorder.filename, "r");
    f_size = f.size();
    sprintf(size_string, "%d", f_size);
    f.close();
  }
  else f_size = 0;
  uint8_t sizelength = (uint8_t)log10(f_size)+1;
  printf("%s %s\n", recorder.filename, size_string);
  wifiUdp.beginPacket(request.sourceIP, ARTNET_PORT_OUT_REC);
  wifiUdp.write("CP");
  wifiUdp.write(UNI);
  wifiUdp.write('f');
  wifiUdp.write(sizelength);
  wifiUdp.write(recorder.filename);
  wifiUdp.write(size_string);
  wifiUdp.endPacket();
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
  for(int i = 0; i < settings.pixelCount; i++) {
    strip.SetPixelColor(i, redd);
  }
  //strip.Show();
  showStrip();
  delay(500);
  for(int i = 0; i < settings.pixelCount; i++) {
    strip.SetPixelColor(i, grenn);
  }
  //strip.Show();
  showStrip();
  delay(500);
  for(int i = 0; i < settings.pixelCount; i++) {
    strip.SetPixelColor(i, bluee);
  }
  //strip.Show();
  showStrip();
  delay(100);
    for(int i = 0; i < settings.pixelCount; i++) {
    strip.SetPixelColor(i, black);
  }
  //strip.Show();
  showStrip();
  delay(500);
}

void test2() {
  RgbColor blu = RgbColor(0, 0, 50);
  for(int i = 0; i < 3; i++) {
    strip.SetPixelColor(settings.startPixel, blu);
    strip.SetPixelColor(settings.endPixel, blu);
    showStrip();
    delay(100);
    strip.SetPixelColor(settings.startPixel, black);
    strip.SetPixelColor(settings.endPixel, black);
    showStrip();
    delay(200);
  }
}

void chaserColor(int speed) {
  chaseColor = HslColor (chaseHue, 1.0f, 0.4f);
  for (int i = 0; i < settings.pixelCount; i++) strip.SetPixelColor(i, chaseColor);
  //strip.Show();
    showStrip();
    chaseHue = chaseHue + 0.005f;
    if (chaseHue >= 1.0) chaseHue = 0;
    //delay(260 - speed);
    delay(15);
  }

void  setStaticColor(RgbColor color) {
  for (int i = 0; i < settings.pixelCount; i++) {
    if(i < settings.startPixel || i > settings.endPixel) {
      strip.SetPixelColor(i, black);
    }
    else {
      strip.SetPixelColor(i, color);
    }
     //strip.Show();
   }
   showStrip();
   //delay(20);
 }

 void setStaticColorDimmed(uint8_t dimmer, RgbColor col) {
   float koeff = dimmer*1.0/255;
   RgbColor tmp_color(col.R*koeff, col.G*koeff, col.B*koeff);
   setStaticColor(tmp_color);
 }

 void setStaticColorDimmedFaded() {
   double temp_red = settings.readedRGB.R;
   double temp_green = settings.readedRGB.G;
   double temp_blue = settings.readedRGB.B;
   double temp_dim = settings.dimmer;
   double r_step = (fred - temp_red)/(fade_ms/fade_frame_dur);
   double g_step = (fgreen - temp_green)/(fade_ms/fade_frame_dur);
   double b_step = (fblue - temp_blue)/(fade_ms/fade_frame_dur);
   double d_step = (ffdim - temp_dim)/(fade_ms/fade_frame_dur);
   for(int i = 0; i < fade_ms/fade_frame_dur; i++) {
     temp_dim = temp_dim + d_step;
     temp_red = temp_red + r_step;
     temp_green = temp_green + g_step;
     temp_blue = temp_blue + b_step;
     RgbColor tmp_color(temp_red, temp_green, temp_blue);
     setStaticColorDimmed(temp_dim, tmp_color);
     delay(fade_frame_dur);
   }
   settings.dimmer = ffdim;
   settings.readedRGB = RgbColor(fred, fgreen, fblue);
   isFading = false;
   formAnswerInfo(ARTNET_PORT_OUT_UPD);
 }

 void fillFixtureData() {
   fixtureData.dimmer = uniData[0];
   fixtureData.shutter = uniData[1];
   fixtureData.red = uniData[2];
   fixtureData.green = uniData[3];
   fixtureData.blue = uniData[4];
   fixtureData.effect = uniData[5];
   fixtureData.speed = uniData[6];
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