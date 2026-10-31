/*
 * OPL debug build (branch debug/ds3-metrics): EE watchdog, see include/eewatchdog.h.
 * Every second it checks whether the menu drew a frame. While it does, a line every 10 seconds says so. When no frame
 * was drawn for 3 seconds, it sends the breadcrumbs, the EE DMA and SIF registers and every EE thread's state (and
 * the semaphores they wait on), then again every 5 seconds while the menu stays stuck.
 * Lines go to the IOP as raw SIF commands (user command 0, EE -> IOP over SIF1), which need no reply: a menu hang
 * stalls the IOP -> EE direction (SIF0), which RPC replies use. ds34usb.irx prints them.
 */

#include "include/opl.h"
#include <kernel.h>
#include <sifcmd.h>
#include <delaythread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "include/eewatchdog.h"

#define WD_SIF_CMD   0 // IOP user SIF command, registered by ds34usb.irx
#define WD_TEXT_SIZE 96

volatile unsigned int wdFrames;
volatile int wdGui, wdPad, wdIo, wdIoType;
volatile unsigned int wdIoCount;
const char *volatile wdNote;

typedef struct
{
    SifCmdHeader_t hdr;
    char text[WD_TEXT_SIZE];
} wd_packet_t;

static wd_packet_t wdPacket __attribute__((aligned(64)));
static u8 wdStack[0x2000] __attribute__((aligned(16)));
static int wdGuiThid;

extern void *_gp;

#define WD_REG(addr) (*(volatile u32 *)(addr))

static void wdSend(const char *fmt, ...)
{
    va_list ap;
    unsigned int id;
    int i;

    va_start(ap, fmt);
    vsnprintf(wdPacket.text, WD_TEXT_SIZE, fmt, ap);
    va_end(ap);

    id = SifSendCmd(WD_SIF_CMD, &wdPacket, sizeof(wdPacket), NULL, NULL, 0);
    if (id == 0)
        return;

    // The packet is sent from wdPacket: wait (at most ~20 ms) until the DMA has read it before it is reused.
    for (i = 0; i < 20 && sceSifDmaStat((int)id) >= 0; i++)
        DelayThread(1000);
}

static void wdDumpRegs(int sec)
{
    wdSend("EEWD %d d5 sif0 chcr=%x madr=%x qwc=%x tadr=%x", sec, WD_REG(0x1000C000), WD_REG(0x1000C010),
           WD_REG(0x1000C020), WD_REG(0x1000C030));
    wdSend("EEWD %d d6 sif1 chcr=%x madr=%x qwc=%x tadr=%x", sec, WD_REG(0x1000C400), WD_REG(0x1000C410),
           WD_REG(0x1000C420), WD_REG(0x1000C430));
    wdSend("EEWD %d d2 gif chcr=%x madr=%x qwc=%x tadr=%x", sec, WD_REG(0x1000A000), WD_REG(0x1000A010),
           WD_REG(0x1000A020), WD_REG(0x1000A030));
    wdSend("EEWD %d dctrl=%x dstat=%x dpcr=%x sqwc=%x rbsr=%x rbor=%x", sec, WD_REG(0x1000E000), WD_REG(0x1000E010),
           WD_REG(0x1000E020), WD_REG(0x1000E030), WD_REG(0x1000E040), WD_REG(0x1000E050));
    wdSend("EEWD %d enabler=%x gifstat=%x intc stat=%x mask=%x", sec, WD_REG(0x1000F520), WD_REG(0x10003020),
           WD_REG(0x1000F000), WD_REG(0x1000F010));
    wdSend("EEWD %d sif mscom=%x smcom=%x msflg=%x smflg=%x ctrl=%x", sec, WD_REG(0x1000F200), WD_REG(0x1000F210),
           WD_REG(0x1000F220), WD_REG(0x1000F230), WD_REG(0x1000F240));
}

static void wdDumpThreads(int sec)
{
    ee_thread_status_t st;
    ee_sema_t sema;
    int tid;

    for (tid = 1; tid < 64; tid++) {
        if (ReferThreadStatus(tid, &st) < 0)
            continue;
        wdSend("EEWD %d thr %d%s st=%x pri=%d wait=%u id=%u func=%p", sec, tid, (tid == wdGuiThid) ? "(gui)" : "",
               st.status, st.current_priority, (unsigned int)st.waitType, (unsigned int)st.waitId, st.func);
        if (st.waitType == 2 && ReferSemaStatus(st.waitId, &sema) >= 0)
            wdSend("EEWD %d   sema %u count=%d max=%d waiting=%d", sec, (unsigned int)st.waitId, sema.count,
                   sema.max_count, sema.wait_threads);
    }
}

static void wdThread(void *arg)
{
    unsigned int lastFrames = 0, lastIo = 0;
    int sec = 0, stuck = 0;

    (void)arg;

    while (1) {
        DelayThread(1000 * 1000);
        sec++;

        if (sec == 1)
            wdSend("EEWD %d start gui thread %d", sec, wdGuiThid);

        if (wdFrames == lastFrames)
            stuck++;
        else
            stuck = 0;

        if (stuck == 0 && sec % 10 == 0)
            wdSend("EEWD %d alive frames/s=%u gui=%d pad=%d io=%d/%d io+=%u note=%s", sec, wdFrames - lastFrames, wdGui,
                   wdPad, wdIo, wdIoType, wdIoCount - lastIo, wdNote ? wdNote : "-");
        if (stuck == 3 || (stuck > 3 && stuck % 5 == 0)) {
            wdSend("EEWD %d HANG no frame for %ds gui=%d pad=%d io=%d/%d note=%s", sec, stuck, wdGui, wdPad, wdIo,
                   wdIoType, wdNote ? wdNote : "-");
            wdDumpRegs(sec);
            wdDumpThreads(sec);
        }

        lastFrames = wdFrames;
        lastIo = wdIoCount;
    }
}

void wdStart(void)
{
    ee_thread_t thread;
    int tid;

    wdGuiThid = GetThreadId();

    memset(&thread, 0, sizeof(thread));
    thread.func = &wdThread;
    thread.stack = wdStack;
    thread.stack_size = sizeof(wdStack);
    thread.gp_reg = &_gp;
    thread.initial_priority = 10;

    tid = CreateThread(&thread);
    if (tid >= 0)
        StartThread(tid, NULL);
}
