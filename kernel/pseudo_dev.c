#include "types.h"
#include "riscv.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "defs.h"
#include "file.h"

#define NULL 0
#define ZERO 1
#define URANDOM 2
#define NULLSTAT 3

#define SIZE 512
char nuuuul[SIZE];

struct pseudo {
    struct spinlock lock;
    uint64 urandom_seed, count;
};

static struct pseudo pseudo_dev;

uint8 urandom_next() {
    pseudo_dev.urandom_seed = pseudo_dev.urandom_seed * 1103515245 + 12345;
    return (uint8)(pseudo_dev.urandom_seed >> 16);
}

int pseudo_read(int user_dst, uint64 dst, int n, short minor) {
    int ans = 0;

    switch (minor) {
        case NULL:
            ans = 0;
            break;
        case ZERO:
            int cc = 0;
            while (n > SIZE) {
                if (either_copyout(user_dst, dst + cc, &nuuuul, SIZE) < 0) {
                    ans = -1;
                    break;
                }
                n -= SIZE;
                cc += SIZE;
            }
            if (ans == -1) break;

            if (either_copyout(user_dst, dst + cc, &nuuuul, n) < 0) {
                ans = -1;
            }
            ans = 0;
            break;
        case URANDOM:
            acquire(&pseudo_dev.lock);
            for (int i = 0; i < n; i++) {
                uint8 new = urandom_next();
                if (either_copyout(user_dst, dst + i, &new, 1) < 0) {
                    release(&pseudo_dev.lock);
                    ans = -1;
                    break;
                }
            }
            if (ans == -1) break;

            release(&pseudo_dev.lock);
            ans = n;
            break;
        case NULLSTAT:
            if (n != sizeof(uint64)) {
                ans = -1;
                break;
            }
            acquire(&pseudo_dev.lock);
            if (either_copyout(user_dst, dst, &pseudo_dev.count, n) < 0) {
                release(&pseudo_dev.lock);
                ans = -1;
                break;
            }
            release(&pseudo_dev.lock);
            ans = n;
            break;
        default:
            break;
    }

    return ans;
}

int pseudo_write(int user_src, uint64 src, int n, short minor) {
    int ans = -1;

    switch (minor) {
        case NULL:
            ans = n;
            break;
        case ZERO:
            ans = -1;
            break;
        case URANDOM:
            if (n != sizeof(uint64)) {
                ans = -1;
                break;
            }
            uint64 new_seed;
            if (either_copyin(&new_seed, user_src, src, n) < 0) {
                ans = -1;
                break;
            }
            acquire(&pseudo_dev.lock);
            pseudo_dev.urandom_seed = new_seed;
            release(&pseudo_dev.lock);
            ans = n;
            break;
        case NULLSTAT:
            acquire(&pseudo_dev.lock);
            pseudo_dev.count += n;
            release(&pseudo_dev.lock);
            ans = n;
            break;
        default:
            break;
    }

    return ans;
}

void pseudo_init(void) {
    initlock(&pseudo_dev.lock, "dev");
    for (int i = 0; i < SIZE; ++i)
        nuuuul[i] = 0;
    pseudo_dev.urandom_seed = 1337; pseudo_dev.count = 0;
    devsw[DEV_PSEUDO].read = pseudo_read;
    devsw[DEV_PSEUDO].write = pseudo_write;
}