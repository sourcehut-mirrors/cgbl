/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "client.h"
#include "debug.h"
#include "processor.h"

#define CGBL_TRACE_ERR(_FORMAT_, ...) \
    cgbl_debug_trace(CGBL_LEVEL_ERROR, _FORMAT_, ##__VA_ARGS__)
#define CGBL_TRACE_WARN(_FORMAT_, ...) \
    cgbl_debug_trace(CGBL_LEVEL_WARNING, _FORMAT_, ##__VA_ARGS__)
#define CGBL_TRACE_INFO(_FORMAT_, ...) \
    cgbl_debug_trace(CGBL_LEVEL_INFORMATION, _FORMAT_, ##__VA_ARGS__)
#define CGBL_TRACE_VERB(_FORMAT_, ...) \
    cgbl_debug_trace(CGBL_LEVEL_VERBOSE, _FORMAT_, ##__VA_ARGS__)

typedef enum {
    CGBL_COMMAND_EXIT = 0,
    CGBL_COMMAND_HELP,
    CGBL_COMMAND_PROCESSOR,
    CGBL_COMMAND_RESET,
    CGBL_COMMAND_RUN,
    CGBL_COMMAND_STEP,
    CGBL_COMMAND_VERSION,
    CGBL_COMMAND_MAX,
} cgbl_command_e;

typedef enum {
    CGBL_LEVEL_ERROR = 0,
    CGBL_LEVEL_WARNING,
    CGBL_LEVEL_INFORMATION,
    CGBL_LEVEL_VERBOSE,
    CGBL_LEVEL_PROMPT,
    CGBL_LEVEL_MAX,
} cgbl_level_e;

static const struct {
    uint8_t value;
    const char *name;
} MAPPER[] = {
    { 0, "MBC0" }, { 8, "MBC0" }, { 9, "MBC0" },
    { 1, "MBC1" }, { 2, "MBC1" }, { 3, "MBC1" },
    { 5, "MBC2" }, { 6, "MBC2" },
    { 15, "MBC3" }, { 16, "MBC3" }, { 17, "MBC3" }, { 18, "MBC3" },
    { 19, "MBC3" },
    { 25, "MBC5" }, { 26, "MBC5" }, { 27, "MBC5" }, { 28, "MBC5" },
    { 29, "MBC5" }, { 30, "MBC5" },
};

static const struct {
    uint8_t value;
    const char *name;
} MODE[] = {
    { 0x00, "DMG" }, { 0x80, "DMG" },
    { 0xC0, "CGB" },
};

static const struct {
    const char *name;
    const char *description;
    const char *usage;
    uint8_t length;
} OPTION[CGBL_COMMAND_MAX] = {
    { .name = "exit", .description = "Exit debug console", .usage = "exit", .length = 1, },
    { .name = "help", .description = "Display help information", .usage = "help", .length = 1, },
    { .name = "proc", .description = "Display processor information", .usage = "proc", .length = 1, },
    { .name = "rst", .description = "Reset bus", .usage = "rst", .length = 1, },
    { .name = "run", .description = "Run to breakpoint", .usage = "run [breakpoint]", .length = 2, },
    { .name = "step", .description = "Step to next instruction", .usage = "step [breakpoint]", .length = 2, },
    { .name = "ver", .description = "Display version information", .usage = "ver", .length = 1, },
};

static const uint16_t RAM[] = {
    1, 1, 1, 4, 16, 8,
};

static const uint16_t ROM[] = {
    2, 4, 8, 16, 32, 64, 128, 256, 512,
};

static const struct {
    const char *begin;
    const char *end;
} TRACE[CGBL_LEVEL_MAX] = {
    { "\033[31m", "\033[m", },
    { "\033[93m", "\033[m", },
    { "\033[0m", "\033[m", },
    { "\033[37m", "\033[m", },
    { "\033[32m", "\033[m", },
};

static struct {
    char prompt[32];
    const char *path;
    cgbl_bank_t *ram;
    const cgbl_bank_t *rom;
} debug = {};

static inline void cgbl_debug_trace(cgbl_level_e level, const char *const format, ...)
{
    char buffer[128] = {};
    FILE *stream = NULL;
    va_list arguments;
    va_start(arguments, format);
    vsnprintf(buffer, sizeof (buffer), format, arguments);
    va_end(arguments);
    switch (level)
    {
        case CGBL_LEVEL_ERROR:
        case CGBL_LEVEL_WARNING:
            stream = stderr;
            break;
        default:
            stream = stdout;
            break;
    }
    fprintf(stream, "%s%s%s", TRACE[level].begin, buffer, TRACE[level].end);
}

static cgbl_error_e cgbl_debug_command_exit(const char **const arguments, uint8_t length)
{
    CGBL_TRACE_INFO("Exiting\n");
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_debug_command_help(const char **const arguments, uint8_t length)
{
    for (cgbl_command_e command = 0; command < CGBL_COMMAND_MAX; ++command)
    {
        char buffer[22] = {};
        snprintf(buffer, sizeof (buffer), "%s", OPTION[command].usage);
        for (uint32_t offset = strlen(buffer); offset < sizeof (buffer); ++offset)
        {
            buffer[offset] = (offset == (sizeof (buffer) - 1)) ? '\0' : ' ';
        }
        CGBL_TRACE_INFO("%s%s\n", buffer, OPTION[command].description);
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_debug_command_processor(const char **const arguments, uint8_t length)
{
    const cgbl_register_e reg[] = {
        CGBL_REGISTER_PC, CGBL_REGISTER_SP, CGBL_REGISTER_AF, CGBL_REGISTER_BC,
        CGBL_REGISTER_DE, CGBL_REGISTER_HL,
    };
    cgbl_error_e result = CGBL_SUCCESS;
    for (uint32_t index = 0; index < CGBL_LENGTH(reg); ++index)
    {
        cgbl_register_t value = {};
        if ((result = cgbl_processor_register_read(reg[index], &value)) != CGBL_SUCCESS)
        {
            break;
        }
        switch (reg[index])
        {
            case CGBL_REGISTER_AF:
                CGBL_TRACE_INFO("AF: %04X (A: %02X, F: %02X) [%c%c%c%c]\n", value.word, value.high, value.low,
                    value.carry ? 'C' : '-', value.half_carry ? 'H' : '-', value.negative ? 'N' : '-', value.zero ? 'Z' : '-');
                break;
            case CGBL_REGISTER_BC:
                CGBL_TRACE_INFO("BC: %04X (B: %02X, C: %02X)\n", value.word, value.high, value.low);
                break;
            case CGBL_REGISTER_DE:
                CGBL_TRACE_INFO("DE: %04X (D: %02X, E: %02X)\n", value.word, value.high, value.low);
                break;
            case CGBL_REGISTER_HL:
                CGBL_TRACE_INFO("HL: %04X (H: %02X, L: %02X)\n", value.word, value.high, value.low);
                break;
            case CGBL_REGISTER_PC:
                CGBL_TRACE_INFO("PC: %04X\n", value.word);
                break;
            case CGBL_REGISTER_SP:
                CGBL_TRACE_INFO("SP: %04X\n", value.word);
                break;
            default:
                break;
        }
    }
    return result;
}

static cgbl_error_e cgbl_debug_command_reset(const char **const arguments, uint8_t length)
{
    return cgbl_bus_reset(debug.rom, debug.ram);
}

static cgbl_error_e cgbl_debug_command_run(const char **const arguments, uint8_t length)
{
    uint16_t breakpoint = 0xFFFF;
    cgbl_error_e result = CGBL_SUCCESS;
    if (length == OPTION[CGBL_COMMAND_RUN].length)
    {
        breakpoint = strtol(arguments[1], NULL, 16);
    }
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
        if ((result = cgbl_bus_run(breakpoint)) != CGBL_SUCCESS)
        {
            if (result != CGBL_QUIT)
            {
                if (result == CGBL_BREAKPOINT)
                {
                    CGBL_TRACE_WARN("Breakpoint: %04X\n", breakpoint);
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

static cgbl_error_e cgbl_debug_command_step(const char **const arguments, uint8_t length)
{
    uint16_t breakpoint = 0xFFFF;
    cgbl_error_e result = CGBL_SUCCESS;
    if (length == OPTION[CGBL_COMMAND_STEP].length)
    {
        breakpoint = strtol(arguments[1], NULL, 16);
    }
    /* TODO: DISASSEMBLE INSTRUCTION */
    if ((result = cgbl_client_poll()) != CGBL_SUCCESS)
    {
        if (result == CGBL_QUIT)
        {
            result = CGBL_SUCCESS;
        }
        return result;
    }
    if ((result = cgbl_bus_step(breakpoint)) != CGBL_SUCCESS)
    {
        if (result == CGBL_BREAKPOINT)
        {
            CGBL_TRACE_WARN("Breakpoint: %04X\n", breakpoint);
            result = CGBL_SUCCESS;
        }
        return result;
    }
    if ((result = cgbl_client_sync()) != CGBL_SUCCESS)
    {
        return result;
    }
    return result;
}

static cgbl_error_e cgbl_debug_command_version(const char **const arguments, uint8_t length)
{
    const cgbl_version_t *const version = cgbl_version();
    CGBL_TRACE_INFO("%u.%u-%x\n", version->major, version->minor, version->patch);
    return CGBL_SUCCESS;
}

static cgbl_error_e (*const COMMAND[CGBL_COMMAND_MAX])(const char **const arguments, uint8_t length) = {
    cgbl_debug_command_exit,
    cgbl_debug_command_help,
    cgbl_debug_command_processor,
    cgbl_debug_command_reset,
    cgbl_debug_command_run,
    cgbl_debug_command_step,
    cgbl_debug_command_version,
};

static char **cgbl_debug_completion(const char *text, int start, int end)
{
    rl_attempted_completion_over = 1;
    return NULL;
}

static void cgbl_debug_header_checksum(void)
{
    uint8_t value = debug.rom->data[0x014D];
    CGBL_TRACE_VERB("014D | Checksum -- %02X\n", value);
}

static void cgbl_debug_header_mapper(void)
{
    const char *name = NULL;
    uint8_t index = 0, value = debug.rom->data[0x0147];
    for (; index < CGBL_LENGTH(MAPPER); ++index)
    {
        if (MAPPER[index].value == value)
        {
            name = MAPPER[index].name;
            break;
        }
    }
    CGBL_TRACE_VERB("0147 | Mapper   -- %02X (%s)\n", value, name ? name : "????");
}

static void cgbl_debug_header_mode(void)
{
    const char *name = NULL;
    uint8_t index = 0, value = debug.rom->data[0x0143];
    for (; index < CGBL_LENGTH(MODE); ++index)
    {
        if (MODE[index].value == value)
        {
            name = MODE[index].name;
            break;
        }
    }
    CGBL_TRACE_VERB("0143 | Mode     -- %02X (%s)\n", value, name ? name : "???");
}

static void cgbl_debug_header_ram(void)
{
    uint8_t value = debug.rom->data[0x0149];
    uint16_t bank = 0;
    if (value < CGBL_LENGTH(RAM))
    {
        bank = RAM[value];
    }
    CGBL_TRACE_VERB("0149 | Ram      -- %02X (%u banks, %u bytes)\n", value, bank, bank * 0x2000U);
}

static void cgbl_debug_header_rom(void)
{
    uint8_t value = debug.rom->data[0x0148];
    uint16_t bank = 0;
    if (value < CGBL_LENGTH(ROM))
    {
        bank = ROM[value];
    }
    CGBL_TRACE_VERB("0148 | Rom      -- %02X (%u banks, %u bytes)\n", value, bank, bank * 0x4000U);
}

static void cgbl_debug_header_title(void)
{
    char title[12] = {};
    for (uint32_t index = 0; index < (sizeof (title) - 1); ++index)
    {
        char value = debug.rom->data[0x0134 + index];
        if (!value)
        {
            break;
        }
        title[index] = value;
    }
    if (!strlen(title))
    {
        strcpy(title, "UNDEFINED");
    }
    CGBL_TRACE_VERB("0134 | Title    -- %s\n", title);
}

static void cgbl_debug_header(void)
{
    const cgbl_version_t *const version = cgbl_version();
    CGBL_TRACE_VERB("CGBL %u.%u-%x\n", version->major, version->minor, version->patch);
    CGBL_TRACE_VERB("Path: %s\n", debug.path);
    if (debug.rom && debug.rom->data)
    {
        CGBL_TRACE_VERB("\n");
        cgbl_debug_header_title();
        cgbl_debug_header_mode();
        cgbl_debug_header_mapper();
        cgbl_debug_header_rom();
        cgbl_debug_header_ram();
        cgbl_debug_header_checksum();
    }
}

static const char *cgbl_debug_prompt(void)
{
    cgbl_register_t reg = {};
    memset(debug.prompt, 0, sizeof (debug.prompt));
    if (cgbl_processor_register_read(CGBL_REGISTER_PC, &reg) == CGBL_SUCCESS)
    {
        snprintf(debug.prompt, sizeof (debug.prompt), "\n%s%04X%s> ",
            TRACE[CGBL_LEVEL_PROMPT].begin, reg.word, TRACE[CGBL_LEVEL_PROMPT].end);
    }
    else
    {
        snprintf(debug.prompt, sizeof (debug.prompt), "\n%s\?\?\?\?%s> ",
            TRACE[CGBL_LEVEL_PROMPT].begin, TRACE[CGBL_LEVEL_PROMPT].end);
    }
    return debug.prompt;
}

cgbl_error_e cgbl_debug_entry(const char *const path, const cgbl_bank_t *const rom, cgbl_bank_t *const ram)
{
    cgbl_error_e result = CGBL_SUCCESS;
    rl_attempted_completion_function = cgbl_debug_completion;
    memset(&debug, 0, sizeof (debug));
    debug.path = path;
    debug.ram = ram;
    debug.rom = rom;
    cgbl_debug_header();
    for (;;)
    {
        char *input = NULL;
        cgbl_command_e command = CGBL_COMMAND_MAX;
        if ((input = readline(cgbl_debug_prompt())) && strlen(input))
        {
            uint8_t length = 0;
            char *argument = NULL;
            const char *arguments[4] = {};
            add_history(input);
            argument = strtok(input, " ");
            while (argument)
            {
                if (length >= 4)
                {
                    break;
                }
                arguments[length++] = argument;
                argument = strtok(NULL, " ");
            }
            for (command = 0; command < CGBL_COMMAND_MAX; ++command)
            {
                if (!strcmp(input, OPTION[command].name))
                {
                    break;
                }
            }
            if (command >= CGBL_COMMAND_MAX)
            {
                CGBL_TRACE_ERR("Command unsupported: \'%s\'\n", input);
                CGBL_TRACE_WARN("Type '%s' for more information\n", OPTION[CGBL_COMMAND_HELP].name);
            }
            else if (length > OPTION[command].length)
            {
                CGBL_TRACE_ERR("Command usage: %s\n", OPTION[command].usage);
                CGBL_TRACE_WARN("Type '%s' for more information\n", OPTION[CGBL_COMMAND_HELP].name);
            }
            else if ((result = COMMAND[command](arguments, length)) == CGBL_FAILURE)
            {
                CGBL_TRACE_ERR("Command failed: \'%s\'\n", input);
            }
        }
        free(input);
        if ((command == CGBL_COMMAND_EXIT) && (result == CGBL_SUCCESS))
        {
            break;
        }
    }
    return result;
}
