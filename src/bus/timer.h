/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef CGBL_TIMER_H_
#define CGBL_TIMER_H_

#include "bus.h"

#define CGBL_TIMER_CONTROL 0xFF07
#define CGBL_TIMER_COUNTER 0xFF05
#define CGBL_TIMER_DIVIDER 0xFF04
#define CGBL_TIMER_MODULO 0xFF06

uint8_t cgbl_timer_read(uint16_t address);
void cgbl_timer_reset(void);
void cgbl_timer_step(void);
void cgbl_timer_write(uint16_t address, uint8_t data);

#endif /* CGBL_TIMER_H_ */
