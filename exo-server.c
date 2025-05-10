#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#define COUNT_SECONDS 5
#define LOG_FILE "/tmp/exo-server.txt"
#define FIFO_NAME "/tmp/exo-server"
#define BUF_SIZE 4096

volatile sig_atomic_t sig_term = 0;
volatile sig_atomic_t sig_int = 0;
volatile sig_atomic_t sig_alrm = 0;
volatile sig_atomic_t sig_usr1 = 0;
volatile sig_atomic_t sig_hup = 0;

FILE *f = NULL;
int fifo = -1;
bool is_demon = false;

typedef struct {
    uint64_t count_messages;
    uint64_t len_messages;
    uint64_t count_alarms;
} Stats;

Stats stats = {0, 0, 0};

void print_stats() {
    fprintf(f, "Statistics: %lu messages, %lu bytes, %lu alarms\n", stats.count_messages, stats.len_messages, stats.count_alarms);
    fflush(f);
}

void sig_term_handler_fnc(int signo) { (void)signo; sig_term = 1; }
void sig_int_handler_fnc(int signo) { (void)signo; sig_int = 1; }
void sig_alrm_handler_fnc(int signo) { (void)signo; sig_alrm = 1; }
void sig_usr1_handler_fnc(int signo) { (void)signo; sig_usr1 = 1; }
void sig_hup_handler_fnc(int signo) { (void)signo; sig_hup = 1; }

void set_signals() {
    struct sigaction sig_term_handler = {.sa_handler = sig_term_handler_fnc};
    if (sigaction(SIGTERM, &sig_term_handler, 0) < 0) {
        perror("sigation(SIGTERM)");
        exit(EXIT_FAILURE);
    }

    struct sigaction sig_int_handler = {.sa_handler = sig_int_handler_fnc};
    if (sigaction(SIGINT, &sig_int_handler, 0) < 0) {
        perror("sigation(SIGINT)");
        exit(EXIT_FAILURE);
    }

    struct sigaction sig_quit_handler = {.sa_handler = SIG_IGN};
    if (sigaction(SIGQUIT, &sig_quit_handler, 0) < 0) {
        perror("sigation(SIGQUIT)");
        exit(EXIT_FAILURE);
    }

    struct sigaction sig_alrm_handler = {.sa_handler = sig_alrm_handler_fnc};
    if (sigaction(SIGALRM, &sig_alrm_handler, 0) < 0) {
        perror("sigation(SIGALRM)");
        exit(EXIT_FAILURE);
    }

    struct sigaction sig_usr1_handler = {.sa_handler = sig_usr1_handler_fnc};
    if (sigaction(SIGUSR1, &sig_usr1_handler, 0) < 0) {
        perror("sigation(SIGUSR1)");
        exit(EXIT_FAILURE);
    }

    struct sigaction sig_hup_handler = {.sa_handler = sig_hup_handler_fnc};
    if (sigaction(SIGHUP, &sig_hup_handler, 0) < 0) {
        perror("sigation(SIGHUP)");
        exit(EXIT_FAILURE);
    }
}

