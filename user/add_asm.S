.global main

main:
    addi sp, sp, -24
    mv a0, sp
    li a1, 25

    call gets

    mv t0, a0
    call replace_loop

    la a0, fmt_input
    mv a1, sp
    call printf

    # проверка, что поданная на вход строка имеет размер <= 23
    li t4, 23
    li t6, 0
    mv a0, sp
    call ret1
    bgt t6, t4, ret3

    # проверка на более 1 пробела
    mv a0, sp
    call find_space1
    addi a0, a0, 1
    
    call find_space2

    li t4, ' '
    # проверка, что в конце нет пробела
    mv a0, sp
    call ret1
    addi a0, a0, -1
    lb t6, 0(a0)
    beq t4, t6, ret2

    # проверка, что в начале нет пробела
    mv a0, sp
    lb t6, 0(a0)
    beq t4, t6, ret2

    call first_num

first_num:
    # первое число
    mv a0, sp

    li t4, 0
    li t5, '-'
    lb t6, 0(a0)

    beq t6, t5, is_negative1

    call parse_num1

replace_loop:
    lb t1, 0(t0)
    beqz t1, ret 
    li t2, '\n'
    beq t1, t2, replace

    addi t0, t0, 1
    j replace_loop

replace:
    sb zero, 0(t0)
    ret

ret:
    ret

find_space1:
    lb t1, 0(a0)
    beqz t1, ret2
    li t2, ' '
    beq t1, t2, ret

    addi a0, a0, 1
    j find_space1

ret1:
    lb t1, 0(a0)
    beqz t1, ret
    addi t6, t6, 1
    addi a0, a0, 1
    j ret1

is_negative1:
    li t4, 1
    addi a0, a0, 1

    call parse_num1

parse_num1:
    call atoi 

    bne t4, zero, get_negative1

    call second_num

get_negative1:
    sub a0, zero, a0

    call second_num

is_negative2:
    li t4, 1
    addi a0, a0, 1

    call parse_num2

parse_num2:
    call atoi 

    bne t4, zero, get_negative2

    call sum

get_negative2:
    sub a0, zero, a0

    call sum

second_num:
    mv s0, a0

    # второе число
    mv a0, sp

    call find_space1
    addi a0, a0, 1

    li t4, 0
    lb t6, 0(a0)

    beq t6, t5, is_negative2

    call parse_num2

sum:
    mv s1, a0

    # сумма
    add a0, s0, s1
    mv a1, a0
    la a0, fmt_sum
    call printf

    li a0, 0
    call exit

find_space2:
    lb t1, 0(a0)
    beqz t1, ret
    li t2, ' '
    beq t1, t2, ret2

    addi a0, a0, 1
    j find_space2

ret2:
    li a0, 2
    la a1, error_many_spaces
    li a2, 74
    li a7, 16
    ecall

    li a0, 1
    call exit

ret3:
    li a0, 2
    la a1, error_buffer_overflow
    li a2, 47
    li a7, 16
    ecall

    li a0, 1
    call exit


.section .rodata
fmt_input: .string "|%s|\n"
fmt_sum: .string "%d\n"
error_many_spaces: .string "Ошибка: строка не имеет вид |число число|\n"
error_buffer_overflow: .string "Ошибка: буфер переполнен!\n"
