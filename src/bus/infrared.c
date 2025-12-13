/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "infrared.h"
#include <string.h>

static struct {
    bool overflow;
    uint16_t divider;
    union {
        struct {
            uint8_t emitting : 1;
            uint8_t receiving : 1;
            uint8_t : 4;
            uint8_t enabled : 2;
        };
        uint8_t raw;
    } control;
} infrared = {};

uint8_t cgbl_infrared_read(uint16_t address) {
    uint8_t result = 0xFF;
    switch (address) {
    case CGBL_INFRARED_CONTROL:
        if (cgbl_bus_mode() == CGBL_MODE_CGB) {
            result = infrared.control.raw;
        }
        break;
    default:
        break;
    }
    return result;
}

void cgbl_infrared_reset(void) {
    memset(&infrared, 0, sizeof(infrared));
    infrared.control.raw = 0x3E;
}

void cgbl_infrared_step(void) {
    if ((cgbl_bus_mode() == CGBL_MODE_CGB) && infrared.control.enabled == 3) {
        bool overflow = ++infrared.divider & 512;
        if (overflow && !infrared.overflow) {
            infrared.control.receiving = !infrared.control.emitting;
        }
        infrared.overflow = overflow;
    }
}

void cgbl_infrared_write(uint16_t address, uint8_t data) {
    switch (address) {
    case CGBL_INFRARED_CONTROL:
        if (cgbl_bus_mode() == CGBL_MODE_CGB) {
            infrared.control.raw = (data & 0xC1) | 0x3E;
            infrared.divider = 0;
            infrared.overflow = false;
        }
        break;
    default:
        break;
    }
}
