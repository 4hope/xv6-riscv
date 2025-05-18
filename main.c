#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <endian.h>
#include <string.h>

#define SUPER_SIZE 1024

uint64_t file_size;
FILE* file_system;
uint32_t block_size;

void direct(uint32_t block_pointer) {
    if (file_size <= 0) return;

    size_t to_print = (block_size > file_size) ? file_size : block_size;
    uint8_t *block_data = malloc(block_size);
    if (!block_data) {
        perror("malloc failed");
        exit(1);
    }
    
    if (block_pointer == 0) {
        memset(block_data, 0, to_print);
    } else {
        if (fseek(file_system, (long)block_pointer * block_size, SEEK_SET)) {
            perror("fseek failed");
            fclose(file_system);
            free(block_data);
            exit(1);
        }
        if (fread(block_data, to_print, 1, file_system) != 1) {
            perror("fread failed");
            fclose(file_system);
            free(block_data);
            exit(1);
        }
    }

    if (fwrite(block_data, 1, to_print, stdout) != to_print) {
        perror("fwrite failed");
        fclose(file_system);
        free(block_data);
        exit(1);
    }
    file_size -= to_print;
}

void indirect(uint8_t block_pointer, int level) {
    if (!block_pointer) {
        for (int i = 0; i < (int)block_size / 4; ++i) {
            direct(0);
        }
        return;
    }
    uint32_t block;
    for (int i = 0; i < (int)block_size / 4; ++i) {
        if (fseek(file_system, block_pointer * block_size + i * 4, SEEK_SET)) {
            perror("fseek failed");
            exit(1);
        }
        if (fread(&block, 4, 1, file_system) != 1) {
            perror("fread failed");
            exit(1);
        }

        if (!block) return;

        if (level != 0)
            indirect(block, level - 1);
        else
            direct(block);
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        perror("format: <filesystem> <inode>\n");
        exit(1);
    }

    int inode = atoi(argv[2]);

    file_system = fopen(argv[1], "rb");
    if (!file_system) {
        perror("fopen error");
        exit(1);
    }

    uint32_t log_block_size, blocks_per_group, inode_per_group, inode_size;
    
    if (fseek(file_system, SUPER_SIZE + 24, SEEK_SET)) {
        perror("fseek failed");
        fclose(file_system);
        exit(1);
    }
    if (fread(&log_block_size, 4, 1, file_system) != 1) {
        perror("fread failed");
        fclose(file_system);
        exit(1);
    }
    log_block_size = le32toh(log_block_size);
    
    block_size = 1024 << log_block_size;
    uint32_t descriptor_table = (block_size == 1024) ? 2 : 1;

    if (fseek(file_system, SUPER_SIZE + 32, SEEK_SET)) {
        perror("fseek failed");
        fclose(file_system);
        exit(1);
    }
    if (fread(&blocks_per_group, 4, 1, file_system) != 1) {
        perror("fread failed");
        fclose(file_system);
        exit(1);
    }
    blocks_per_group = le32toh(blocks_per_group);

    if (fseek(file_system, SUPER_SIZE + 40, SEEK_SET)) {
        perror("fseek failed");
        fclose(file_system);
        exit(1);
    }
    if (fread(&inode_per_group, 4, 1, file_system) != 1) {
        perror("fread failed");
        fclose(file_system);
        exit(1);
    }
    inode_per_group = le32toh(inode_per_group);

    if (fseek(file_system, SUPER_SIZE + 88, SEEK_SET)) {
        perror("fseek failed");
        fclose(file_system);
        exit(1);
    }
    if (fread(&inode_size, 4, 1, file_system) != 1) {
        perror("fread failed");
        fclose(file_system);
        exit(1);
    }
    inode_size = le32toh(inode_size);

    uint32_t inode_group = (inode - 1) / inode_per_group;

    if (fseek(file_system, descriptor_table * block_size + inode_group * 32, SEEK_SET)) {
        perror("fseek failed");
        fclose(file_system);
        exit(1);
    }

    uint32_t addr_inode_table;
    if (fseek(file_system, 8, SEEK_CUR)) {
        perror("fseek failed");
        fclose(file_system);
        exit(1);
    }
    if (fread(&addr_inode_table, 4, 1, file_system) != 1) {
        perror("fread failed");
        fclose(file_system);
        exit(1);
    }
    addr_inode_table = le32toh(addr_inode_table);

    uint32_t inode_index_in_group = (inode - 1) % inode_per_group;
    // uint32_t count_block = (inode_index_in_group * inode_size) / block_size; babybaby
    uint32_t inode_offset = addr_inode_table * block_size + inode_index_in_group * inode_size;

    if (fseek(file_system, inode_offset, SEEK_SET)) {
        perror("fseek to inode failed");
        fclose(file_system);
        exit(1);
    }

    uint32_t lower_bits;
    if (fseek(file_system, 4, SEEK_CUR)) {
        perror("fseek to inode failed");
        fclose(file_system);
        exit(1);
    }
    if (fread(&lower_bits, 4, 1, file_system) != 1) {
        perror("fread failed");
        fclose(file_system);
        exit(1);
    }

    uint32_t upper_bits;
    if (fseek(file_system, 104, SEEK_CUR)) {
        perror("fseek to inode failed");
        fclose(file_system);
        exit(1);
    }
    if (fread(&upper_bits, 4, 1, file_system) != 1) {
        perror("fread failed");
        fclose(file_system);
        exit(1);
    }
    lower_bits = le32toh(lower_bits);
    upper_bits = le32toh(upper_bits);

    file_size = lower_bits + (((long long)upper_bits) << 32);

    if (fseek(file_system, inode_offset, SEEK_SET)) {
        perror("fseek to inode failed");
        fclose(file_system);
        exit(1);
    }

    uint32_t block_pointers[15];
    for (int i = 0; i < 15; i++) {
        if (fseek(file_system, 40, SEEK_CUR)) {
            perror("fseek to inode failed");
            fclose(file_system);
            exit(1);
        }
        if (fread(&block_pointers[i], 4, 1, file_system) != 1) {
            perror("fread failed");
            fclose(file_system);
            exit(1);
        }
        block_pointers[i] = le32toh(block_pointers[i]);
    }

    for (int i = 0; i < 12; ++i) {
        direct(block_pointers[i]);
    }

    for (int i = 12; i < 15; ++i) {
        indirect(block_pointers[i], i - 11);
    }
    
    fclose(file_system);
    return 0;
}
