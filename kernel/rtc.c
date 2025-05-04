#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"

uint32 rtc_low() {
    return *(volatile uint32 *)RTC_LOW;
}

uint32 rtc_high() {
    return *(volatile uint32 *)RTC_HIGH;
}

uint64 sys_rtc_read() {
    uint32 low = rtc_low(), high = rtc_high();
    return ((uint64)high << 32) + low;
}