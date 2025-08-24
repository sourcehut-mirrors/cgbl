/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef CGBL_BUS_H_
#define CGBL_BUS_H_

#include "common.h"

#define CGBL_BUS_MODE 0xFF4C
#define CGBL_BUS_PRIORITY 0xFF6C
#define CGBL_BUS_SPEED 0xFF4D

typedef enum {
    CGBL_MODE_DMG = 0,
    CGBL_MODE_CGB,
    CGBL_MODE_MAX,
} cgbl_mode_e;

typedef enum {
    CGBL_PRIORITY_DMG = 0,
    CGBL_PRIORITY_CGB,
    CGBL_PRIORITY_MAX,
} cgbl_priority_e;

typedef enum {
    CGBL_SPEED_NORMAL = 0,
    CGBL_SPEED_DOUBLE,
    CGBL_SPEED_MAX,
} cgbl_speed_e;

typedef struct {
    uint32_t length;
    uint8_t *data;
} cgbl_bank_t;

cgbl_mode_e cgbl_bus_mode(void);
cgbl_priority_e cgbl_bus_priority(void);
uint8_t cgbl_bus_read(uint16_t address);
cgbl_error_e cgbl_bus_reset(const cgbl_bank_t *const rom, cgbl_bank_t *const ram);
cgbl_error_e cgbl_bus_run(void);
cgbl_error_e cgbl_bus_run_breakpoint(uint16_t breakpoint);
cgbl_speed_e cgbl_bus_speed(void);
bool cgbl_bus_speed_change(void);
cgbl_error_e cgbl_bus_step(uint16_t breakpoint);
void cgbl_bus_write(uint16_t address, uint8_t data);

#endif /* CGBL_BUS_H_ */
