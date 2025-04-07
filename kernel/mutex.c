#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"

int mutexalloc(struct file **f)
{
    struct sleeplock *m;
    m = 0;
    *f = 0;
    if ((*f = filealloc()) == 0) {
        if (m)
            kfree(m);
        if (*f)
            fileclose(*f);
        return -1;
    }
    if ((m = (struct sleeplock *)kalloc()) == 0) {
        if (m)
            kfree(m);
        if (*f)
            fileclose(*f);
        return -1;
    }
    initsleeplock(m, "mutex");
    (*f)->type = FD_MUTEX;
    (*f)->readable = 0;
    (*f)->writable = 0;
    (*f)->mutex = m;
    printf("Mutex alloced\n");
    return 0;
}

void mutexclose(struct file *f)
{
    if (f->type == FD_MUTEX) {
        struct sleeplock *m = f->mutex;
        kfree(m);
    }
    f->type = FD_NONE;
    printf("Mutex closed\n");
}