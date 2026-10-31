/*
 * OPL debug build (branch debug/ds3-metrics): DualShock 3/4 USB driver metrics.
 * Included once by the in-game driver (modules/pademu/ds34usb.c) and once by the menu's driver
 * (modules/ds34usb/iop/ds34usb.c). A thread prints a "DS3M" line per pad once a second when something changed or a
 * call is stuck, every 10 seconds while a pad is connected, and every driver event (connect, release,
 * configuration) with the time it happened. The events are printed again at 15 and 30 seconds, as the network
 * that carries the log may come up after them. Nothing is printed from the USB threads or the polling itself.
 */

#ifndef DS3METRICS_H
#define DS3METRICS_H

#include <types.h>
#include <stdio.h>
#include <thbase.h>
#include <thsemap.h>
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
    u32 polls, pollIn, pollInTime;
    u32 repQ, repQErr, repDone, repBad, repTimeout, repEarly;
    u32 cmdQ, cmdQErr, cmdDone, cmdBad, cmdTimeout, cmdEarly;
    u32 maxRepWait, maxCmdWait, maxPoll; // us, reset after each line
    u32 topRepWait, topCmdWait, topPoll; // us, since the module started
    int topRepOut, topCmdOut;            // most transfers queued without a completion, since start
    int lastQErr, lastBad;
    int mutex, cmdSema;
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
static volatile int ds3mPollThid;                       // thread of the latest poll
static volatile u32 ds3mRpcIn, ds3mRpcOut, ds3mRpcTime; // menu: RPC calls entered and left
static volatile int ds3mRpcCmd, ds3mRpcThid;

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

static inline void ds3mSemas(int pad, int mutex, int cmdSema)
{
    ds3m[pad].mutex = mutex;
    ds3m[pad].cmdSema = cmdSema;
}

// Call after a TransferWait(): done0 is the completion counter read before the transfer was queued.
static inline void ds3mWaited(int pad, int cmd, u32 t0, u32 done0)
{
    ds3m_pad_t *m = &ds3m[pad];
    u32 us = ds3mUs(ds3mNow() - t0);
    int out;

    if (cmd) {
        if (m->cmdDone == done0) {
            if (us >= 5000)
                m->cmdTimeout++;
            else
                m->cmdEarly++;
        }
        if (us > m->maxCmdWait)
            m->maxCmdWait = us;
        if (us > m->topCmdWait)
            m->topCmdWait = us;
        out = (int)(m->cmdQ - m->cmdDone);
        if (out > m->topCmdOut)
            m->topCmdOut = out;
    } else {
        if (m->repDone == done0) {
            if (us >= 5000)
                m->repTimeout++;
            else
                m->repEarly++;
        }
        if (us > m->maxRepWait)
            m->maxRepWait = us;
        if (us > m->topRepWait)
            m->topRepWait = us;
        out = (int)(m->repQ - m->repDone);
        if (out > m->topRepOut)
            m->topRepOut = out;
    }
}

static inline u32 ds3mPollEnter(int pad)
{
    u32 now = ds3mNow();

    ds3m[pad].pollInTime = now;
    ds3m[pad].pollIn++;
    ds3mPollThid = GetThreadId();
    return now;
}

static inline void ds3mPolled(int pad, u32 t0)
{
    u32 us = ds3mUs(ds3mNow() - t0);

    ds3m[pad].polls++;
    if (us > ds3m[pad].maxPoll)
        ds3m[pad].maxPoll = us;
    if (us > ds3m[pad].topPoll)
        ds3m[pad].topPoll = us;
}

static inline void ds3mRpcEnter(int cmd)
{
    ds3mRpcTime = ds3mNow();
    ds3mRpcCmd = cmd;
    ds3mRpcThid = GetThreadId();
    ds3mRpcIn++;
}

