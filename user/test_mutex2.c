#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

void perror(const char *str) {
    write(2, str, strlen(str));
    exit(1);
}

void test_read_write_mutex()
{
    printf("Test: read/write on mutex\n");
    int m = mutex();
    if (m < 0) {
        perror("ERROR: mutex\n");
    }

    char buf[10];
    if (read(m, buf, sizeof(buf)) >= 0)
        printf("ERROR: read on mutex should fail\n");
    else
        printf("PASS: read on mutex failed as expected\n");

    if (write(m, buf, sizeof(buf)) >= 0)
        printf("ERROR: write on mutex should fail\n");
    else
        printf("PASS: write on mutex failed as expected\n");

    close(m);
}

void test_close_locked_mutex()
{
    printf("Test: close locked mutex(self)\n");
    int m = mutex();
    if (m < 0) {
        perror("ERROR: mutex\n");
    }

    if (mutex_lock(m) < 0) {
        perror("ERROR: mutex_lock\n");
    }

    if (close(m) < 0)
        printf("ERROR: process cannot close its mutex\n");
    else
        printf("PASS: process close its mutex\n");


    printf("Test: close locked mutex(other)\n");
    m = mutex();
    if (m < 0) {
        perror("ERROR: mutex\n");
    }

    if (mutex_lock(m) < 0) {
        perror("ERROR: mutex_lock\n");
    }

    int pid = fork();
    if (pid < 0) {
        perror("ERROR: fork\n");
    }

    if (pid == 0) {
        if (close(m))
            printf("ERROR: close foreign mutex\n");
        else
            printf("PASS: can't close foreign mutex\n");
        exit(0);
    }
    else {
        wait(0);
    }
    close(m);
}

void test_process_exit() {
    printf("Test: process exit\n");

    int pid = fork();
    if (pid < 0) {
        perror("ERROR: fork\n");
    }

    if (pid == 0) {    
        int m = mutex();
        if (m < 0) {
            perror("ERROR: mutex\n");
        }
        if (mutex_lock(m) < 0) {
            perror("ERROR: mutex_lock\n");
        }
        printf("Child: exiting with locked mutex\n");
        exit(0);
    }
    else {
        wait(0);
        printf("ERROR if there was no message, PASS if there was\n");
    }
}

void test_unlock_other_process()
{
    printf("Test: unlock mutex held by another process\n");
    int m = mutex();
    if (m < 0) {
        perror("ERROR: mutex\n");
    }

    int pid = fork();
    if (pid == 0) {
        if (mutex_lock(m) < 0) {
            perror("ERROR: mutex_lock\n");
        }
        sleep(3);
        if (mutex_unlock(m) < 0) {
            perror("ERROR: mutex_unlock\n");
        }
        exit(0);
    }
    else {
        sleep(1);
        if (mutex_unlock(m) < 0)
            printf("PASS: Failed to unlock child's mutex as expected\n");
        else
            printf("ERROR: Parent unlocked child's mutex\n");

        wait(0);
        close(m);
    }
}

int main()
{
    test_read_write_mutex();
    test_close_locked_mutex();
    test_process_exit();
    test_unlock_other_process();

    exit(0);
}
