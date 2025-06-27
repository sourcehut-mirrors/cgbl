/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef CGBL_MAPPER_2_H_
#define CGBL_MAPPER_2_H_

#include "cartridge.h"

#define CGBL_MAPPER_2_BANK_SELECT_BEGIN 0x0000
#define CGBL_MAPPER_2_BANK_SELECT_END 0x3FFF

uint8_t cgbl_mapper_2_read(uint16_t address);
void cgbl_mapper_2_reset(void);
void cgbl_mapper_2_write(uint16_t address, uint8_t data);

#endif /* CGBL_MAPPER_2_H_ */
