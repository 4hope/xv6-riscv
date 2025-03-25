#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

void perror(const char *str) {
    write(2, str, strlen(str));
    exit(1);
}

void print_args(int pid, int argc, char *argv[], int use_mutex, int m) {
    if (use_mutex) {
        if (mutex_lock(m) < 0)
            perror("ERROR: mutex_lock\n");
    }

    char c[2];
    c[1] = '\0';
    for (int i = 1; i < argc; i++) {
        for (int j = 0; argv[i][j] != '\0'; j++) {
            c[0] = argv[i][j];
            printf("pid: %d, arg %d, char %s\n", pid, i, c);
        }
    }
    
    if (use_mutex) {
        if (mutex_unlock(m) < 0)
            perror("ERROR: mutex_unlock\n");
    }
}

int main(int argc, char *argv[])
{
    printf("Without mutex:\n");
    int pid = fork();
    if (pid < 0) {
        perror("ERROR: fork\n");
    }
    print_args(getpid(), argc, argv, 0, 0);
    if (pid == 0)
        exit(0);
    else
        wait(0);

    printf("With mutex:\n");
    int m = mutex();
    if (m < 0) {
        perror("ERROR: mutex\n");
    }
    pid = fork();
    if (pid < 0) {
        perror("ERROR: fork\n");
    }
    print_args(getpid(), argc, argv, 1, m);
    if (pid == 0)
        exit(0);
    else
        wait(0);

    close(m);
    exit(0);
}
