#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define BUFSIZE 2048

void my_write(int fd, char *ptr, int len) {
    ssize_t ret;
    while (len > 0) {
        ret = write(fd, ptr, len);
        if (ret < 0) {
            perror("Ошибка: не удалось записать строку");
            exit(1);
        }
        len -= ret;
        ptr += ret;
    }
}

int main(int argc, char *argv[])
{
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        perror("Ошибка создания pipe");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        perror("Ошибка fork");
        exit(1);
    }

    if (pid == 0){
        if (close(pipefd[1]) == -1) {
            perror("Ошибка close");
            exit(1);
        }
        
        if (close(0) == -1) {
            perror("Ошибка close");
            exit(1);
        }

        char buffer[BUFSIZE];
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, BUFSIZE)) != 0)
        {
            if (bytes_read < 0)
            {
                perror("Ошибка чтения из pipe");
                exit(1);
            }
            my_write(1, buffer, bytes_read);
        }

        if (close(pipefd[0]) == -1) {
            perror("Ошибка close");
            exit(1);
        }
    }
    else {
        if (close(pipefd[0]) == -1) {
            perror("Ошибка close");
            exit(1);
        }

        int len_buf = 0;
        char buffer[BUFSIZE];
        for (int i = 0; i < argc; i++) {
            int arg_len = strlen(argv[i]);
            if (len_buf + arg_len + 1 <= BUFSIZE) {
                memcpy(buffer + len_buf, argv[i], arg_len);
                len_buf += arg_len;
                buffer[len_buf] = '\n';
                len_buf++;
            }
            else {
                my_write(pipefd[1], buffer, len_buf);
                len_buf = 0;
            }
        }
        if (len_buf > 0) {
            my_write(pipefd[1], buffer, len_buf);
        }

        if (close(pipefd[1])== -1) {
            perror("Ошибка close");
            exit(1);
        }

        int status;
        wait(&status);
        exit(0);
    }
}
