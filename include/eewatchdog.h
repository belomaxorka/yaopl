/*
 * OPL debug build (branch debug/ds3-metrics): EE watchdog.
 * The menu's loops and the IO thread leave breadcrumbs here; a watchdog thread sends them, and the state of every EE
 * thread when the menu stops drawing frames, to the IOP's ds34usb module, which prints them ("EEWD" lines) to the
 * network log.
 */

#ifndef EEWATCHDOG_H
#define EEWATCHDOG_H

extern volatile unsigned int wdFrames;
extern volatile int wdGui, wdPad, wdIo, wdIoType;
extern volatile unsigned int wdIoCount, wdStress;
extern const char *volatile wdNote;

// Where OPL's settings came from (_loadConfig) and the plasma background colours (guiDrawBGPlasma).
extern volatile unsigned int wdCfgCount, wdBgTarget, wdBgCur, wdPlasma;
extern volatile int wdCfgResult;
extern char wdCfgFile[64];

void wdStart(void);

#endif
