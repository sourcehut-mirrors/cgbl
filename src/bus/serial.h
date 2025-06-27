/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef CGBL_SERIAL_H_
#define CGBL_SERIAL_H_

#include "bus.h"

#define CGBL_SERIAL_CONTROL 0xFF02
#define CGBL_SERIAL_DATA 0xFF01

uint8_t cgbl_serial_read(uint16_t address);
void cgbl_serial_reset(void);
void cgbl_serial_step(void);
void cgbl_serial_write(uint16_t address, uint8_t data);

#endif /* CGBL_SERIAL_H_ */
