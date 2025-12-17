/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "bootloader.h"
#include <string.h>

static const uint8_t BOOTROM[][CGBL_BOOTLOADER_ROM_WIDTH] = { {
#embed "bootloader/bootrom_0.bin"
                                                              },
                                                              {
#embed "bootloader/bootrom_1.bin"
                                                              } };

static struct {
    bool enabled;
} bootloader = {};

bool cgbl_bootloader_enabled(void) {
    return bootloader.enabled;
}

uint8_t cgbl_bootloader_read(uint16_t address) {
    uint8_t result = 0xFF;
    switch (address) {
    case CGBL_BOOTLOADER_ROM_0_BEGIN ... CGBL_BOOTLOADER_ROM_0_END:
        if (bootloader.enabled) {
            result = BOOTROM[0][address - CGBL_BOOTLOADER_ROM_0_BEGIN];
        }
        break;
    case CGBL_BOOTLOADER_ROM_1_BEGIN ... CGBL_BOOTLOADER_ROM_1_END:
        if (bootloader.enabled) {
            result = BOOTROM[1][address - CGBL_BOOTLOADER_ROM_1_BEGIN];
        }
        break;
    default:
        break;
    }
    return result;
}

void cgbl_bootloader_reset(void) {
    memset(&bootloader, 0, sizeof(bootloader));
    bootloader.enabled = true;
}

void cgbl_bootloader_write(uint16_t address, uint8_t data) {
    switch (address) {
    case CGBL_BOOTLOADER_DISABLE:
        if (bootloader.enabled && data) {
            bootloader.enabled = false;
        }
        break;
    default:
        break;
    }
}
