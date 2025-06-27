/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include "bootloader.h"
#include "cartridge.h"

static struct {
    struct {
        uint8_t ram[CGBL_MEMORY_RAM_HIGH_WIDTH];
    } high;
    struct {
        union {
            struct {
                uint8_t select : 3;
            };
            uint8_t raw;
        } bank;
        uint8_t ram[8][CGBL_MEMORY_RAM_WORK_WIDTH];
    } work;
} memory = {};

uint8_t cgbl_memory_read(uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case CGBL_BOOTLOADER_ROM_0_BEGIN ... CGBL_BOOTLOADER_ROM_0_END:
        case CGBL_BOOTLOADER_ROM_1_BEGIN ... CGBL_BOOTLOADER_ROM_1_END:
            if (cgbl_bootloader_enabled())
            {
                result = cgbl_bootloader_read(address);
            }
            else
            {
                result = cgbl_cartridge_read(address);
            }
            break;
        case CGBL_MEMORY_RAM_ECHO_0_BEGIN ... CGBL_MEMORY_RAM_ECHO_0_END:
            result = memory.work.ram[0][address - CGBL_MEMORY_RAM_ECHO_0_BEGIN];
            break;
        case CGBL_MEMORY_RAM_ECHO_1_BEGIN ... CGBL_MEMORY_RAM_ECHO_1_END:
            if ((cgbl_bus_mode() == CGBL_MODE_CGB) && memory.work.bank.select)
            {
                result = memory.work.ram[memory.work.bank.select][address - CGBL_MEMORY_RAM_ECHO_1_BEGIN];
            }
            else
            {
                result = memory.work.ram[1][address - CGBL_MEMORY_RAM_ECHO_1_BEGIN];
            }
            break;
        case CGBL_MEMORY_RAM_HIGH_BEGIN ... CGBL_MEMORY_RAM_HIGH_END:
            result = memory.high.ram[address - CGBL_MEMORY_RAM_HIGH_BEGIN];
            break;
        case CGBL_MEMORY_RAM_UNUSED_BEGIN ... CGBL_MEMORY_RAM_UNUSED_END:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                result = (address & 0xF0) | ((address & 0xF0) >> 4);
            }
            else
            {
                result = 0;
            }
            break;
        case CGBL_MEMORY_RAM_WORK_0_BEGIN ... CGBL_MEMORY_RAM_WORK_0_END:
            result = memory.work.ram[0][address - CGBL_MEMORY_RAM_WORK_0_BEGIN];
            break;
        case CGBL_MEMORY_RAM_WORK_1_BEGIN ... CGBL_MEMORY_RAM_WORK_1_END:
            if ((cgbl_bus_mode() == CGBL_MODE_CGB) && memory.work.bank.select)
            {
                result = memory.work.ram[memory.work.bank.select][address - CGBL_MEMORY_RAM_WORK_1_BEGIN];
            }
            else
            {
                result = memory.work.ram[1][address - CGBL_MEMORY_RAM_WORK_1_BEGIN];
            }
            break;
        case CGBL_MEMORY_RAM_WORK_SELECT:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                result = memory.work.bank.raw;
            }
            break;
        default:
            result = cgbl_cartridge_read(address);
            break;
    }
    return result;
}

cgbl_error_e cgbl_memory_reset(const cgbl_bank_t *const rom, cgbl_bank_t *const ram)
{
    cgbl_error_e result = CGBL_SUCCESS;
    memset(&memory, 0, sizeof (memory));
    memory.work.bank.raw = 0xF8;
    if ((result = cgbl_cartridge_reset(rom, ram)) == CGBL_SUCCESS)
    {
        cgbl_bootloader_reset();
    }
    return result;
}

void cgbl_memory_write(uint16_t address, uint8_t data)
{
    switch (address)
    {
        case CGBL_BOOTLOADER_DISABLE:
            cgbl_bootloader_write(address, data);
            break;
        case CGBL_MEMORY_RAM_ECHO_0_BEGIN ... CGBL_MEMORY_RAM_ECHO_0_END:
            memory.work.ram[0][address - CGBL_MEMORY_RAM_ECHO_0_BEGIN] = data;
            break;
        case CGBL_MEMORY_RAM_ECHO_1_BEGIN ... CGBL_MEMORY_RAM_ECHO_1_END:
            if ((cgbl_bus_mode() == CGBL_MODE_CGB) && memory.work.bank.select)
            {
                memory.work.ram[memory.work.bank.select][address - CGBL_MEMORY_RAM_ECHO_1_BEGIN] = data;
            }
            else
            {
                memory.work.ram[1][address - CGBL_MEMORY_RAM_ECHO_1_BEGIN] = data;
            }
            break;
        case CGBL_MEMORY_RAM_HIGH_BEGIN ... CGBL_MEMORY_RAM_HIGH_END:
            memory.high.ram[address - CGBL_MEMORY_RAM_HIGH_BEGIN] = data;
            break;
        case CGBL_MEMORY_RAM_UNUSED_BEGIN ... CGBL_MEMORY_RAM_UNUSED_END:
            break;
        case CGBL_MEMORY_RAM_WORK_0_BEGIN ... CGBL_MEMORY_RAM_WORK_0_END:
            memory.work.ram[0][address - CGBL_MEMORY_RAM_WORK_0_BEGIN] = data;
            break;
        case CGBL_MEMORY_RAM_WORK_1_BEGIN ... CGBL_MEMORY_RAM_WORK_1_END:
            if ((cgbl_bus_mode() == CGBL_MODE_CGB) && memory.work.bank.select)
            {
                memory.work.ram[memory.work.bank.select][address - CGBL_MEMORY_RAM_WORK_1_BEGIN] = data;
            }
            else
            {
                memory.work.ram[1][address - CGBL_MEMORY_RAM_WORK_1_BEGIN] = data;
            }
            break;
        case CGBL_MEMORY_RAM_WORK_SELECT:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                memory.work.bank.raw = (data & 7) | 0xF8;
            }
            break;
        default:
            cgbl_cartridge_write(address, data);
            break;
    }
}
