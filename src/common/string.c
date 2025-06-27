/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <stdarg.h>
#include <stdio.h>
#include "common.h"

cgbl_error_e cgbl_string_allocate(char **const string, const char *const format, ...)
{
    uint32_t length = 0;
    va_list arguments;
    cgbl_error_e result = CGBL_SUCCESS;
    va_start(arguments, format);
    length = vsnprintf(NULL, 0, format, arguments) + 1;
    va_end(arguments);
    if ((result = cgbl_buffer_allocate((uint8_t **const)string, length)) == CGBL_SUCCESS)
    {
        va_start(arguments, format);
        vsnprintf(*string, length, format, arguments);
        va_end(arguments);
    }
    return result;
}

void cgbl_string_free(char *const string)
{
    cgbl_buffer_free((uint8_t *const)string);
}
