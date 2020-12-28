#include "helpers.h"

bool noWs = false;
bool noArtNet = false;
bool noAll = false;
uint8_t debugValue = 0;


WiFiUDP wifiUdp;
settings_t settings = {
  mode : 0, // WIFI or LAN or AUTO mode variable (0 - WIFI, 1 - LAN, 2 - AUTO, 3 - FIXTURE MODE)
  autoMode : 0, // mode for Automatic strip control
  fxSpeed : 0.3, //speed for playing effects from FS
  color : blue, //color for static automode
  fxNumber : 0, //number of internal chase
  dimmer : 255,
  //recordedEffNum : 0 //number of recorded effect
  universe: UNI,
  address: UNI%21*24 + 1,
  reverse: 0,
  pixelCount: 120,
  startPixel: 0,
  endPixel: 120,
  segment: 15,
  /////////////
  (char*)"esp001\0",
    playlistSize: 0,
    playlistMode: false,
    fxColor: blue,
    strobe: 255,
    fxSize: 100,
    fxParts: 1,
    fxFade: 0,
    fxParams: 0,
    fxSpread: 1,
    fxWidth: 1,
    fxReverse: false,
    fxAttack: false,
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

IPAddress sourceIP;

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
uint8_t hData[5];
uint8_t hData1[13];
uint8_t hData2[17];

void initModes() {
  if(!LittleFS.exists(FILE_MODES)) {
    printf("**** Mode_file not exists, creating...\n");
    saveSettingsToFs();
  }
  if(!LittleFS.exists(NAME_FILE)) {
    printf("***namefile not exists, creating...\n");
    saveNameToFs(true);
  }
  if(!LittleFS.exists(IP_FILE)) {
    printf("***namefile not exists, creating...\n");
    saveIpToFs();
  }

    printf("Reading values from mode_file\n");
    //fillSettingsFromFs(&settings);
    loadSettingsFromFs();
    FX.setSettings(&settings);// copy settings address to FX object
    printf("Readed!\n");
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
    f.write(settings.mode); //0
    f.write(settings.autoMode); //1
    f.write(settings.fxNumber); //2
    f.write(speedToInt(settings.fxSpeed)); //3 
    f.write(settings.color.R); //4
    f.write(settings.color.G); //5
    f.write(settings.color.B); //6
    f.write(settings.dimmer); //7
    f.write(settings.universe); //8
    f.write(addr_low); //9
    f.write(addr_high); //10
    f.write(settings.reverse); //11
    f.write(settings.pixelCount); //12
    f.write(settings.startPixel); //13
    f.write(settings.endPixel); //14
    f.write(settings.segment); //15
    ///////////////////////////
    f.write(settings.fxColor.R); //16
    f.write(settings.fxColor.G); //17
    f.write(settings.fxColor.B); //18
    f.write(settings.strobe); //19
    f.write(settings.fxSize); //20
    f.write(settings.fxParts); //21
    f.write(settings.fxFade); //22
    f.write(settings.fxParams); //23
    f.write(settings.fxSpread); //24
    f.write(settings.fxWidth); //25
    f.write(playlistPeriod); //26
    f.write(playlistPeriod>>8); //27
    delay(50);
    f.close();
  }
  printf("Saved Settings\n");
}

void saveIpToFs() {
  File ipfile = LittleFS.open(IP_FILE, "w");
    if(!ipfile) {
    printf("**** Open ipfile for writing fails\n");
  }
  else{
    for(int i = 0; i < 4; i++) {
      ipfile.write(sourceIP[i]);
    }
    delay(50);
    ipfile.close();
  }
}

void loadIpFromFs() {
  File f = LittleFS.open(IP_FILE, "r");
  uint8_t temp[4];
  f.read(temp, 4);
  f.close();
  for(int i = 0; i < 4; i++) {
    sourceIP[i] = temp[i];
  }
}

