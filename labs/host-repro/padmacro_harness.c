/* Host harness for modules/pademu/padmacro.c: padMacroInit() and padMacroPerform()
 * on one DS2 report, as PadEmu builds it from a DS3/DS4. Prints the buttons the
 * game sees as pressed, the non-zero pressure bytes and the left stick X.
 * Modes: padmacro-t <scenario>; a scenario sets the PADMACRO word (the button
 * remap in bits 18-19) and the buttons held on the controller. */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef short s16;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef int bool;
#define true 1
#define false 0
#define __attribute__(x)

enum DS2ButtonBitNumber {
    DS2BtnBit_Select = 0,
    DS2BtnBit_L3 = 1,
    DS2BtnBit_R3 = 2,
    DS2BtnBit_Start = 3,
    DS2BtnBit_Up = 4,
    DS2BtnBit_Right = 5,
    DS2BtnBit_Down = 6,
    DS2BtnBit_Left = 7,
    DS2BtnBit_L2 = 8,
    DS2BtnBit_R2 = 9,
    DS2BtnBit_L1 = 10,
    DS2BtnBit_R1 = 11,
    DS2BtnBit_Triangle = 12,
    DS2BtnBit_Circle = 13,
    DS2BtnBit_Cross = 14,
    DS2BtnBit_Square = 15,
};
enum DS2Buttons {
    DS2ButtonUp = (1 << 4),
    DS2ButtonRight = (1 << 5),
    DS2ButtonDown = (1 << 6),
    DS2ButtonLeft = (1 << 7),
    DS2ButtonStart = (1 << 3),
};

#include "padmacro_ds2report.c"

enum PadMacroAxes {
    PadMacroAxisLX = 0,
    PadMacroAxisLY = 1,
    PadMacroAxisRX = 2,
    PadMacroAxisRY = 3,
};
enum PadMacroRemaps {
    PadMacroRemapOff = 0,
    PadMacroRemapTriggersFace = 1,
    PadMacroRemapTriggersShoulders = 2,
};

/* the types and statics of padmacro.c, including the ones a fix may add */
typedef union
{
    struct
    {
        u32 left_stick_slowdown  : 4;
        u32 right_stick_slowdown : 4;
        u32 l_slowdown_enable    : 1;
        u32 l_slowdown_toggle    : 1;
        u32 r_slowdown_enable    : 1;
        u32 r_slowdown_toggle    : 1;
        u32 lx_invert            : 1;
        u32 ly_invert            : 1;
        u32 rx_invert            : 1;
        u32 ry_invert            : 1;
        u32 turbo_speed          : 2;
        u32 button_remap         : 2;
    };
    u32 raw;
} PadMacroSettings_t;

static const u8 button_to_analog_lookup[12] = {
    offsetof(struct ds2report, PressureUp),
    offsetof(struct ds2report, PressureRight),
    offsetof(struct ds2report, PressureDown),
    offsetof(struct ds2report, PressureLeft),
    offsetof(struct ds2report, PressureL2),
    offsetof(struct ds2report, PressureR2),
    offsetof(struct ds2report, PressureL1),
    offsetof(struct ds2report, PressureR1),
    offsetof(struct ds2report, PressureTriangle),
    offsetof(struct ds2report, PressureCircle),
    offsetof(struct ds2report, PressureCross),
    offsetof(struct ds2report, PressureSquare),
};

static struct
{
    u16 buttons;
    bool special_button;
} gPreviousButtons = {0};
static u16 gLeftStickSlowdownMask;
enum StickSlowdownToggle {
    SLOWDOWN_PRESS_MODE = 0,
    SLOWDOWN_TOGGLE_MODE = 2,
    SLOWDOWN_TOGGLE_ON = 3,
};
static u8 gLeftStickSlowdownToggle;
static u16 gRightStickSlowdownMask;
static u8 gRightStickSlowdownToggle;
static u8 gStickInvertBitfield;
static bool gTurboButtonWaiting = false;
static u16 gTurboButtonMask = 0;
static u8 gTurboSpeed = 0;
static u8 gTurboButtonCounter = 0;
static u8 gButtonRemap = PadMacroRemapOff;

