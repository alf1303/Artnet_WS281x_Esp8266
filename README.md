# Artnet_WS281x_Esp8266
 Controls WS2812/13 via Artnet using esp8266
 

Compatible with Android application (for using without artnet): https://play.google.com/store/apps/details?id=alfarom.apps.ledcontroller

NOTE: After using mobile application esp8266 will no longer react on ARTNET data (TODO: add possibility of changing modes in application). For reverting to Artnet mode need to format LitteFS or change mode with help of service application

 Change #define UNI 33 for assigning destination artnet universe and IP Address

#define DROP_PACKETS
Using this allow dropping packets, if interval after previous packet is less then MIN_TIME (30ms for now)

#define MIN_TIME 15 Minimum time duration in ms between 2 received packets, when packet will be shown. (recommended - 15 for Continuous mode, 30 for reduced)

GENERAL INFO:

WIFI SSID: udp

WIFI PASSWORD: esp18650

Router IP Address: 192.168.0.101

Nodes IP Address range: 192.168.0.21 - 192.168.0.63

Nodes works better in broadcast mode

For MagicQ use CONTINUOUS mode (33fps) or Reduced (15fps) or Mixed+Changes

Nodes Artnet universes range: 21-63 (node's working universe is equal to last IP address byte)

WORKING MODES:

 - WIFI

 - #LAN not active  now

 - AUTOMODE
 
 - FIXTURE

Compatible with Android application: https://play.google.com/store/apps/details?id=alfarom.apps.ledcontroller


////////////////OLD, but present in firmware. Also there is new mode for joining leds in segments for occupying smaller amount of DMX channels (size of segments can be set via service mobile application or firmware)

AUTOMODE has a STATIC submode for showing static color which is writed in FS and CHASE submode. In CHASE submode, esp8266 plays packets from FS, which were stored there via RECORDING process.

  RECORDING process:

   - channel 510 - number of stored effect (should be 11-99)

   - channel 511 - stop recording (255 - stop)

   - channel 512 - start recording (250 - start recording without autodetection of effect loop, 251 - 255 start recording with autodetection) Autodetection of loop means that when 
  esp8266 detects, that while recording it receives packet, similar as was in 20th frame from the begining, it automatically stops recording.
  251 - delta for comparing packets is 1
  252 - delta is 2
  253 - delta is 3
  254 - delta is 4
  255 - delta is 5

  FIXTURE MODE (DMX):

  1 - Dimmer

  2 - Shutter

  3 - Red

  4 - Green

  5 - Blue

  6 - Effect

  7 - Speed

  When Effect is bigger than 0, Red Green Blue and Shutter are inactive
