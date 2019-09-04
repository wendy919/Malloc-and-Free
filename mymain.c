#include <stdio.h>
#include "mem.h"

int main(int argc, char *argv[]) {
    mem_init(4096);
    void *ptr, *ptr1, *ptr2, *ptr3;
    printf("------1------\n");
    mem_dump();
    ptr2 = mem_alloc(16, 1);
    ptr3 = mem_alloc(16, 1);
    ptr = mem_alloc(1024, 2);
    mem_alloc(8, 0);
    mem_free(ptr);
    printf("------2------\n");
    mem_dump();
    ptr = mem_alloc(16, 2);
    ptr1 = mem_alloc(32, 1);
    mem_free(ptr1);
    mem_alloc(4, 0);
    mem_free(ptr);
    ptr = mem_alloc(80, 1);
    ptr1 = mem_alloc(80, 1);
    mem_free(ptr);
    printf("------5------\n");
    mem_dump();
    mem_free(ptr1);
    printf("------5------\n");
    mem_dump();
    printf("------5------\n");
    mem_free(ptr2);
    mem_dump();
    printf("------5------\n");
    mem_free(ptr3);
    mem_dump();
    return 0;
}