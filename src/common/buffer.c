/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include "common.h"

cgbl_error_e cgbl_buffer_allocate(uint8_t **const buffer, uint32_t length)
{
    if (!(*buffer = calloc(length, sizeof (**buffer))))
    {
        return CGBL_ERROR("Failed to allocate buffer: %u bytes", length);
    }
    return CGBL_SUCCESS;
}

void cgbl_buffer_free(uint8_t *const buffer)
{
    free(buffer);
}
