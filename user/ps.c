#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/procinfo.h"

void perror(const char *str) {
    write(2, str, strlen(str));
    exit(1);
}

void print_state(enum procinfo_state state) {
    if (state == PROC_UNUSED)
        printf("UNUSED");
    else if (state == PROC_USED)
        printf("USED");
    else if (state == PROC_SLEEPING)
        printf("SLEEPING");
    else if (state == PROC_RUNNABLE)
        printf("RUNNABLE");
    else if (state == PROC_RUNNING)
        printf("RUNNING");
    else if (state == PROC_ZOMBIE)
        printf("ZOMBIE");
}

void print_ps_listinfo() {
    int lim = 1;
    struct procinfo *ptable = 0;

    while (1) {
        ptable = malloc(sizeof(struct procinfo) * lim);
        if (!ptable) {
            perror("Ошибка: не удалось выделить память\n");
        }

        int count_process = ps_listinfo(ptable, lim);

        if (count_process < 0) {
            if (count_process == -1) {
                free(ptable);
                lim *= 2;
                continue;
            }
            else {
                free(ptable);
                perror("Ошибка: запись по указанному адресу пользовательского процесса недопустима\n");
            }
        }

        for (int i = 0; i < count_process; ++i) {
            struct procinfo *curr_proc = &ptable[i];
            printf("id = %d, name = %s, state = ", curr_proc->pid, curr_proc->name);

            print_state(curr_proc->state);

            printf(", ppid = %d, pname = %s\n", curr_proc->ppid, curr_proc->pname);
        }

        free(ptable);
        break;
    }
}

int main() {
    print_ps_listinfo();
    exit(0);
}