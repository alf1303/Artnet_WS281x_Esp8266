# Artnet_WS281x_Esp8266
 Controls WS2812/13 via Artnet using esp8266 and ENC28j60 for LAN

#define FLASH_SELECT
Use this if changing runmode from WIFI to LAN should be with inbuilt FLASH button. While loading ESP reads byte from address '0' in EEPROM (1 - LAN mode, 0, 2-255 - WIFI mode) and set up ESP to according mode. Pressing FLASH button changes Led indicator (on - LAN, off - WIFI) and writes to EEPROM new value, which will be used during next load.

#define EXTERNAL_SELECT
Use this if changing runmode should be with external button, connected to D1 (GPIO 5). LOW is WIFI, HIGH is LAN. Setting mode performs while loading ESP. Led indicator shows current mode (ON - WIFI, OFF - LAN)

Used libraries:
