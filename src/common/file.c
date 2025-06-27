/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "common.h"

bool cgbl_file_exists(const char *const path)
{
    FILE *file = NULL;
    if (!(file = fopen(path, "rb")))
    {
        return false;
    }
    fclose(file);
    return true;
}

cgbl_error_e cgbl_file_read(const char *const path, uint8_t **const buffer, uint32_t *const length)
{
    FILE *file = NULL;
    cgbl_error_e result = CGBL_SUCCESS;
    if (!(file = fopen(path, "rb")))
    {
        return CGBL_ERROR("Failed to open file: %s", path);
    }
    fseek(file, 0, SEEK_END);
    *length = ftell(file);
    fseek(file, 0, SEEK_SET);
    if ((result = cgbl_buffer_allocate(buffer, *length)) == CGBL_SUCCESS)
    {
        if (fread(*buffer, sizeof (**buffer), *length, file) != *length)
        {
            result = CGBL_ERROR("Failed to read file: %s", path);
        }
    }
    fclose(file);
    return result;
}

cgbl_error_e cgbl_file_write(const char *const path, const uint8_t *const buffer, uint32_t length)
{
    FILE *file = NULL;
    cgbl_error_e result = CGBL_SUCCESS;
    if (!(file = fopen(path, "wb")))
    {
        return CGBL_ERROR("Failed to open file: %s", path);
    }
    if (fwrite(buffer, sizeof (*buffer), length, file) != length)
    {
        result = CGBL_ERROR("Failed to write file: %s", path);
    }
    fclose(file);
    return result;
}
