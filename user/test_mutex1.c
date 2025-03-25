#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

void print_args(int pid, int argc, char *argv[], int use_mutex, int m) {
    for (int i = 1; i < argc; i++) {
        for (int j = 0; argv[i][j] != '\0'; j++) {
            if (use_mutex) {
                mutex_lock(m);
            }

            printf("pid: %d, arg %d, char '%c'\n", pid, i, argv[i][j]);

            if (use_mutex) {
                mutex_unlock(m);
            }
        }
    }
}

void perror(const char *str) {
    write(2, str, strlen(str));
    exit(1);
}

int main(int argc, char *argv[])
{
    int m = sys_mutex();
    if (m < 0) {
        perror("Ошибка sys_mutex\n");
    }

    int pid = fork();
    if (pid < 0) {
        perror("Ошибка fork\n");
    }

    if (pid == 0) {
        print_args(getpid(), argc, argv, 0, 0);
        print_args(getpid(), argc, argv, 1, m);
        exit(0);
    }
    else {
        print_args(getpid(), argc, argv, 0, 0);
        print_args(getpid(), argc, argv, 1, m);
        wait(0);
    }

    exit(0);
}
