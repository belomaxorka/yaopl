/*
 * OPL debug build (branch debug/ds3-metrics): DualShock 3/4 USB driver metrics.
 * Included once by the in-game driver (modules/pademu/ds34usb.c) and once by the menu's driver
 * (modules/ds34usb/iop/ds34usb.c). A thread prints a "DS3M" line per pad once a second when something changed,
 * every 10 seconds while a pad is connected, and every driver event (connect, release, configuration) with the
 * time it happened. Nothing is printed from the USB threads or the pad polling itself.
 */

#ifndef DS3METRICS_H
#define DS3METRICS_H

#include <types.h>
#include <stdio.h>
#include <thbase.h>
#include <intrman.h>

#ifndef DS3M_NAME
#define DS3M_NAME "?"
#endif

#define DS3M_PADS   4
#define DS3M_EVENTS 32

enum {
    DS3M_EV_CONNECT = 1, // a = devId, b = type
    DS3M_EV_ENDPOINTS,   // a = interrupt IN endpoint, b = OUT endpoint
    DS3M_EV_SETCONFIG,   // a = UsbSetDeviceConfiguration() result
    DS3M_EV_CONFIGURED,  // a = result passed to the callback, b = time spent in the callback (us)
    DS3M_EV_INIT,        // a = DS3 enable report queue result
    DS3M_EV_DISCONNECT,  // a = devId
    DS3M_EV_RELEASE,     // a = devId, b = status before
    DS3M_EV_NOSLOT,      // a = devId
};

typedef struct
{
    u32 polls;
    u32 repQ, repQErr, repDone, repBad, repTimeout, repEarly;
    u32 cmdQ, cmdQErr, cmdDone, cmdBad, cmdTimeout, cmdEarly;
    u32 maxRepWait, maxCmdWait, maxPoll; // us, reset after each line
    int lastQErr, lastBad;
} ds3m_pad_t;

typedef struct
{
    u32 time;
    u8 type, pad;
    short a;
    int b;
} ds3m_event_t;

static ds3m_pad_t ds3m[DS3M_PADS];
static ds3m_event_t ds3mEvents[DS3M_EVENTS];
static volatile u32 ds3mEvWrite, ds3mEvRead, ds3mEvLost;
static int (*ds3mGetStatus)(int pad);
static u32 ds3mStartTime;

static inline u32 ds3mNow(void)
{
    iop_sys_clock_t clock;

    GetSystemTime(&clock);
    return clock.lo;
}

// Bus clock ticks (36.864 MHz) to microseconds without overflowing 32 bits.
static inline u32 ds3mUs(u32 ticks)
{
    return (ticks / 36864) * 1000 + ((ticks % 36864) * 1000) / 36864;
}

static void ds3mEvent(int type, int pad, int a, int b)
{
    int state;
    ds3m_event_t *ev;

    CpuSuspendIntr(&state);
    if (ds3mEvWrite - ds3mEvRead < DS3M_EVENTS) {
        ev = &ds3mEvents[ds3mEvWrite % DS3M_EVENTS];
        ev->time = ds3mNow();
        ev->type = type;
        ev->pad = pad;
        ev->a = a;
        ev->b = b;
        ds3mEvWrite++;
    } else
        ds3mEvLost++;
    CpuResumeIntr(state);
}

// Call after a TransferWait(): done0 is the completion counter read before the transfer was queued.
static inline void ds3mWaited(int pad, int cmd, u32 t0, u32 done0)
{
    ds3m_pad_t *m = &ds3m[pad];
    u32 us = ds3mUs(ds3mNow() - t0);

    if (cmd) {
        if (m->cmdDone == done0) {
            if (us >= 5000)
                m->cmdTimeout++;
            else
                m->cmdEarly++;
        }
        if (us > m->maxCmdWait)
            m->maxCmdWait = us;
    } else {
        if (m->repDone == done0) {
            if (us >= 5000)
                m->repTimeout++;
            else
                m->repEarly++;
        }
        if (us > m->maxRepWait)
            m->maxRepWait = us;
    }
}

static inline void ds3mPolled(int pad, u32 t0)
{
    u32 us = ds3mUs(ds3mNow() - t0);

    ds3m[pad].polls++;
    if (us > ds3m[pad].maxPoll)
        ds3m[pad].maxPoll = us;
}

