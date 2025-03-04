#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/stat.h"
#include "user/user.h"

void perror(const char *str) {
    write(2, str, strlen(str));
    exit(1);
}

void my_write(char* ptr, int pipefd[2], int len) {
    int ret;
    while (len > 0) {
        ret = write(pipefd[1], ptr, len);
        if (ret < 0) {
            perror("Ошибка: не удалось записать строку!\n");
        }
        len -= ret;
        ptr += ret;
    }
}

int main(int argc, char **argv) {
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        perror("Ошибка создания pipe\n");
    }

    int pid = fork();
    if (pid < 0) {
        perror("Ошибка fork\n");
    }

    if (pid == 0) {
        if (close(pipefd[1]) == -1) {
            perror("Ошибка close\n");
        }
        
        if (close(0) == -1) {
            perror("Ошибка close\n");
        }

        if (dup(pipefd[0]) == -1) {
            perror("Ошибка dup\n");
        }

        if (close(pipefd[0]) == -1) {
            perror("Ошибка close\n");
        }

        char *wc_argv[] = {"/wc", 0};
        exec("/wc", wc_argv);

        perror("Ошибка exec\n");
    }
    else {
        if (close(pipefd[0]) == -1) {
            perror("Ошибка close\n");
        }

        int len_buf = 0;
        char buffer[128];
        for (int i = 0; i < argc; i++) {
            int arg_len = strlen(argv[i]);
            if (len_buf + arg_len + 1 <= (int)sizeof(buffer) - 1) {
                memcpy(buffer + len_buf, argv[i], arg_len);
                len_buf += arg_len;
                buffer[len_buf] = '\n';
                len_buf++;
            }
            else {
                my_write(buffer, pipefd, len_buf);
                memset(buffer, 0, sizeof(buffer));
                len_buf = 0;
            }
        }
        if (len_buf > 0) {
            my_write(buffer, pipefd, len_buf);
        }

        if (close(pipefd[1])== -1) {
            perror("Ошибка close\n");
        }

        int status;
        wait(&status);
        exit(0);
    }
}