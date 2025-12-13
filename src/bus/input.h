/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef CGBL_INPUT_H_
#define CGBL_INPUT_H_

#include "bus.h"

#define CGBL_INPUT_STATE 0xFF00

typedef enum {
    CGBL_BUTTON_A = 0,
    CGBL_BUTTON_B,
    CGBL_BUTTON_SELECT,
    CGBL_BUTTON_START,
    CGBL_BUTTON_RIGHT,
    CGBL_BUTTON_LEFT,
    CGBL_BUTTON_UP,
    CGBL_BUTTON_DOWN,
    CGBL_BUTTON_MAX
} cgbl_button_e;

bool (*cgbl_input_button(void))[CGBL_BUTTON_MAX];
uint8_t cgbl_input_read(uint16_t address);
void cgbl_input_reset(void);
void cgbl_input_step(void);
void cgbl_input_write(uint16_t address, uint8_t data);

#endif /* CGBL_INPUT_H_ */
