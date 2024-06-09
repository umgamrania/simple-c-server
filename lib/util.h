#pragma once

#ifndef _UTIL_H
#define _UTIL_H

#include <string.h>
#include <stdlib.h>

char **split(char *str, char *delim, int *count) {
    int len_delim = strlen(delim);
    int len_str = strlen(str);

    int needle = 0;
    int start = 0;

    int initial_arr_size = len_str / 4;
    int current_arr_size = 0;
    char **arr = (char**)malloc(initial_arr_size * sizeof(char*));

    for(int i = 0; i < len_str; i++) {
        if(str[i] == delim[needle]) {
            needle++;
            if(needle == len_delim) {
                if(current_arr_size == initial_arr_size) {
                    arr = (char**)realloc(arr, initial_arr_size * 2 * sizeof(char*));
                    initial_arr_size *= 2;
                    if(arr == NULL) {
                        printf("Reallocation of pointer array failed\n");
                        return NULL;
                    }
                }
                int substr_size = (i - len_delim + 1) - start;
                arr[current_arr_size] = (char*)malloc(substr_size + 1);
                strncpy(arr[current_arr_size], &(str[start]), substr_size);
                arr[current_arr_size][substr_size] = '\0';
                current_arr_size++;
                start = i + 1;
                needle = 0;
            }
        }else{
            needle = 0;
        }
    }

    int substr_size = len_str - start;
    arr[current_arr_size] = (char*)malloc(substr_size + 1);
    strncpy(arr[current_arr_size], &(str[start]), substr_size);
    arr[current_arr_size][substr_size] = '\0';

    if(count != NULL)
        *count = current_arr_size + 1;

    return arr;
}

int str_ends_with(char *str, char *tail) {
    int len_str = strlen(str);
    int len_tail = strlen(tail);

    if(len_tail > len_str) {
        return 0;
    }

    // result = last N chars of str == tail
    // where N is len_tail

    return !strcmp(&(str[len_str - len_tail]), tail);
}

#endif