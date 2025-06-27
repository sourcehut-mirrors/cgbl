/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef CGBL_MAPPER_1_H_
#define CGBL_MAPPER_1_H_

#include "cartridge.h"

#define CGBL_MAPPER_1_BANK_HIGH_BEGIN 0x4000
#define CGBL_MAPPER_1_BANK_HIGH_END 0x5FFF
#define CGBL_MAPPER_1_BANK_LOW_BEGIN 0x2000
#define CGBL_MAPPER_1_BANK_LOW_END 0x3FFF
#define CGBL_MAPPER_1_BANK_SELECT_BEGIN 0x6000
#define CGBL_MAPPER_1_BANK_SELECT_END 0x7FFF
#define CGBL_MAPPER_1_RAM_ENABLE_BEGIN 0x0000
#define CGBL_MAPPER_1_RAM_ENABLE_END 0x1FFF

uint8_t cgbl_mapper_1_read(uint16_t address);
void cgbl_mapper_1_reset(void);
void cgbl_mapper_1_write(uint16_t address, uint8_t data);

#endif /* CGBL_MAPPER_1_H_ */
