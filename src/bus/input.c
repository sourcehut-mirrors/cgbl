/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include "input.h"
#include "processor.h"

static struct {
    bool overflow;
    uint16_t divider;
    struct {
        bool current[CGBL_BUTTON_MAX];
        bool next[CGBL_BUTTON_MAX];
    } button;
    union {
        struct {
            uint8_t pressed : 4;
            uint8_t direction : 1;
            uint8_t button : 1;
        };
        uint8_t raw;
    } state;
} input = {};

bool (*cgbl_input_button(void))[CGBL_BUTTON_MAX]
{
    return &input.button.next;
}

uint8_t cgbl_input_read(uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case CGBL_INPUT_STATE:
            result = input.state.raw;
            break;
        default:
            break;
    }
    return result;
}

void cgbl_input_reset(void)
{
    memset(&input, 0, sizeof (input));
    input.state.raw = 0xCF;
}

void cgbl_input_step(void)
{
    bool overflow = ++input.divider & 256;
    if (overflow && !input.overflow)
    {
        bool changed = false;
        for (cgbl_button_e button = 0; button < CGBL_BUTTON_MAX; ++button)
        {
            if (input.button.current[button] != input.button.next[button])
            {
                input.button.current[button] = input.button.next[button];
                changed = true;
            }
        }
        if (changed)
        {
            cgbl_processor_interrupt(CGBL_INTERRUPT_INPUT);
        }
    }
    input.overflow = overflow;
}

void cgbl_input_write(uint16_t address, uint8_t data)
{
    switch (address)
    {
        case CGBL_INPUT_STATE:
            input.state.raw = data | 0xCF;
            if (!input.state.button)
            {
                for (cgbl_button_e button = CGBL_BUTTON_A; button <= CGBL_BUTTON_START; ++button)
                {
                    if (input.button.current[button])
                    {
                        input.state.pressed &= ~(1 << (button - CGBL_BUTTON_A));
                    }
                }
            }
            if (!input.state.direction)
            {
                for (cgbl_button_e button = CGBL_BUTTON_RIGHT; button <= CGBL_BUTTON_DOWN; ++button)
                {
                    if (input.button.current[button])
                    {
                        input.state.pressed &= ~(1 << (button - CGBL_BUTTON_RIGHT));
                    }
                }
            }
            break;
        default:
            break;
    }
}