static const char *ds3mEventName(int type)
{
    switch (type) {
        case DS3M_EV_CONNECT:
            return "connect dev/type";
        case DS3M_EV_ENDPOINTS:
            return "endpoints in/out";
        case DS3M_EV_SETCONFIG:
            return "set-config rc";
        case DS3M_EV_CONFIGURED:
            return "configured rc/us";
        case DS3M_EV_INIT:
            return "ds3-enable rc";
        case DS3M_EV_DISCONNECT:
            return "disconnect dev";
        case DS3M_EV_RELEASE:
            return "release dev/status";
        case DS3M_EV_NOSLOT:
            return "no-slot dev";
        default:
            return "?";
    }
}

static void ds3mThread(void *arg)
{
    static ds3m_pad_t prev[DS3M_PADS];
    ds3m_pad_t cur;
    ds3m_event_t ev;
    u32 sec = 0, quiet[DS3M_PADS] = {0};
    int pad, state, status, have;

    (void)arg;

    printf("DS3M %s start\n", DS3M_NAME);

    while (1) {
        DelayThread(1000 * 1000);
        sec++;

        while (1) {
            CpuSuspendIntr(&state);
            have = ds3mEvRead != ds3mEvWrite;
            if (have) {
                ev = ds3mEvents[ds3mEvRead % DS3M_EVENTS];
                ds3mEvRead++;
            }
            CpuResumeIntr(state);
            if (!have)
                break;
            printf("DS3M %s %u ev t=%ums p%u %s %d %d\n", DS3M_NAME, (unsigned int)sec,
                   (unsigned int)(ds3mUs(ev.time - ds3mStartTime) / 1000), ev.pad, ds3mEventName(ev.type), ev.a, ev.b);
        }
        if (ds3mEvLost) {
            printf("DS3M %s %u lost %u events\n", DS3M_NAME, (unsigned int)sec, (unsigned int)ds3mEvLost);
            ds3mEvLost = 0;
        }

        for (pad = 0; pad < DS3M_PADS; pad++) {
            CpuSuspendIntr(&state);
            cur = ds3m[pad];
            ds3m[pad].maxRepWait = 0;
            ds3m[pad].maxCmdWait = 0;
            ds3m[pad].maxPoll = 0;
            CpuResumeIntr(state);

            status = ds3mGetStatus ? ds3mGetStatus(pad) : 0;

            if (cur.polls == prev[pad].polls && cur.repDone == prev[pad].repDone && cur.cmdDone == prev[pad].cmdDone &&
                cur.repQ == prev[pad].repQ && cur.cmdQ == prev[pad].cmdQ) {
                if (status == 0 || ++quiet[pad] < 10)
                    continue;
            }
            quiet[pad] = 0;

            printf("DS3M %s %u p%d st=%x poll=+%u rep q=+%u done=+%u out=%d to=+%u early=+%u bad=+%u qerr=+%u w=%u"
                   " cmd q=+%u done=+%u out=%d to=+%u early=+%u bad=+%u qerr=+%u w=%u maxpoll=%u lastbad=%d lastqerr=%d\n",
                   DS3M_NAME, (unsigned int)sec, pad, status, (unsigned int)(cur.polls - prev[pad].polls),
                   (unsigned int)(cur.repQ - prev[pad].repQ), (unsigned int)(cur.repDone - prev[pad].repDone),
                   (int)(cur.repQ - cur.repDone), (unsigned int)(cur.repTimeout - prev[pad].repTimeout),
                   (unsigned int)(cur.repEarly - prev[pad].repEarly), (unsigned int)(cur.repBad - prev[pad].repBad),
                   (unsigned int)(cur.repQErr - prev[pad].repQErr), (unsigned int)cur.maxRepWait,
                   (unsigned int)(cur.cmdQ - prev[pad].cmdQ), (unsigned int)(cur.cmdDone - prev[pad].cmdDone),
                   (int)(cur.cmdQ - cur.cmdDone), (unsigned int)(cur.cmdTimeout - prev[pad].cmdTimeout),
                   (unsigned int)(cur.cmdEarly - prev[pad].cmdEarly), (unsigned int)(cur.cmdBad - prev[pad].cmdBad),
                   (unsigned int)(cur.cmdQErr - prev[pad].cmdQErr), (unsigned int)cur.maxCmdWait,
                   (unsigned int)cur.maxPoll, cur.lastBad, cur.lastQErr);
            prev[pad] = cur;
        }
    }
}

static void ds3mStart(int (*getStatus)(int pad))
{
    iop_thread_t thread;
    int thid;

    ds3mGetStatus = getStatus;
    ds3mStartTime = ds3mNow();

    thread.attr = TH_C;
    thread.option = 0;
    thread.thread = ds3mThread;
    thread.stacksize = 0x800;
    thread.priority = 1;

    thid = CreateThread(&thread);
    if (thid > 0)
        StartThread(thid, NULL);
}

#endif
