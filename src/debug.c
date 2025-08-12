/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include "bus.h"
#include "client.h"
#include "debug.h"

static struct {
    cgbl_bank_t *ram;
    const cgbl_bank_t *rom;
} debug = {};

cgbl_error_e cgbl_debug_entry(const cgbl_bank_t *const rom, cgbl_bank_t *const ram)
{
    cgbl_error_e result = CGBL_SUCCESS;
    memset(&debug, 0, sizeof (debug));
    debug.ram = ram;
    debug.rom = rom;
    /* TODO */
    result = CGBL_ERROR("Debug mode unimplemented");
    /* ---- */
    return result;
}
