/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include "processor.h"
#include "serial.h"

static struct {
    bool overflow;
    uint8_t data;
    uint16_t divider;
    union {
        struct {
            uint8_t select : 1;
            uint8_t speed : 1;
            uint8_t : 5;
            uint8_t enabled : 1;
        };
        uint8_t raw;
    } control;
} serial = {};

uint8_t cgbl_serial_read(uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case CGBL_SERIAL_CONTROL:
            result = serial.control.raw;
            break;
        case CGBL_SERIAL_DATA:
            result = serial.data;
            break;
        default:
            break;
    }
    return result;
}

void cgbl_serial_reset(void)
{
    memset(&serial, 0, sizeof (serial));
    serial.control.raw = 0x7C;
}

void cgbl_serial_step(void)
{
    for (uint8_t cycle = 0; cycle < ((cgbl_bus_speed() == CGBL_SPEED_DOUBLE) ? 2 : 1); ++cycle)
    {
        if (serial.control.enabled && serial.control.select)
        {
            bool overflow = ++serial.divider & (serial.control.speed ? 64 : 2048);
            if (overflow && !serial.overflow)
            {
                cgbl_processor_signal(CGBL_INTERRUPT_SERIAL);
                serial.control.enabled = false;
                serial.data = 0xFF;
                serial.divider = 0;
                serial.overflow = false;
            }
            else
            {
                serial.overflow = overflow;
            }
        }
    }
}

void cgbl_serial_write(uint16_t address, uint8_t data)
{
    switch (address)
    {
        case CGBL_SERIAL_CONTROL:
            serial.control.raw = (data & 0x83) | 0x7C;
            serial.divider = 0;
            serial.overflow = false;
            break;
        case CGBL_SERIAL_DATA:
            serial.data = data;
            break;
        default:
            break;
    }
}
