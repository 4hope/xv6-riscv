#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/procinfo.h"

void check(int x) {
    if (x) printf("Ok\n");
    else printf("Wrong\n");
}

int main() {
    int result;

    result = ps_listinfo(0, 0);
    check(result > 0);

    struct procinfo small_buf[1];
    result = ps_listinfo(small_buf, 0);
    check(result == -1);

    int lim = 1;
    struct procinfo *ptable = 0;
    while (1) {
        ptable = malloc(sizeof(struct procinfo) * lim);
        result = ps_listinfo(ptable, lim);

        if (result == -1) {
            free(ptable);
            lim *= 2;
            continue;
        }

        free(ptable);
        break;
    }
    check(result > 0);

    result = ps_listinfo((struct procinfo *)0xFFFFFFFF, 10);
    check(result == -2);
    exit(0);
}