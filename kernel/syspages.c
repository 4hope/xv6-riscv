#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

void print_page(int level, uint64 index, uint64 pt_id, pagetable_t pt, int mask) {
    char flags[8];
    flags[0] = (pt_id & PTE_R) ? 'R' : '_';
    flags[1] = (pt_id & PTE_W) ? 'W' : '_';
    flags[2] = (pt_id & PTE_X) ? 'X' : '_';
    flags[3] = (pt_id & PTE_U) ? 'U' : '_';
    flags[4] = (pt_id & PTE_G) ? 'G' : '_';
    flags[5] = (pt_id & PTE_A) ? 'A' : '_';
    flags[6] = (pt_id & PTE_D) ? 'D' : '_';
    flags[7] = '\0';

    if (!(((mask == 1 || mask == 3) && (flags[5] == 'A')) ||
        ((mask == 2 || mask == 3) && (flags[6] == 'D')) || (mask == 0)) && level == 3) {
            return;
        }

    const char *indent = "";
    switch (level) {
        case 1: indent = ""; break;
        case 2: indent = "........."; break;
        case 3: indent = "..................."; break;
        default: break;
    }

    const char *zeroes = "";
    if (index < 10) zeroes = "00";
    else if (index < 100) zeroes = "0";

    printf("%s0x%s%ld -> %p %s\n", indent, zeroes, index, pt, flags);
}

uint64 print_pages(uint64 buf, uint64 len, int mask) {
    if (mask > 3 || mask < 0 || len < 0)
        return -1;

    struct proc *p = myproc();
    pagetable_t pt0 = p->pagetable;

    uint64 start = 0, end = MAXVA - 1;
    if (buf && len) {
        start = buf;
        end = buf + len;
    }

    printf("PAGETABLE %p\n", pt0);

    for (int i = 0; i < 512; ++i) {
        if (!(pt0[i] & PTE_V)) continue;

        uint64 pt1_va = (uint64)i << PXSHIFT(2);
        uint64 pt1_va_end = pt1_va + (1UL << PXSHIFT(2));

        if (pt1_va >= end || pt1_va_end <= start)
            continue;

        pagetable_t pt1 = (pagetable_t)PTE2PA(pt0[i]);
        print_page(1, i, pt0[i], pt1, mask);

        for (int j = 0; j < 512; ++j) {
            if (!(pt1[j] & PTE_V)) continue;

            uint64 pt2_va = pt1_va | (uint64)j << PXSHIFT(1);
            uint64 pt2_va_end = pt2_va + (1UL << PXSHIFT(1));

            if (pt2_va >= end || pt2_va_end <= start)
                continue;

            pagetable_t pt2 = (pagetable_t)PTE2PA(pt1[j]);
            print_page(2, j, pt1[j], pt2, mask);

            for (int k = 0; k < 512; ++k) {
                if (!(pt2[k] & PTE_V)) continue;

                uint64 pt3_va = pt2_va | (uint64)k << PXSHIFT(0);
                uint64 pt3_va_end = pt3_va + (1UL << PXSHIFT(0));

                if (pt3_va >= end || pt3_va_end <= start)
                    continue;

                print_page(3, k, pt2[k], (pagetable_t)PTE2PA(pt2[k]), mask);
            }
        }
    }  
    return 0; 
}

uint64 rm_flags(uint64 buf, uint64 len, int mask) {
    if (mask > 3 || mask < 0 || len < 0)
        return -1;

    struct proc *p = myproc();
    pagetable_t pt0 = p->pagetable;

    uint64 start = 0, end = MAXVA - 1;
    if (buf && len) {
        start = buf;
        end = buf + len;
    }

    for (int i = 0; i < 512; ++i) {
        if (!(pt0[i] & PTE_V)) continue;

        uint64 pt1_va = (uint64)i << PXSHIFT(2);
        uint64 pt1_va_end = pt1_va + (1UL << PXSHIFT(2));

        if (pt1_va >= end || pt1_va_end <= start)
            continue;

        pagetable_t pt1 = (pagetable_t)PTE2PA(pt0[i]);

        for (int j = 0; j < 512; ++j) {
            if (!(pt1[j] & PTE_V)) continue;

            uint64 pt2_va = pt1_va | (uint64)j << PXSHIFT(1);
            uint64 pt2_va_end = pt2_va + (1UL << PXSHIFT(1));

            if (pt2_va >= end || pt2_va_end <= start)
                continue;

            pagetable_t pt2 = (pagetable_t)PTE2PA(pt1[j]);

            for (int k = 0; k < 512; ++k) {
                if (!(pt2[k] & PTE_V)) continue;

                uint64 pt3_va = pt2_va | (uint64)k << PXSHIFT(0);
                uint64 pt3_va_end = pt3_va + (1UL << PXSHIFT(0));

                if (pt3_va >= end || pt3_va_end <= start)
                    continue;

                if ((mask == 1 || mask == 3) && (pt2[k] & (uint64)PTE_A))
                    pt2[k] &= ~(uint64)PTE_A;
                if ((mask == 2 || mask == 3) && (pt2[k] & (uint64)PTE_D))
                    pt2[k] &= ~(uint64)PTE_D;
            }
        }
    }  
    return 0; 
}

uint64 sys_print_pages(void) {
    uint64 buf, len;
    int mask;

    argaddr(0, &buf);
    argaddr(1, &len);
    argint(2, &mask);
    return print_pages(buf, len, mask);
}

uint64 sys_rm_flags(void) {
    uint64 buf, len;
    int mask;

    argaddr(0, &buf);
    argaddr(1, &len);
    argint(2, &mask);
    return rm_flags(buf, len, mask);
}