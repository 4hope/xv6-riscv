#include "kernel/types.h"
#include "user/user.h"

#define SIZE 2048
#define ZERO 0
#define A 1
#define D 2
#define AD (A | D)

void perror(const char *str) {
    write(2, str, strlen(str));
    exit(1);
}

int main(int argc, char *argv[]) {
    printf("Таблица страниц при старте\n");
    print_pages(0, 0, ZERO);

    char var = 55;
    char stack_array[SIZE];
    stack_array[0] = 'a';
    stack_array[2000] = 'b';

    printf("\nТаблица страниц после работы со стеком\n");
    print_pages(&var, 1, D);
    print_pages(stack_array, SIZE, A);

    char *arr = malloc(SIZE);
    if ((uint64)arr == -1) {
        perror("Ошибка выделения памяти\n");
    }
    printf("\nТаблица страниц после выделения массива на куче\n");
    print_pages(arr, SIZE, AD);

    printf("\nТаблица страниц после Сброса A и D\n");
    rm_flags(arr, SIZE, AD);
    print_pages(arr, SIZE, AD);

    char val = arr[0];
    (void)val;
    printf("\nТаблица страниц после чтения данных\n");
    print_pages(arr, SIZE, AD);

    arr[0] = 'X';
    arr[2000] = 'Y';
    printf("\nТаблица страниц после изменения данных\n");
    print_pages(arr, SIZE, AD);

    free(arr);
    printf("\nТаблица страниц после освобождения массива\n");
    print_pages(arr, SIZE, AD);

    printf("\nСтраницы с флaгом А\n");
    print_pages(0, 0, A);

    printf("\nСтраницы с флaгом D\n");
    print_pages(0, 0, D);

    printf("\nСтраницы с флaгом A или D\n");
    print_pages(0, 0, AD);
}