static inline void ds3mRpcLeave(void)
{
    ds3mRpcOut++;
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

static void ds3mPrintEvent(const char *what, u32 sec, const ds3m_event_t *ev)
{
    printf("DS3M %s %u %s t=%ums p%u %s %d %d\n", DS3M_NAME, (unsigned int)sec, what,
           (unsigned int)(ds3mUs(ev->time - ds3mStartTime) / 1000), ev->pad, ds3mEventName(ev->type), ev->a, ev->b);
}

// "thr" state: status (1 run, 2 ready, 4 wait, 8 suspend), wait type (1 sleep, 2 delay, 3 sema, 4 event flag) and
// the id waited for; "sema" state: count and waiting threads.
static void ds3mPrintThread(const char *what, u32 sec, int thid)
{
    iop_thread_info_t info;

    if (thid <= 0 || ReferThreadStatus(thid, &info) != 0)
        return;
    printf("DS3M %s %u %s thr=%x st=%d wait=%d id=%x pri=%d\n", DS3M_NAME, (unsigned int)sec, what, thid, info.status,
           info.waitType, info.waitId, info.currentPriority);
}

static void ds3mPrintSema(u32 sec, int pad, const char *what, int sema)
{
    iop_sema_info_t info;

    if (sema < 0 || ReferSemaStatus(sema, &info) != 0)
        return;
    printf("DS3M %s %u p%d %s=%x count=%d waiting=%d\n", DS3M_NAME, (unsigned int)sec, pad, what, sema, info.current,
           info.numWaitThreads);
}

static void ds3mThread(void *arg)
{
    static ds3m_pad_t prev[DS3M_PADS];
    static u32 prevRpcIn;
    ds3m_pad_t cur;
    ds3m_event_t ev;
    u32 sec = 0, quiet[DS3M_PADS] = {0}, i, rpcIn;
    int pad, state, status, have, stuck;

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
            ds3mPrintEvent("ev", sec, &ev);
        }
        if (ds3mEvLost) {
            printf("DS3M %s %u lost %u events\n", DS3M_NAME, (unsigned int)sec, (unsigned int)ds3mEvLost);
            ds3mEvLost = 0;
        }
        if (sec == 15 || sec == 30) {
            for (i = (ds3mEvWrite > DS3M_EVENTS) ? ds3mEvWrite - DS3M_EVENTS : 0; i < ds3mEvRead; i++)
                ds3mPrintEvent("hist", sec, &ds3mEvents[i % DS3M_EVENTS]);
        }

        rpcIn = ds3mRpcIn;
        if (rpcIn != ds3mRpcOut) {
            printf("DS3M %s %u rpc cmd=%d busy=%ums calls=+%u\n", DS3M_NAME, (unsigned int)sec, ds3mRpcCmd,
                   (unsigned int)(ds3mUs(ds3mNow() - ds3mRpcTime) / 1000), (unsigned int)(rpcIn - prevRpcIn));
            ds3mPrintThread("rpc", sec, ds3mRpcThid);
        } else if (rpcIn == prevRpcIn && rpcIn != 0 && sec % 5 == 0) {
            printf("DS3M %s %u rpc idle: no calls from the EE\n", DS3M_NAME, (unsigned int)sec);
        }
        prevRpcIn = rpcIn;

        for (pad = 0; pad < DS3M_PADS; pad++) {
            CpuSuspendIntr(&state);
            cur = ds3m[pad];
            ds3m[pad].maxRepWait = 0;
            ds3m[pad].maxCmdWait = 0;
            ds3m[pad].maxPoll = 0;
            CpuResumeIntr(state);

            status = ds3mGetStatus ? ds3mGetStatus(pad) : 0;
            stuck = cur.pollIn != cur.polls;

            if (!stuck && cur.polls == prev[pad].polls && cur.repDone == prev[pad].repDone &&
                cur.cmdDone == prev[pad].cmdDone && cur.repQ == prev[pad].repQ && cur.cmdQ == prev[pad].cmdQ) {
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
            printf("DS3M %s %u p%d top since start: rep out=%d w=%u cmd out=%d w=%u poll=%u\n", DS3M_NAME,
                   (unsigned int)sec, pad, cur.topRepOut, (unsigned int)cur.topRepWait, cur.topCmdOut,
                   (unsigned int)cur.topCmdWait, (unsigned int)cur.topPoll);

            if (stuck) {
                printf("DS3M %s %u p%d STUCK in a poll for %ums\n", DS3M_NAME, (unsigned int)sec, pad,
                       (unsigned int)(ds3mUs(ds3mNow() - cur.pollInTime) / 1000));
                ds3mPrintThread("poll", sec, ds3mPollThid);
            }
            if (stuck || cur.polls == prev[pad].polls) {
                ds3mPrintSema(sec, pad, "mutex", cur.mutex);
                ds3mPrintSema(sec, pad, "cmd", cur.cmdSema);
            }
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
