/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef CGBL_H_
#define CGBL_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum { CGBL_SUCCESS = 0, CGBL_FAILURE, CGBL_BREAKPOINT, CGBL_QUIT } cgbl_error_e;

typedef struct {
    bool debug;
    bool fullscreen;
    uint8_t scale;
} cgbl_option_t;

typedef struct {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
} cgbl_version_t;

cgbl_error_e cgbl_entry(const char *const path, const cgbl_option_t *const option);
const char *cgbl_error(void);
const cgbl_version_t *cgbl_version(void);

#endif /* CGBL_H_ */
