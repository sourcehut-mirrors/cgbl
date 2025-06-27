/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef CGBL_MAPPER_0_H_
#define CGBL_MAPPER_0_H_

#include "cartridge.h"

uint8_t cgbl_mapper_0_read(uint16_t address);
void cgbl_mapper_0_write(uint16_t address, uint8_t data);

#endif /* CGBL_MAPPER_0_H_ */
