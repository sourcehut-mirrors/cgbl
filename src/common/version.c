/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "common.h"

static const cgbl_version_t VERSION = { CGBL_VERSION_MAJOR, CGBL_VERSION_MINOR, CGBL_VERSION_PATCH };

const cgbl_version_t *cgbl_version(void) {
    return &VERSION;
}
