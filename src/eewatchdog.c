/*
 * OPL debug build (branch debug/ds3-metrics): EE watchdog, see include/eewatchdog.h.
 * Every second it checks whether the menu drew a frame. While it does, a line every 10 seconds says so. When no frame
 * was drawn for 3 seconds, it sends the breadcrumbs and every EE thread's state (and the semaphores they wait on),
 * then again every 5 seconds while the menu stays stuck. It uses its own RPC client, not libds34usb's.
 */

#include "include/opl.h"
#include <kernel.h>
#include <sifrpc.h>
#include <delaythread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "include/eewatchdog.h"

#define WD_RPC_ID  0x18E3878E // ds34usb.irx
#define WD_RPC_LOG 9

volatile unsigned int wdFrames;
volatile int wdGui, wdPad, wdIo, wdIoType;
volatile unsigned int wdIoCount;
const char *volatile wdNote;

static SifRpcClientData_t wdClient;
static char wdBuf[256] __attribute__((aligned(64)));
static u8 wdStack[0x2000] __attribute__((aligned(16)));
static int wdGuiThid;

extern void *_gp;

static void wdSend(const char *fmt, ...)
{
    va_list ap;
    int len;

    if (wdClient.server == NULL)
        return;

    va_start(ap, fmt);
    len = vsnprintf(wdBuf, 200, fmt, ap);
    va_end(ap);
    if (len < 0)
        return;
    if (len > 199)
        len = 199;
    wdBuf[len] = '\n';
    wdBuf[len + 1] = '\0';

    SifCallRpc(&wdClient, WD_RPC_LOG, 0, wdBuf, (len + 2 + 15) & ~15, NULL, 0, NULL, NULL);
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

        if (wdClient.server == NULL) {
            if (SifBindRpc(&wdClient, WD_RPC_ID, 0) < 0 || wdClient.server == NULL)
                continue;
            wdSend("EEWD %d start gui thread %d", sec, wdGuiThid);
        }

        if (wdFrames == lastFrames)
            stuck++;
        else
            stuck = 0;

        if (stuck == 0 && sec % 10 == 0)
            wdSend("EEWD %d alive frames/s=%u gui=%d pad=%d io=%d/%d io+=%u note=%s", sec, wdFrames - lastFrames, wdGui, wdPad,
                   wdIo, wdIoType, wdIoCount - lastIo, wdNote ? wdNote : "-");
        if (stuck == 3 || (stuck > 3 && stuck % 5 == 0)) {
            wdSend("EEWD %d HANG no frame for %ds gui=%d pad=%d io=%d/%d io+=%u note=%s", sec, stuck, wdGui, wdPad, wdIo,
                   wdIoType, wdIoCount - lastIo, wdNote ? wdNote : "-");
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
