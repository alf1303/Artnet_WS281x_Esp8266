# Artnet_WS281x_Esp8266
 Controls WS2812/13 via Artnet using esp8266 and ENC28j60 for LAN

#define FLASH_SELECT
Use this if changing runmode from WIFI to LAN should be with inbuilt FLASH button. While loading ESP reads byte from address '0' in EEPROM (1 - LAN mode, 0, 2-255 - WIFI mode) and set up ESP to according mode. Pressing FLASH button changes Led indicator (on - LAN, off - WIFI) and writes to EEPROM new value, which will be used during next load.

#define EXTERNAL_SELECT
Use this if changing runmode should be with external button, connected to D1 (GPIO 5). LOW is WIFI, HIGH is LAN. Setting mode performs while loading ESP. Led indicator shows current mode (ON - WIFI, OFF - LAN)

#define DROP_PACKETS
Using this allow dropping packets, if interval after previous packets is less then MIN_TIME (30ms for now)
#define MIN_TIME 15 Minimum time duration in ms between 2 received packets, when packet will be shown. (15 for Continuous mode, 30 for reduced)

GENERAL INFO:
WIFI SSID: ANetEsp
WIFI PASSWORD: ktulhu_1234
Router IP Address: 2.0.0.101
Nodes IP Address range: 2.0.0.21 - 2.0.0.63
Nodes works in unicast mode ONLY
For MagicQ use CONTINUOUS mode
Nodes Artnet universes range: 21-63 (node's working universe is equal to last IP address byte)

WORKING MODES:
WIFI
-Led near RESET button is OFF
LAN
-Led near RESET button is ON
AUTOMODE
-Led near ESP chip is ON, when OFF, AUTOMODE is disabled

Changing Modes:
- Pressing button FLASH shortly changes mode between WIFI and LAN  (Led near RESET button indicates)
  Switching power OFF/ON needed for starting new selected mode

- Pressing button FLASH for more than 5 seconds switch to AUTOMODE (Led near ESP chip indicates)
  Then by shortly presses FLASH button you can scroll between automodes (CHASE, WHITE, RED, GREEN, BLUE)

- Press FLASH button for more then 5 seconds for switching to Artnet mode (WIFI or LAN)

- RECORDED mode. Saves reveived packet to FS and play it after reset(ON/OFF). For entering this mode you need to send ARTNET packet with needed data (510 = 170leds*3colors) 510 bytes, and byte number 511 have to hold value 175. For saving, ESP needs to receive at least 25 packets with data and byte number 512 setted to 201.
