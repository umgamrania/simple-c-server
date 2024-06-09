#pragma once

#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

#include <stdio.h>

int file_exists(char *path) {
    FILE *f = fopen(path, "r");

    if(f == NULL) 
        return 0;
    
    fclose(f);
    return 1;
}

long get_file_size(char *path) {
    FILE *f = fopen(path, "r");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);
    return size;
}

#endif