//
// Created by avitalrose on 8/11/21.
//
#include "stdio.h"

int main(int argc, char *argv[]) {
    printf("hello world from c program\n");
    printf("number of arguments is %d\n", argc);
    printf("arguments are: %s\n", argv[1]);
    fflush(stdout);
    return 0;
}