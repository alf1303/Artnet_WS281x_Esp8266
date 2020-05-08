#ifdef DROP_PACKETS 
#define MIN_TIME 30 // Minimum time duration between 2 packets for allowing show packets (in milliseconds)
#endif
long newTime = 0; // holds time for calculating time interval between packets (for DROP_PACLETS mode)
long noSignalTime = 0; // holds time for calculating time interval after last arrived packet (for NOSIGNAL blackout mode)
bool blackoutSetted = false; // used for avoiding blackout if no signal, when in RECORDED mode
int recordPacketsCounter = 0; // counting packets for allow saving received packet into FS (for avoiding signal noise issues)

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