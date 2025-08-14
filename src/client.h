/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef CGBL_CLIENT_H_
#define CGBL_CLIENT_H_

#include "common.h"

#define CGBL_CLIENT_FRAME_DURATION (1000.f / (float)CGBL_CLIENT_FRAME_RATE)
#define CGBL_CLIENT_FRAME_RATE 60
#define CGBL_CLIENT_SCALE_MAX 8
#define CGBL_CLIENT_SCALE_MIN 1
#define CGBL_CLIENT_VSYNC true

cgbl_error_e cgbl_client_create(uint8_t scale, bool fullscreen);
void cgbl_client_destroy(void);
cgbl_error_e cgbl_client_poll(void);
cgbl_error_e cgbl_client_sync(void);

#endif /* CGBL_CLIENT_H_ */
