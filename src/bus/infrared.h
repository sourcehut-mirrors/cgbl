/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef CGBL_INFRARED_H_
#define CGBL_INFRARED_H_

#include "bus.h"

#define CGBL_INFRARED_CONTROL 0xFF56

uint8_t cgbl_infrared_read(uint16_t address);
void cgbl_infrared_reset(void);
void cgbl_infrared_step(void);
void cgbl_infrared_write(uint16_t address, uint8_t data);

#endif /* CGBL_INFRARED_H_ */
