/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef CGBL_COMMON_H_
#define CGBL_COMMON_H_

#include "cgbl.h"

#define CGBL_VERSION_MAJOR 0
#define CGBL_VERSION_MINOR 2
#define CGBL_VERSION_PATCH 0x8609ebc

#define CGBL_ERROR(_FORMAT_, ...) \
    cgbl_error_set(__FILE__, __LINE__, _FORMAT_, ##__VA_ARGS__)
#define CGBL_LENGTH(_ARRAY_) \
    (sizeof (_ARRAY_) / sizeof (*(_ARRAY_)))
#define CGBL_WIDTH(_BEGIN_, _END_) \
    (((_END_) + 1) - (_BEGIN_))

cgbl_error_e cgbl_buffer_allocate(uint8_t **const buffer, uint32_t length);
void cgbl_buffer_free(uint8_t *const buffer);
cgbl_error_e cgbl_error_set(const char *const path, uint32_t line, const char *const format, ...);
bool cgbl_file_exists(const char *const path);
cgbl_error_e cgbl_file_read(const char *const path, uint8_t **const buffer, uint32_t *const length);
cgbl_error_e cgbl_file_write(const char *const path, const uint8_t *const buffer, uint32_t length);
cgbl_error_e cgbl_string_allocate(char **const string, const char *const format, ...);
void cgbl_string_free(char *const string);

#endif /* CGBL_COMMON_H_ */
