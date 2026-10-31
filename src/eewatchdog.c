/*
 * OPL debug build (branch debug/ds3-metrics): EE watchdog, see include/eewatchdog.h.
 * Every second it checks whether the menu drew a frame (guiEndFrame, which every loop calls, dialogs too). While it
 * does, a line every 10 seconds says so. When no frame was drawn for 3 seconds, it sends the breadcrumbs, the EE DMA
 * and SIF registers and every EE thread's state (and the semaphores they wait on), then again every 5 seconds while
 * the menu stays stuck.
 * Lines go to the IOP as raw SIF commands (user command 0, EE -> IOP over SIF1), which need no reply: a menu hang
 * stalls the IOP -> EE direction (SIF0), which RPC replies use. ds34usb.irx prints them.
 *
 * A VBlank interrupt handler watches too ("EEIW" lines): in a hang caught with build 5 the watchdog thread fell silent
 * as well, so something keeps every thread from running. While no frame is drawn it samples the interrupted PC (COP0
 * EPC) every VBlank and reports the samples and the running and ready threads; a line every ~30 s shows it is alive.
 * No EEIW line during a hang means the EE takes no interrupts at all (a crash or a loop with interrupts disabled).
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
volatile unsigned int wdIoCount, wdStress;
const char *volatile wdNote;

typedef struct
{
    SifCmdHeader_t hdr;
    char text[WD_TEXT_SIZE];
} wd_packet_t;

static wd_packet_t wdPacket __attribute__((aligned(64)));

// Interrupt watchdog: its own packets, each read by the SIF DMA long before the ring comes back to it.
#define WDI_PACKETS 16
#define WDI_SAMPLES 8
typedef union
{
    wd_packet_t pkt;
    u8 pad[128];
} wdi_slot_t;
static wdi_slot_t wdiSlots[WDI_PACKETS] __attribute__((aligned(64)));
static unsigned int wdiSlot, wdiTicks, wdiLastFrames, wdiStuck, wdiEpc[WDI_SAMPLES];
static char *wdiPos, *wdiEnd;
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

// Line builders for interrupt context: no vsnprintf there.
static void wdiBegin(const char *tag)
{
    wd_packet_t *pkt = &wdiSlots[wdiSlot % WDI_PACKETS].pkt;

    wdiPos = pkt->text;
    wdiEnd = pkt->text + WD_TEXT_SIZE - 1;
    while (*tag != '\0' && wdiPos < wdiEnd)
        *wdiPos++ = *tag++;
}

static void wdiStr(const char *str)
{
    while (*str != '\0' && wdiPos < wdiEnd)
        *wdiPos++ = *str++;
}

static void wdiHex(unsigned int value)
{
    int shift;

    for (shift = 28; shift >= 0 && wdiPos < wdiEnd; shift -= 4)
        *wdiPos++ = "0123456789abcdef"[(value >> shift) & 0xF];
}

static void wdiDec(unsigned int value)
{
    char digits[10];
    int n = 0;

    do {
        digits[n++] = '0' + value % 10;
        value /= 10;
    } while (value != 0);
    while (n > 0 && wdiPos < wdiEnd)
        *wdiPos++ = digits[--n];
}

static void wdiSend(void)
{
    wd_packet_t *pkt = &wdiSlots[wdiSlot % WDI_PACKETS].pkt;

    *wdiPos = '\0';
    if (isceSifSendCmd(WD_SIF_CMD, pkt, sizeof(*pkt), NULL, NULL, 0) != 0)
        wdiSlot++;
}

static void wdiDump(void)
{
    ee_thread_status_t st;
    int tid, i, lines = 0;

    wdiBegin("EEIW ");
    wdiDec(wdiTicks);
    wdiStr(" HANG vblanks=");
    wdiDec(wdiStuck);
    wdiStr(" gui=");
    wdiDec(wdGui);
    wdiStr(" pad=");
    wdiDec(wdPad);
    wdiStr(" io=");
    wdiDec(wdIo);
    wdiStr(" stress=");
    wdiDec(wdStress);
    wdiSend();

    wdiBegin("EEIW ");
    wdiDec(wdiTicks);
    wdiStr(" epc");
    for (i = 0; i < WDI_SAMPLES; i++) {
        wdiStr(" ");
        wdiHex(wdiEpc[i]);
    }
    wdiSend();

    // The running thread is the one the VBlank interrupted; ready ones are waiting for the CPU.
    for (tid = 1; tid < 256 && lines < 10; tid++) {
        if (iReferThreadStatus(tid, &st) < 0 || (st.status != 1 && st.status != 2 && tid != wdGuiThid))
            continue;
        wdiBegin("EEIW ");
        wdiDec(wdiTicks);
        wdiStr(" thr ");
        wdiDec(tid);
        wdiStr(tid == wdGuiThid ? "(gui) st=" : " st=");
        wdiHex(st.status);
        wdiStr(" pri=");
        wdiDec(st.current_priority);
        wdiStr(" wait=");
        wdiDec(st.waitType);
        wdiStr(" id=");
        wdiDec(st.waitId);
        wdiStr(" func=");
        wdiHex((unsigned int)st.func);
        wdiSend();
        lines++;
    }
}

static s32 wdiVblank(s32 cause)
{
    unsigned int epc, frames = wdFrames;

    (void)cause;

    __asm__ __volatile__("mfc0 %0, $14"
                         : "=r"(epc));
    wdiTicks++;
    wdiEpc[wdiTicks % WDI_SAMPLES] = epc;

    if (frames != wdiLastFrames) {
        wdiLastFrames = frames;
        wdiStuck = 0;
    } else
        wdiStuck++;

    // ~9 s at 50 Hz, longer than a theme reload; then every ~5 s.
    if (wdiStuck == 450 || (wdiStuck > 450 && wdiStuck % 250 == 0))
        wdiDump();
    else if (wdiTicks % 1500 == 0) {
        wdiBegin("EEIW ");
        wdiDec(wdiTicks);
        wdiStr(" alive frames=");
        wdiDec(frames);
        wdiStr(" epc=");
        wdiHex(epc);
        wdiSend();
    }

    ExitHandler();
    return 0;
}

static void wdDumpThreads(int sec)
{
    ee_thread_status_t st;
    ee_sema_t sema;
    int tid;

    // Thread IDs keep growing across launches: check every slot of the kernel's table, skip the free ones.
    for (tid = 1; tid < 256; tid++) {
        memset(&st, 0, sizeof(st));
        if (ReferThreadStatus(tid, &st) < 0 || st.status == 0)
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
            wdSend("EEWD %d alive frames/s=%u gui=%d pad=%d io=%d/%d io+=%u stress=%u note=%s", sec,
                   wdFrames - lastFrames, wdGui, wdPad, wdIo, wdIoType, wdIoCount - lastIo, wdStress,
                   wdNote ? wdNote : "-");
        if (stuck == 3 || (stuck > 3 && stuck % 5 == 0)) {
            wdSend("EEWD %d HANG no frame for %ds gui=%d pad=%d io=%d/%d stress=%u note=%s", sec, stuck, wdGui, wdPad,
                   wdIo, wdIoType, wdStress, wdNote ? wdNote : "-");
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

    DIntr();
    AddIntcHandler(INTC_VBLANK_S, &wdiVblank, 0);
    EnableIntc(INTC_VBLANK_S);
    EIntr();
}
