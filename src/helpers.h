// ARTNET CODES
#define ARTNET_DATA 0x50
#define ARTNET_POLL 0x20
#define ARTNET_POLL_REPLY 0x21
#define ARTNET_PORT 6454
#define ARTNET_HEADER 17

//NEOPIXEL Variables
#define colorSaturation 200 //brightness for AUTO mode
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

char* convertModes(int mod); //Converts digital values to String names for General mode
char* convertAutoModes(int automod); //Converts digital values to String names for Auto modes
void chaserColor();
void setStaticColor(RgbColor);