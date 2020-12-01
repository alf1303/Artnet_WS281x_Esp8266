#pragma once
#define SPEED_MIN_INT 1
#define SPEED_MAX_INT 100
#define SPEED_MIN_DOUBLE 0.1
#define SPEED_MAX_DOUBLE 10.0
#define WIDTH_MIN_INT 1
#define WIDTH_MAX_INT 100
#define WIDTH_MIN_DOUBLE 0.8
#define WIDTH_MAX_DOUBLE 3.0
#define PARTS_MIN_INT 1
#define PARTS_MAX_INT 100
#define PARTS_MIN_DOUBLE 0.1
#define PARTS_MAX_DOUBLE 5

#define PIP PI

typedef struct {
    uint8_t mode; // WIFI or LAN or AUTO mode variable (0 - WIFI, 1 - LAN, 2 - AUTO, 3 - FIXTURE MODE)
    uint8_t autoMode; // mode for Automatic strip control
    double fxSpeed; //speed for playing effects from FS
    RgbColor color; //color for static automode
    uint8_t fxNumber; //number of internal chase
    uint8_t dimmer; //intensity
    //uint8_t recordedEffNum; //number of recorded effect
    uint8_t universe;
    uint16_t address;
    uint8_t reverse;
    uint8_t pixelCount;
    uint8_t startPixel;
    uint8_t endPixel;
    uint8_t segment;
    ///////////////////////////////
    char* name;
    uint8_t playlistSize;
    bool playlistMode;
    RgbColor fxColor;
    uint8_t strobe;
    uint8_t fxSize;
    uint8_t fxParts;
    uint8_t fxFade;
    uint8_t fxParams;
    uint8_t fxSpread;
    uint8_t fxWidth;
    boolean fxReverse;
    boolean fxAttack;
    boolean fxSymm;
    boolean fxRnd;
    boolean fxRndColor;
} settings_t;

//playlist item setting type
typedef struct {
    uint8_t dimmer;
    RgbColor color;
    RgbColor fxColor;
    uint8_t strobe;
    uint8_t fxNumber;
    double fxSpeed;
    uint8_t fxSize;
    uint8_t fxParts;
    uint8_t fxFade;
    uint8_t fxParams;
    uint8_t fxSpread;
    uint8_t fxWidth;
    boolean fxReverse;
    boolean fxAttack;
    boolean fxSymm;
    boolean fxRnd;
    boolean fxRndColor;
} ledsettings_t;

struct RgbTemp_t{
    double R;
    double G;
    double B;

    RgbTemp_t(double r, double g, double b):
        R(r), G(g), B(b) {};

    RgbTemp_t() {};
};