/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef CGBL_DEBUG_H_
#define CGBL_DEBUG_H_

#include "bus.h"

cgbl_error_e cgbl_debug_entry(const char *const path, const cgbl_bank_t *const rom, cgbl_bank_t *const ram);

#endif /* CGBL_DEBUG_H_ */
