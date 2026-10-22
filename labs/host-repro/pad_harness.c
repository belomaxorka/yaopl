/* Host harness for src/pad.c readPads(): the button repeat countdown across the
 * wrap of the 32-bit tick counter. cpu_ticks() is scripted: three polls 10 ms
 * apart, the last one after the counter wrapped. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int u32;
#define LOG(...)

struct pad_data_t
{
    int port, slot;
};

/* the statics of pad.c, including the ones a fix may add */
static u32 curtime = 0;
static u32 time_since_last = 0;
static u32 lastticks = 0;
static u32 tickrem = 0;
static unsigned short pad_count = 1;
static struct pad_data_t pad_data[4];
static u32 paddata;
static u32 oldpaddata;
static int delaycnt[16];
static int paddelay[16];

static u32 fake_ticks;
static int held;

static u32 cpu_ticks(void) { return fake_ticks; }
static int readPad(struct pad_data_t *pad)
{
    paddata = held ? 0x40 : 0;
    return 0;
}
static int getKeyPressed(int id) { return held; }

#include "pad_extracted.c"

int main(int argc, char **argv)
{
    const u32 ms = CLOCKS_PER_MILISEC;
    int i, before;

    for (i = 0; i < 16; i++)
        paddelay[i] = 200;

    /* 15 ms before the wrap, nothing held: the countdown is armed at the initial delay */
    fake_ticks = (u32)(0 - 15 * ms);
    held = 0;
    readPads();
    /* 5 ms before the wrap, button held */
    fake_ticks = (u32)(0 - 5 * ms);
    held = 1;
    readPads();
    before = delaycnt[0];
    /* 5 ms after the wrap, still held: 10 ms passed */
    fake_ticks = 5 * ms;
    readPads();
    printf("held button: countdown %d ms before the wrap, %d ms after it (10 ms later)\n", before, delaycnt[0]);
    return 0;
}
