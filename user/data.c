#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static const char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void print_time(uint64 time) {
    uint64 seconds = time / 1000000000;
    uint64 nanoseconds = time % 1000000000;

    uint64 minutes = seconds / 60;
    seconds %= 60;

    uint64 hours = minutes / 60;
    minutes %= 60;

    uint64 days = hours / 24;
    hours %= 24;

    uint64 year = 1970;
    uint64 day_of_year = days;

    while (day_of_year >= 365) {
        if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
            if (day_of_year >= 366) {
                day_of_year -= 366;
                year++;
            }
            else {
                break;
            }
        }
        else {
            day_of_year -= 365;
            year++;
        }
    }

    int month = 0;
    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
        days_in_month[1] = 29;
    }

    while (day_of_year >= days_in_month[month]) {
        day_of_year -= days_in_month[month];
        month++;
    }

    printf("%s %ld %ld ", months[month], day_of_year + 1, year);

    if (hours < 10)
        printf("0");
    printf("%ld:", hours);

    if (minutes < 10)
        printf("0");
    printf("%ld:", minutes);

    if (seconds < 10)
        printf("0");
    printf("%ld.", seconds);

    printf("%ld\n", nanoseconds);
}

int main(int argc, char *argv[]) {
    uint64 time = rtc_read();
    print_time(time);
    exit(0);
}