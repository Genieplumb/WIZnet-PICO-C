
/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>

#include "port_common.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "RP2040.h"
/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
#define CMD_STRING_SIZE     128

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */


/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
int getInputString(char *buffP);
char* custom_strtok(char* str, const char* delim);
// uint32_t string_to_hex32(const char* str);
// uint64_t string_to_hex64(const char* str) ;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */

int getInputString(char *buffP)
{
    int bytes_read = 0;
    char c = 0;
    do
    {
        c = getchar_timeout_us(0xFFFFFFFF);
        if (c == '\r')
            break;
        if (c == '\b') /* Backspace */
        {
            if (bytes_read > 0)
            {
                printf("\b \b");
                bytes_read--;
            }
            continue;
        }
        if (bytes_read >= CMD_STRING_SIZE)
        {
            printf("Command string size overflow\r\n");
            bytes_read = 0;
            continue;
        }
        if (c >= 0x20 && c <= 0x7E)
        {
            buffP[bytes_read++] = c;
            printf("%c", c);
        }
    } while (1);
    printf("\n\r");
    buffP[bytes_read] = '\0';
    return bytes_read;
}

char* custom_strtok(char* str, const char* delim) 
{
    static char* next = NULL; 
    if (str != NULL) {
        next = str; 
    }
    if (next == NULL) {
        return NULL; 
    }

    char* token_start = next;

    while (*next) {
        const char* d = delim;
        while (*d) {
            if (*next == *d) { 
                *next = '\0';  
                next++;        
                if (token_start != next - 1) {
                    return token_start; 
                } else {
                    token_start = next; 
                    break;
                }
            }
            d++;
        }
        next++;
    }

    if (*token_start) {
        next = NULL;
        return token_start;
    }

    return NULL;
}
#if 0
static int hex_char_to_int(char c) 
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1; 
}

uint32_t string_to_hex32(const char* str) {
    uint32_t result = 0;

    while (*str) {
        int value = hex_char_to_int(*str);
        if (value == -1) {
            printf("Invalid hex character: %c\n", *str);
            return 0; 
        }

        if (result > (0xFFFFFFFF >> 4)) {
            printf("Overflow detected: input too large for 32-bit\n");
            return 0; 
        }

        result = (result << 4) | value; 
        str++;
    }

    return result;
}

uint64_t string_to_hex64(const char* str) 
{
    uint64_t result = 0;

    while (*str) {
        int value = hex_char_to_int(*str);
        if (value == -1) {
            printf("Invalid hex character: %c\n", *str);
            return 0; 
        }
        if (result > (0xFFFFFFFFFFFFFFFF >> 4)) {
            printf("Overflow detected: input too large for 64-bit\n");
            return 0; 
        }

        result = (result << 4) | value; 
        str++;
    }

    return result;
}
#endif