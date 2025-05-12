#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"

struct spinlock rtc_lock;

void rtc_init() {
    initlock(&rtc_lock, "rtc");
}

uint32 rtc_low() {
    acquire(&rtc_lock);
    uint32 low = *(volatile uint32 *)RTC_LOW;
    release(&rtc_lock);
    return low;
}

uint32 rtc_high() {
    acquire(&rtc_lock);
    uint32 low = *(volatile uint32 *)RTC_HIGH;
    release(&rtc_lock);
    return low;
}

uint64 sys_rtc_read() {
    uint32 low = rtc_low(), high = rtc_high();
    return ((uint64)high << 32) + low;
}