#include "padmacro_extracted.c"

static const char *names[16] = {"Select", "L3", "R3", "Start", "Up", "Right", "Down", "Left",
                                "L2", "R2", "L1", "R1", "Triangle", "Circle", "Cross", "Square"};

static void press(struct ds2report *rep, int bit, u8 pressure)
{
    rep->nButtonState &= ~(1 << bit);
    if (bit >= DS2BtnBit_Up)
        ((u8 *)rep)[button_to_analog_lookup[bit - DS2BtnBit_Up]] = pressure;
}

int main(int argc, char **argv)
{
    const char *s = argc > 1 ? argv[1] : "";
    PadMacroSettings_t settings;
    struct ds2report rep;
    int i;

    settings.raw = 0;
    memset(&rep, 0, sizeof(rep));
    rep.nButtonState = 0xFFFF;
    rep.LeftStickX = rep.LeftStickY = rep.RightStickX = rep.RightStickY = 128;

    if (!strcmp(s, "off")) { /* no remap: gas on R2 and Cross held */
        press(&rep, DS2BtnBit_R2, 200);
        press(&rep, DS2BtnBit_Cross, 255);
    } else if (!strcmp(s, "face-triggers")) { /* L2/R2 <-> Square/Cross; R2 half, L2 light */
        settings.raw = 1 << 18;
        press(&rep, DS2BtnBit_R2, 200);
        press(&rep, DS2BtnBit_L2, 40);
    } else if (!strcmp(s, "face-cross")) { /* L2/R2 <-> Square/Cross; Cross and Circle held */
        settings.raw = 1 << 18;
        press(&rep, DS2BtnBit_Cross, 255);
        press(&rep, DS2BtnBit_Circle, 255);
    } else if (!strcmp(s, "shoulders")) { /* L2/R2 <-> L1/R1; R2 and L1 held */
        settings.raw = 2 << 18;
        press(&rep, DS2BtnBit_R2, 180);
        press(&rep, DS2BtnBit_L1, 255);
    } else if (!strcmp(s, "reserved")) { /* value 3 is not a remap */
        settings.raw = 3u << 18;
        press(&rep, DS2BtnBit_R2, 200);
    } else if (!strcmp(s, "igr-face") || !strcmp(s, "igr-shoulders")) { /* the IGR combo L1+L2+R1+R2+Start+Select */
        settings.raw = (s[4] == 'f' ? 1 : 2) << 18;
        press(&rep, DS2BtnBit_L1, 255);
        press(&rep, DS2BtnBit_L2, 255);
        press(&rep, DS2BtnBit_R1, 255);
        press(&rep, DS2BtnBit_R2, 255);
        press(&rep, DS2BtnBit_Start, 0);
        press(&rep, DS2BtnBit_Select, 0);
    } else if (!strcmp(s, "slowdown")) { /* left slow-down on R2 and remap 1: R2 slows the stick */
        settings.raw = (1 << 18) | (1 << 8) | DS2BtnBit_R2;
        press(&rep, DS2BtnBit_R2, 255);
        rep.LeftStickX = 255;
    } else {
        printf("unknown scenario\n");
        return 2;
    }

    padMacroInit(settings.raw);
    padMacroPerform(&rep, false);

    printf("pressed:");
    for (i = 0; i < 16; i++)
        if (!(rep.nButtonState & (1 << i)))
            printf(" %s", names[i]);
    printf(" pressure:");
    for (i = DS2BtnBit_Up; i < 16; i++) {
        u8 p = ((u8 *)&rep)[button_to_analog_lookup[i - DS2BtnBit_Up]];
        if (p)
            printf(" %s=%d", names[i], p);
    }
    printf(" LX=%d\n", rep.LeftStickX);
    return 0;
}
