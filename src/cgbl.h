/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef CGBL_H_
#define CGBL_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    CGBL_SUCCESS = 0,
    CGBL_FAILURE,
    CGBL_QUIT,
} cgbl_error_e;

typedef struct {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
} cgbl_version_t;

cgbl_error_e cgbl_entry(const char *const path, uint8_t scale, bool fullscreen);
const char *cgbl_error(void);
const cgbl_version_t *cgbl_version(void);

#endif /* CGBL_H_ */
