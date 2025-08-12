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

typedef enum {
    CGBL_REGISTER_A = 0,
    CGBL_REGISTER_AF,
    CGBL_REGISTER_B,
    CGBL_REGISTER_BC,
    CGBL_REGISTER_C,
    CGBL_REGISTER_D,
    CGBL_REGISTER_DE,
    CGBL_REGISTER_E,
    CGBL_REGISTER_F,
    CGBL_REGISTER_H,
    CGBL_REGISTER_HL,
    CGBL_REGISTER_L,
    CGBL_REGISTER_PC,
    CGBL_REGISTER_SP,
    CGBL_REGISTER_MAX,
} cgbl_register_e;

typedef union {
    struct {
        union {
            struct {
                uint8_t : 4;
                uint8_t carry : 1;
                uint8_t half_carry : 1;
                uint8_t negative : 1;
                uint8_t zero : 1;
            };
            uint8_t low;
        };
        uint8_t high;
    };
    uint16_t word;
} cgbl_register_t;

bool cgbl_processor_halted(void);
cgbl_error_e cgbl_processor_register_read(cgbl_register_e address, cgbl_register_t *const data);
cgbl_error_e cgbl_processor_register_write(cgbl_register_e address, const cgbl_register_t *const data);
uint8_t cgbl_processor_read(uint16_t address);
void cgbl_processor_reset(void);
void cgbl_processor_signal(cgbl_interrupt_e interrupt);
cgbl_error_e cgbl_processor_step(uint16_t breakpoint);
bool cgbl_processor_stopped(void);
void cgbl_processor_write(uint16_t address, uint8_t data);

#endif /* CGBL_PROCESSOR_H_ */