void cleanup() {
    if (f && f != stdout) {
        if (fclose(f) == -1) {
            perror("fclose failed");
            exit(EXIT_FAILURE);
        }
    }
    if (fifo != -1 && close(fifo) < 0) {
        perror("close failed");
        exit(EXIT_FAILURE);
    }
    if (unlink(FIFO_NAME) == -1) {
        perror("unlink failed");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

void demonize(bool flag) {
    if (is_demon) return;

    if (flag) {
        daemon(1, 0);
        f = fopen(LOG_FILE, "w");
        if (!f) {
            perror("fopen failed");
            exit(EXIT_FAILURE);
        }
    }
    else {
        pid_t pid = fork();
        if (pid < 0)
            exit(EXIT_FAILURE);
        if (pid > 0)
            exit(EXIT_SUCCESS);

        if (setsid() == -1) {
            perror("setsid failed");
            exit(EXIT_FAILURE);
        }

        pid = fork();
        if (pid < 0)
            exit(EXIT_FAILURE);
        if (pid > 0)
            exit(EXIT_SUCCESS);

        umask(0);
        chdir("/");

        f = fopen(LOG_FILE, "w");
        if (!f) {
            perror("fopen failed");
            exit(EXIT_FAILURE);
        }

        int fd = fileno(f);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
    }
    is_demon = true;
}

void eintr_error() {
    if (sig_term) {
        fprintf(f, "SIGTERM: interruption of work at the user's request\n");
        print_stats();
        cleanup();
    }
    else if (sig_int){
        fprintf(f, "SIGINT: user request, read the data and finish the work\n");
        print_stats();
    }
    else if (sig_usr1) {
        sig_usr1 = 0;
        print_stats();
    }
    else if (sig_alrm) {
        sig_alrm = 0;
        stats.count_alarms++;
        fprintf(f, "SIGALRM: Waiting for data...\n");
        alarm(COUNT_SECONDS);
    }
    else if (sig_hup) {
        sig_hup = 0;
        demonize(false);
        fprintf(f, "SIGHUP: switching to daemon mode\n");
        print_stats();
        alarm(COUNT_SECONDS);
    }
}

int main(int argc, char **argv) {
    // expected input: ./program -d (if it is demon), ./program (if it is foreground regime)
    (void)argv;
    
    if (argc != 1 && argc != 2) {
        exit(EXIT_FAILURE);
    }

    f = stdout;
    if (argc == 2) {
        demonize(true);
        is_demon = true;
    }
    set_signals();

    if (mkfifo(FIFO_NAME, 0600) == -1) {
        if (errno == EEXIST) {
            struct stat st;
            if (stat(FIFO_NAME, &st) == -1) {
                perror("stat failed");
                exit(EXIT_FAILURE);
            }
            if (!S_ISFIFO(st.st_mode)) {
                perror("file exists but it is not FIFO");
                exit(EXIT_FAILURE);
            }
            else {
                fprintf(f, "FIFO exists, will use it\n");
            }
        }
        else {
            perror("mkfifo failed");
            exit(EXIT_FAILURE);
        }
    }
    else 
        fprintf(f, "FIFO created\n");

    alarm(COUNT_SECONDS);

    bool is_sig_int = false;
    while (true) {
        fifo = open(FIFO_NAME, O_RDONLY);
        if (fifo < 0) {
            if (errno == EINTR) {
                eintr_error();
                if (sig_int) {
                    sig_int = 0;
                    cleanup();
                }
                continue;
            }
            else {
                perror("open failed");
                exit(EXIT_FAILURE);
            }
        }

        char buffer[BUF_SIZE + 1];
        ssize_t n;
        while (true) {
            n = read(fifo, buffer, BUF_SIZE);
            if (n < 0) {
                if (errno == EINTR) {
                    eintr_error();
                    if (sig_int) {
                        sig_int = 0;
                        is_sig_int = true;
                    }
                    continue;
                }
                else {
                    perror("read failed");
                    exit(EXIT_FAILURE);
                }
            }
            else if (n == 0) {
                if (is_sig_int) cleanup();

                if (fifo != -1 && close(fifo) < 0) {
                    perror("close failed");
                    exit(EXIT_FAILURE);
                }
                break;
            }
    
            buffer[n] = '\0';
            if (buffer[n - 1] != '\n') {
                buffer[n - 1] = '\n';
            }

            fputs(buffer, f);

            fflush(f);

            stats.len_messages += n;
        }
        stats.count_messages++;
        
        if (sig_alrm || sig_hup || sig_term || sig_usr1 || sig_int) {
            eintr_error();
            if (sig_int) {
                sig_int = 0;
                cleanup();
            }
        }
    }
}