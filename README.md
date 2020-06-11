# Artnet_WS281x_Esp8266
 Controls WS2812/13 via Artnet using esp8266

#define DROP_PACKETS
Using this allow dropping packets, if interval after previous packets is less then MIN_TIME (30ms for now)
#define MIN_TIME 15 Minimum time duration in ms between 2 received packets, when packet will be shown. (15 for Continuous mode, 30 for reduced)

GENERAL INFO:
WIFI SSID: udp
WIFI PASSWORD: esp18650
Router IP Address: 2.0.0.101
Nodes IP Address range: 2.0.0.21 - 2.0.0.63
Nodes works in unicast mode ONLY
For MagicQ use CONTINUOUS mode (33fps) or Reduced (60fps) or Mixed+Changes
Nodes Artnet universes range: 21-63 (node's working universe is equal to last IP address byte)

WORKING MODES:
WIFI
#LAN
AUTOMODE

AUTOMODE has a STATIC submode for showing static color which is writed in FS and CHASE submode. In CHASE submode, esp8266 plays packets from FS, which were stored there via RECORDING process.
  RECORDING process.
  channel 510 - number of stored effect (should be 0-9)
  channel 511 - stop recording (255 - stop)
  channel 512 - start recording (250 - start recording without autodetection of effect loop, 251 - 255 start recording with autodetection) Autodetection of loop means that when esp8266 detects, that while recording it receives packet, similar as was in 20th frame from the begining, it automatically stops recording
