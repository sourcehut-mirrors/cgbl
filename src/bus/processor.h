/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef CGBL_PROCESSOR_H_
#define CGBL_PROCESSOR_H_

#include "bus.h"

#define CGBL_PROCESSOR_INTERRUPT_ENABLE 0xFFFF
#define CGBL_PROCESSOR_INTERRUPT_FLAG 0xFF0F

typedef enum {
    CGBL_INTERRUPT_VBLANK = 0,
    CGBL_INTERRUPT_SCREEN,
    CGBL_INTERRUPT_TIMER,
    CGBL_INTERRUPT_SERIAL,
    CGBL_INTERRUPT_INPUT,
    CGBL_INTERRUPT_MAX,
} cgbl_interrupt_e;

bool cgbl_processor_halted(void);
uint8_t cgbl_processor_read(uint16_t address);
void cgbl_processor_reset(void);
void cgbl_processor_signal(cgbl_interrupt_e interrupt);
cgbl_error_e cgbl_processor_step(void);
bool cgbl_processor_stopped(void);
void cgbl_processor_write(uint16_t address, uint8_t data);

#endif /* CGBL_PROCESSOR_H_ */
