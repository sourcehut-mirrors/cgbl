/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

static char error[256] = {};

const char *cgbl_error(void)
{
    return error;
}

cgbl_error_e cgbl_error_set(const char *const path, uint32_t line, const char *const format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    vsnprintf(error, sizeof (error), format, arguments);
    va_end(arguments);
#ifndef NDEBUG
    snprintf(error + strlen(error), sizeof (error) - strlen(error), " (%s@%u)", path, line);
#endif /* NDEBUG */
    return CGBL_FAILURE;
}
