#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/stat.h"
#include "user/user.h"

void print_error(const char *str) {
    write(2, str, strlen(str));
    exit(1);
}

void child_process() {
    int sleep_time = 10;
    sleep(sleep_time);
    exit(1);
}

void parent_process(int child_pid, int kill_option) {
    printf("Родительский процесс: %d, дочерний процесс: %d\n", getpid(), child_pid);

    if (kill_option == 1) {
        if (kill(child_pid) < 0) {
            print_error("Ошибка kill\n");
        }
    }

    int status;
    int terminated_pid = wait(&status);
    if (terminated_pid < 0) {
        print_error("Ошибка wait\n");
    }

    printf("Дочерний процесс %d завершился. Код возврата: %d\n", terminated_pid, status);
}

int main(int argc, char **argv) {
    int kill_option = 0;

    if (argc > 1 && strcmp(argv[1], "-b\0") == 0) {
        kill_option = 1;
    }

    int pid = fork();

    if (pid < 0)
    {
        print_error("Ошибка fork\n");
    }

    if (pid == 0)
    {
        child_process();
    }
    else
    {
        parent_process(pid, kill_option);
    }
    exit(0);
}