void loadSettingsFromFs() {
  loadIpFromFs();
  File f = LittleFS.open(FILE_MODES, "r");
  uint8_t temp[28];
  f.read(temp, 28);
  f.close();
  settings.mode = temp[0];
  settings.autoMode = temp[1];
  settings.fxSpeed = speedToDouble(temp[3]);
  settings.color = RgbColor(temp[4], temp[5], temp[6]);
  settings.fxNumber = temp[2];
  settings.dimmer = temp[7];
  settings.universe = temp[8];
  settings.address = temp[9] + temp[10];
  settings.reverse = temp[11];
  settings.pixelCount = temp[12];
  settings.startPixel = temp[13];
  settings.endPixel = temp[14];
  settings.segment = temp[15];
  settings.fxColor = RgbColor(temp[16], temp[17], temp[18]);
  settings.strobe = temp[19];
  settings.fxSize = temp[20];
  settings.fxParts = temp[21];
  settings.fxFade = temp[22];
  settings.fxParams = temp[23];
  settings.fxSpread = temp[24];
  settings.fxWidth = temp[25];
  playlistPeriod = temp[26] + (temp[27]<<8);
  loadPlaylist();
}

void fillSettingsFromFs(settings_t* temp_set) {
  File f = LittleFS.open(FILE_MODES, "r");
  uint8_t temp[16];
  f.read(temp, 16);
  *temp_set = {
    mode : temp[0],
    autoMode : temp[1],
    fxSpeed: speedToDouble(temp[3]),
    color : RgbColor(temp[4], temp[5], temp[6]),
    fxNumber : temp[2],
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

void saveNameToFs(bool first) {
  File f_name = LittleFS.open(NAME_FILE, "w");
  if(!f_name) {
    printf("**** Open namefile for writing fails\n");
  }
  else {
    f_name.print(settings.name);
    delay(50);
    f_name.close();
    printf("name saved\n");
  }
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
  case 'a': //nows
    if(debugValue == 255) {
      noWs = true;
    }
    else {
      noWs = false;
    }
    break;
  case 'b': //noartnet
    if(debugValue == 255) {
      noArtNet = true;
    }
    else {
      noArtNet = false;
    }
    break;
  case 'c': //setpower
     if(debugValue >= 0 && debugValue <= 20) {
       WiFi.setOutputPower(debugValue*1.0f);
     } //16.4 20max
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
  if (settings.mode != request.mode || settings.autoMode != request.autoMode || settings.fxNumber != request.numEff) {
    recorder.tryStopReading();
  }
  switch (request.mask)
  {
  case 1:
    settings.dimmer = request.dimmer;
    break;
  case 3: //grandmaster
    settings.dimmer = request.dimmer;
    settings.fxSize = request.fxSize;
    break;
  case 2:
    settings.color = request.color;
    break;
  case 4:
    settings.fxSpeed = request.fxSpeed;
    break;
  case 8:
    settings.mode = request.mode;
    printf("*******************reqmode: %d, setMode: %d\n", request.mode, settings.mode);
    if(settings.mode == 0) {
      settings.universe = UNI;
    }
    else if(settings.mode == 3) {
      settings.universe = UNIVERSE;
    }
    settings.autoMode = request.autoMode;
    //settings.fxNumber = request.numEff;
    break;
  case 15:
    settings.dimmer = request.dimmer;
    settings.color = request.color;
    settings.fxSpeed = request.fxSpeed;
    settings.mode = request.mode;
    settings.autoMode = request.autoMode;
    //settings.fxNumber = request.numEff;
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
    ///////
    settings.fxColor = request.fxColor;
    settings.fxFade = request.fxFade;
    settings.fxNumber = request.numEff;
    settings.fxParts = request.fxParts;
    settings.fxParams = request.fxParams;
    settings.fxSize = request.fxSize;
    if(settings.fxSpeed != request.fxSpeed) {
      FX.speedChanged = true;
    }
    settings.fxSpeed = request.fxSpeed;
    settings.fxSpread = request.fxSpread;
    settings.fxWidth = request.fxWidth;
    if(settings.fxReverse != (request.fxParams&1)) {
      FX.needRecalculate = true;
    }
    settings.fxReverse = request.fxParams&1;
    settings.fxAttack = (request.fxParams>>1)&1;
    if(settings.fxSymm != ((request.fxParams>>2)&1)) {
      FX.needRecalculate = true;
    }
    settings.fxSymm = (request.fxParams>>2)&1;
     if(settings.fxRnd != ((request.fxParams>>3)&1)) {
      FX.needRecalculate = true;
    }
    settings.fxRnd = (request.fxParams>>3)&1;
    settings.fxRndColor = (request.fxParams>>7)&1;
    break;
  case 129:
    settings.fxColor = request.fxColor;
    break;
  case 130:
    settings.fxFade = request.fxFade;
    settings.fxNumber = request.numEff;
    settings.fxParts = request.fxParts;
    settings.fxParams = request.fxParams;
    settings.fxSize = request.fxSize;
    if(settings.fxSpeed != request.fxSpeed) {
      FX.speedChanged = true;
    }
    settings.fxSpeed = request.fxSpeed;
    settings.fxSpread = request.fxSpread;
    settings.fxWidth = request.fxWidth;
    if(settings.fxReverse != (request.fxParams&1)) {
      FX.needRecalculate = true;
    }
    settings.fxReverse = request.fxParams&1;
    settings.fxAttack = (request.fxParams>>1)&1;
    if(settings.fxSymm != ((request.fxParams>>2)&1)) {
      FX.needRecalculate = true;
    }
    settings.fxSymm = (request.fxParams>>2)&1;
    if(settings.fxRnd != ((request.fxParams>>3)&1)) {
      FX.needRecalculate = true;
    }
    settings.fxRnd = (request.fxParams>>3)&1;
    settings.fxRndColor = (request.fxParams>>7)&1;
    break;
  case 131: 
    settings.fxParams = request.fxParams;
    settings.playlistMode = (request.fxParams>>6)&1;
     if(!settings.playlistMode) {
      resetPlaylist();
    }
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

void printIpAddress(char* msg, IPAddress addr){
  printf("%s %d.%d.%d.%d\n", msg, addr[0], addr[1], addr[2], addr[3]);
}

void formAnswerInfo(int port) {
  printIpAddress((char*)"requestIP: ", sourceIP);
  wifiUdp.beginPacket(sourceIP, port);
  wifiUdp.write("CP"); //0-1
  wifiUdp.write(UNI);
  wifiUdp.write(VERSION);
  wifiUdp.write(temp_set.mode);
  wifiUdp.write(settings.mode);
  wifiUdp.write(temp_set.autoMode);
  wifiUdp.write(settings.autoMode);
  wifiUdp.write(temp_set.fxNumber);
  wifiUdp.write(settings.fxNumber);
  wifiUdp.write(speedToInt(temp_set.fxSpeed));
  wifiUdp.write(speedToInt(settings.fxSpeed));
  wifiUdp.write(temp_set.color.R);
  wifiUdp.write(temp_set.color.G);
  wifiUdp.write(temp_set.color.B);
  wifiUdp.write(settings.color.R);
  wifiUdp.write(settings.color.G);
  wifiUdp.write(settings.color.B); //23
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
  //////////////////////////////////
  wifiUdp.write(0); //34
  wifiUdp.write(0); //35
  wifiUdp.write(0); //36
  wifiUdp.write(settings.fxColor.R); //37
  wifiUdp.write(settings.fxColor.G); //38
  wifiUdp.write(settings.fxColor.B); //39
  wifiUdp.write(settings.strobe); //40
  wifiUdp.write(settings.fxSize); //41
  wifiUdp.write(settings.fxParts); //42
  wifiUdp.write(settings.fxFade); //43
  wifiUdp.write(settings.fxParams); //44
  wifiUdp.write(settings.fxSpread); //45
  wifiUdp.write(settings.fxWidth); //46
  wifiUdp.write(0); //47
  wifiUdp.write(strlen(settings.name)); //48
  wifiUdp.write(0); //49
  wifiUdp.write(0); //50
  wifiUdp.write(settings.name);
  wifiUdp.write(0);
  wifiUdp.write(0);
  wifiUdp.endPacket();
  printf("Answer: a: %d, u: %d, r: %d, p: %d, st: %d, end: %d, seg: %d\n", settings.address, settings.universe, settings.reverse, settings.pixelCount, settings.startPixel, settings.endPixel, settings.segment);
}

void chasePlayer(uint8_t fxNumber, uint8_t speed, uint8_t dimmer) {
  if (fxNumber == 1) {
    chaserColor(speed);
  }
  else if(fxNumber >= 11 && fxNumber <= 19) {
    recorder.setFile(fxNumber);
    if(LittleFS.exists(recorder.filename)){
      sendWSread(recorder.readPacket(fxNumber, speed), dimmer);
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
    strip.SetPixelColor(settings.endPixel-1, blu);
    showStrip();
    delay(100);
    strip.SetPixelColor(settings.startPixel, black);
    strip.SetPixelColor(settings.endPixel-1, black);
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

  void setPixelColor(int i, RgbColor A) {
    RgbColor B(FX.fxData[i].R, FX.fxData[i].G, FX.fxData[i].B);
    double fxDim = FX.normToDouble(settings.fxSize, 0, 100, 0.0, 1.0);
      RgbColor ccolor(RgbColor(A.R + fxDim*B.R, A.G + fxDim*B.G, A.B + fxDim*B.B));
    strip.SetPixelColor(i, ccolor);
}

void  setStaticColor(RgbColor color) {
  for (int i = 0; i < settings.pixelCount; i++) {
    if(i < settings.startPixel || i > settings.endPixel) {
      strip.SetPixelColor(i, black);
    }
    else {
      //strip.SetPixelColor(i, color);
      setPixelColor(i, color);
    }
   }
   showStrip();
 }

 void setStaticColorDimmed(uint8_t dimmer, RgbColor col) {
   float koeff = dimmer*1.0/255;
   RgbColor tmp_color(col.R*koeff, col.G*koeff, col.B*koeff);
   setStaticColor(tmp_color);
 }

 void setStaticColorDimmedFaded() {
   double temp_red = settings.color.R;
   double temp_green = settings.color.G;
   double temp_blue = settings.color.B;
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
   settings.color = RgbColor(fred, fgreen, fblue);
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

//min represents maximum speed, max - minimum speed
double speedNormal(double speed, double min, double max) {
  double result;
  double scale = (1.0*(max - min))/(SPEED_MAX_DOUBLE - SPEED_MIN_DOUBLE);
  if((SPEED_MIN_DOUBLE == 0 && min == 0) || (SPEED_MIN_DOUBLE != 0 && min != 0)) {
    result = speed*scale;
  }
  else if(SPEED_MIN_DOUBLE == 0) {
    result = speed*scale + min;
  }
  else if(min == 0) {
    result = speed*scale + max;
  }
  return result;
}

 uint8_t speedToInt(double speed) {
  double result;
  double scale = (1.0*(SPEED_MAX_INT - SPEED_MIN_INT))/(SPEED_MAX_DOUBLE - SPEED_MIN_DOUBLE);
  if((SPEED_MIN_DOUBLE == 0 && SPEED_MIN_INT == 0) || (SPEED_MIN_DOUBLE != 0 && SPEED_MIN_INT != 0)) {
    result = speed*scale;
  }
  else if(SPEED_MIN_DOUBLE == 0) {
    result = speed*scale + SPEED_MIN_INT;
  }
  else if(SPEED_MIN_INT == 0) {
    result = speed*scale + SPEED_MAX_INT;
  }
  return result;
}

double speedToDouble(uint8_t speed) {
  double result;
  double scale = (SPEED_MAX_DOUBLE - SPEED_MIN_DOUBLE)/(SPEED_MAX_INT - SPEED_MIN_INT);
   if((SPEED_MIN_DOUBLE == 0 && SPEED_MIN_INT == 0) || (SPEED_MIN_DOUBLE != 0 && SPEED_MIN_INT != 0)) {
    result = speed*scale;
  }
  else if(SPEED_MIN_INT == 0) {
    result = speed*scale + SPEED_MIN_DOUBLE;
  }
  else if(SPEED_MIN_DOUBLE == 0) {
    result = speed*scale + SPEED_MAX_DOUBLE;
  }
  return result;
}

bool compareIpAddresses(IPAddress a, IPAddress b) {
  bool result = true;
  for(int i = 0; i < 4; i++) {
    if(a[i] != b[i]) return false;
  }
  return result;
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