/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include "audio.h"
#include "processor.h"
#include "timer.h"

static const uint16_t OVERFLOW[] = {
    512, 8, 32, 128,
};

static struct {
    uint8_t counter;
    uint16_t divider;
    uint8_t modulo;
    union {
        struct {
            uint8_t mode : 2;
            uint8_t enabled : 1;
        };
        uint8_t raw;
    } control;
    struct {
        bool audio;
        bool timer;
    } overflow;
} timer = {};

uint8_t cgbl_timer_read(uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case CGBL_TIMER_CONTROL:
            result = timer.control.raw;
            break;
        case CGBL_TIMER_COUNTER:
            result = timer.counter;
            break;
        case CGBL_TIMER_DIVIDER:
            result = timer.divider >> 8;
            break;
        case CGBL_TIMER_MODULO:
            result = timer.modulo;
            break;
        default:
            break;
    }
    return result;
}

void cgbl_timer_reset(void)
{
    memset(&timer, 0, sizeof (timer));
    timer.control.raw = 0xF8;
}

void cgbl_timer_step(void)
{
    for (uint8_t cycle = 0; cycle < ((cgbl_bus_speed() == CGBL_SPEED_DOUBLE) ? 2 : 1); ++cycle)
    {
        if (!cgbl_processor_stopped())
        {
            bool overflow = false;
            ++timer.divider;
            if (timer.control.enabled)
            {
                overflow = timer.divider & OVERFLOW[timer.control.mode];
                if (overflow && !timer.overflow.timer && !++timer.counter)
                {
                    cgbl_processor_signal(CGBL_INTERRUPT_TIMER);
                    timer.counter = timer.modulo;
                }
                timer.overflow.timer = overflow;
            }
            overflow = timer.divider & ((cgbl_bus_speed() == CGBL_SPEED_DOUBLE) ? 16384 : 8192);
            if (overflow && !timer.overflow.audio)
            {
                cgbl_audio_signal();
            }
            timer.overflow.audio = overflow;
        }
    }
}

void cgbl_timer_write(uint16_t address, uint8_t data)
{
    switch (address)
    {
        case CGBL_TIMER_CONTROL:
            timer.control.raw = data | 0xF8;
            break;
        case CGBL_TIMER_COUNTER:
            timer.counter = data;
            break;
        case CGBL_TIMER_DIVIDER:
            timer.divider = 0;
            break;
        case CGBL_TIMER_MODULO:
            timer.modulo = data;
            break;
        default:
            break;
    }
}
