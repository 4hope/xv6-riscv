#include "kernel/types.h"
#include "user/user.h"

#define SIZE 2048

void perror(const char *str) {
    write(2, str, strlen(str));
    exit(1);
}

int main(int argc, char *argv[]) {
    printf("Таблица страниц при старте\n");
    print_pages(0, 0, 0);

    char var = 55;
    char stack_array[SIZE];
    stack_array[0] = 'a';
    stack_array[2000] = 'b';

    printf("\nТаблица страниц после работы со стеком\n");
    print_pages(&var, 1, 2);
    print_pages(stack_array, SIZE, 1);

    char *arr = malloc(SIZE);
    if ((uint64)arr == -1) {
        perror("Ошибка выделения памяти\n");
    }
    printf("\nТаблица страниц после выделения массива на куче\n");
    print_pages(arr, SIZE, 3);

    printf("\nТаблица страниц после Сброса A и D\n");
    rm_flags(arr, SIZE, 3);
    print_pages(arr, SIZE, 3);

    char val = arr[0];
    (void)val;
    printf("\nТаблица страниц после чтения данных\n");
    print_pages(arr, SIZE, 3);

    arr[0] = 'X';
    arr[2000] = 'Y';
    printf("\nТаблица страниц после изменения данных\n");
    print_pages(arr, SIZE, 3);

    free(arr);
    printf("\nТаблица страниц после освобождения массива\n");
    print_pages(arr, SIZE, 3);

    printf("\nСтраницы с флaгом А\n");
    print_pages(0, 0, 1);

    printf("\nСтраницы с флaгом D\n");
    print_pages(0, 0, 2);

    printf("\nСтраницы с флaгом A или D\n");
    print_pages(0, 0, 3);
}