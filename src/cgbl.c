/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include "cartridge.h"
#include "client.h"
#include "debug.h"

static struct {
    const cgbl_option_t *option;
    struct {
        char *path;
        cgbl_bank_t bank;
    } ram;
    struct {
        cgbl_bank_t bank;
    } rom;
} cgbl = {};

static cgbl_error_e cgbl_ram_load(const char *const path)
{
    cgbl_error_e result = CGBL_SUCCESS;
    if (path)
    {
        if ((result = cgbl_string_allocate(&cgbl.ram.path, "%s.ram", path)) == CGBL_SUCCESS)
        {
            if (cgbl_file_exists(cgbl.ram.path))
            {
                result = cgbl_file_read(cgbl.ram.path, &cgbl.ram.bank.data, &cgbl.ram.bank.length);
            }
            else
            {
                cgbl.ram.bank.length = 17 * CGBL_CARTRIDGE_RAM_WIDTH;
                result = cgbl_buffer_allocate(&cgbl.ram.bank.data, cgbl.ram.bank.length);
            }
        }
    }
    return result;
}

static cgbl_error_e cgbl_ram_save(void)
{
    cgbl_error_e result = CGBL_SUCCESS;
    if (cgbl.ram.path)
    {
        result = cgbl_file_write(cgbl.ram.path, cgbl.ram.bank.data, cgbl.ram.bank.length);
    }
    return result;
}

static void cgbl_ram_unload(void)
{
    if (cgbl.ram.path)
    {
        cgbl_string_free(cgbl.ram.path);
    }
    if (cgbl.ram.bank.data)
    {
        cgbl_buffer_free(cgbl.ram.bank.data);
    }
}

static cgbl_error_e cgbl_rom_load(const char *const path)
{
    cgbl_error_e result = CGBL_SUCCESS;
    if (path)
    {
        result = cgbl_file_read(path, &cgbl.rom.bank.data, &cgbl.rom.bank.length);
    }
    return result;
}

static void cgbl_rom_unload(void)
{
    if (cgbl.rom.bank.data)
    {
        cgbl_buffer_free(cgbl.rom.bank.data);
    }
}

static cgbl_error_e cgbl_run_debug(void)
{
    cgbl_error_e result = CGBL_SUCCESS;
    if ((result = cgbl_client_sync()) == CGBL_SUCCESS)
    {
        result = cgbl_debug_entry(&cgbl.rom.bank, &cgbl.ram.bank);
    }
    return result;
}

static cgbl_error_e cgbl_run_release(void)
{
    cgbl_error_e result = CGBL_SUCCESS;
    for (;;)
    {
        if ((result = cgbl_client_poll()) != CGBL_SUCCESS)
        {
            if (result == CGBL_QUIT)
            {
                result = CGBL_SUCCESS;
            }
            break;
        }
        if ((result = cgbl_bus_run(0xFFFF)) != CGBL_SUCCESS)
        {
            if (result != CGBL_QUIT)
            {
                if (result == CGBL_BREAKPOINT)
                {
                    result = CGBL_SUCCESS;
                }
                break;
            }
        }
        if ((result = cgbl_client_sync()) != CGBL_SUCCESS)
        {
            break;
        }
    }
    return result;
}

static cgbl_error_e cgbl_run(void)
{
    cgbl_error_e result = CGBL_SUCCESS;
    if ((result = cgbl_bus_reset(&cgbl.rom.bank, &cgbl.ram.bank)) == CGBL_SUCCESS)
    {
        if ((result = cgbl_client_create(cgbl.option->scale, cgbl.option->fullscreen)) == CGBL_SUCCESS)
        {
            result = cgbl.option->debug ? cgbl_run_debug() : cgbl_run_release();
            cgbl_client_destroy();
        }
    }
    return result;
}

cgbl_error_e cgbl_entry(const char *const path, const cgbl_option_t *const option)
{
    cgbl_error_e result = CGBL_SUCCESS;
    memset(&cgbl, 0, sizeof (cgbl));
    cgbl.option = option;
    if ((result = cgbl_rom_load(path)) == CGBL_SUCCESS)
    {
        if ((result = cgbl_ram_load(path)) == CGBL_SUCCESS)
        {
            if ((result = cgbl_run()) == CGBL_SUCCESS)
            {
                result = cgbl_ram_save();
            }
            cgbl_ram_unload();
        }
        cgbl_rom_unload();
    }
    return result;
}
