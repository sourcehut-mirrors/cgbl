/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "client.h"
#include "debug.h"
#include "processor.h"

#define TRACE_ERROR(_FORMAT_, ...) \
    cgbl_debug_trace(CGBL_LEVEL_ERROR, _FORMAT_, ##__VA_ARGS__)
#define TRACE_WARNING(_FORMAT_, ...) \
    cgbl_debug_trace(CGBL_LEVEL_WARNING, _FORMAT_, ##__VA_ARGS__)
#define TRACE_INFORMATION(_FORMAT_, ...) \
    cgbl_debug_trace(CGBL_LEVEL_INFORMATION, _FORMAT_, ##__VA_ARGS__)
#define TRACE_VERBOSE(_FORMAT_, ...) \
    cgbl_debug_trace(CGBL_LEVEL_VERBOSE, _FORMAT_, ##__VA_ARGS__)

typedef enum {
    CGBL_COMMAND_EXIT = 0,
    CGBL_COMMAND_DISASSEMBLE,
    CGBL_COMMAND_HELP,
    CGBL_COMMAND_INTERRUPT,
    CGBL_COMMAND_MEMORY_READ,
    CGBL_COMMAND_MEMORY_WRITE,
    CGBL_COMMAND_PROCESSOR,
    CGBL_COMMAND_REGISTER_READ,
    CGBL_COMMAND_REGISTER_WRITE,
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

static const char *INTERRUPT[CGBL_INTERRUPT_MAX] = {
    "vblk", "lcdc", "tmr", "ser",
    "joy",
};

static const struct {
    const char *name;
    const char *description;
    const char *usage;
    uint8_t min;
    uint8_t max;
} OPTION[CGBL_COMMAND_MAX] = {
    { .name = "exit", .description = "Exit debug console", .usage = "exit", .min = 1, .max = 1, },
    { .name = "dasm", .description = "Disassemble instructions", .usage = "dasm addr [len]", .min = 2, .max = 3, },
    { .name = "help", .description = "Display help information", .usage = "help", .min = 1, .max = 1, },
    { .name = "itr", .description = "Interrupt bus", .usage = "itr int", .min = 2, .max = 2, },
    { .name = "memr", .description = "Read data from memory", .usage = "memr addr [len]", .min = 2, .max = 3, },
    { .name = "memw", .description = "Write data to memory", .usage = "memw addr data [len]", .min = 3, .max = 4, },
    { .name = "proc", .description = "Display processor information", .usage = "proc", .min = 1, .max = 1, },
    { .name = "regr", .description = "Read data from register", .usage = "regr reg", .min = 2, .max = 2, },
    { .name = "regw", .description = "Write data to register", .usage = "regw reg data", .min = 3, .max = 3, },
    { .name = "rst", .description = "Reset bus", .usage = "rst", .min = 1, .max = 1, },
    { .name = "run", .description = "Run to breakpoint", .usage = "run [bp]", .min = 1, .max = 2, },
    { .name = "step", .description = "Step to next instruction", .usage = "step [bp]", .min = 1, .max = 2, },
    { .name = "ver", .description = "Display version information", .usage = "ver", .min = 1, .max = 1, },
};

static const char *REGISTER[CGBL_REGISTER_MAX] = {
    "a", "af", "b", "bc",
    "c", "d", "de", "e",
    "f", "h", "hl", "l",
    "pc", "sp",
};

static const struct {
    const char *begin;
    const char *end;
} TRACE[CGBL_LEVEL_MAX] = {
    { "\x1B[31m", "\x1B[m", },
    { "\x1B[93m", "\x1B[m", },
    { "\x1B[0m", "\x1B[m", },
    { "\x1B[37m", "\x1B[m", },
    { "\x1B[32m", "\x1B[m", },
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

static inline void cgbl_debug_disassemble(uint16_t address, uint16_t length)
{
    if (length > 1)
    {
        /* TODO */
    }
    else
    {
        /* TODO */
    }
}

static inline cgbl_interrupt_e cgbl_debug_interrupt(const char *const name)
{
    cgbl_interrupt_e result = 0;
    for (; result < CGBL_INTERRUPT_MAX; ++result)
    {
        if (!strcmp(name, INTERRUPT[result]))
        {
            break;
        }
    }
    return result;
}

static inline cgbl_register_e cgbl_debug_register(const char *const name)
{
    cgbl_register_e result = 0;
    for (; result < CGBL_REGISTER_MAX; ++result)
    {
        if (!strcmp(name, REGISTER[result]))
        {
            break;
        }
    }
    return result;
}

static inline cgbl_error_e cgbl_debug_register_data(const char *const name, cgbl_register_t *const data)
{
    cgbl_register_e reg = cgbl_debug_register(name);
    if (reg < CGBL_REGISTER_MAX)
    {
        return cgbl_processor_register_read(reg, data);
    }
    return CGBL_FAILURE;
}

static cgbl_error_e cgbl_debug_command_exit(const char **const arguments, uint8_t length)
{
    TRACE_INFORMATION("Exiting\n");
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_debug_command_disassemble(const char **const arguments, uint8_t length)
{
    uint16_t address = 0, offset = 1;
    cgbl_register_t data = {};
    if (cgbl_debug_register_data(arguments[1], &data) == CGBL_SUCCESS)
    {
        address = data.word;
    }
    else
    {
        address = strtol(arguments[1], NULL, 16);
    }
    if (length == OPTION[CGBL_COMMAND_DISASSEMBLE].max)
    {
        offset = strtol(arguments[length - 1], NULL, 16);
    }
    cgbl_debug_disassemble(address, offset);
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
        TRACE_INFORMATION("%s%s\n", buffer, OPTION[command].description);
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_debug_command_interrupt(const char **const arguments, uint8_t length)
{
    cgbl_interrupt_e interrupt = cgbl_debug_interrupt(arguments[1]);
    if (interrupt >= CGBL_INTERRUPT_MAX)
    {
        return CGBL_FAILURE;
    }
    cgbl_processor_interrupt(interrupt);
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_debug_command_memory_read(const char **const arguments, uint8_t length)
{
    uint16_t address = 0, offset = 1;
    cgbl_register_t data = {};
    if (cgbl_debug_register_data(arguments[1], &data) == CGBL_SUCCESS)
    {
        address = data.word;
    }
    else
    {
        address = strtol(arguments[1], NULL, 16);
    }
    if (length == OPTION[CGBL_COMMAND_MEMORY_READ].max)
    {
        offset = strtol(arguments[length - 1], NULL, 16);
    }
    if (offset > 1)
    {
        /* TODO */
    }
    else
    {
        TRACE_INFORMATION("%02X\n", cgbl_bus_read(address));
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_debug_command_memory_write(const char **const arguments, uint8_t length)
{
    uint16_t address = 0, offset = 1;
    cgbl_register_t data = {};
    if (cgbl_debug_register_data(arguments[1], &data) == CGBL_SUCCESS)
    {
        address = data.word;
    }
    else
    {
        address = strtol(arguments[1], NULL, 16);
    }
    data.low = strtol(arguments[2], NULL, 16);
    if (length == OPTION[CGBL_COMMAND_MEMORY_WRITE].max)
    {
        offset = strtol(arguments[length - 1], NULL, 16);
    }
    for (uint32_t index = address; index < address + offset; ++index)
    {
        cgbl_bus_write(index, data.low);
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
        cgbl_register_t data = {};
        if ((result = cgbl_processor_register_read(reg[index], &data)) != CGBL_SUCCESS)
        {
            break;
        }
        switch (reg[index])
        {
            case CGBL_REGISTER_AF:
                TRACE_INFORMATION("AF: %04X (A: %02X, F: %02X) [%c%c%c%c]\n", data.word, data.high, data.low,
                    data.carry ? 'C' : '-', data.half_carry ? 'H' : '-', data.negative ? 'N' : '-', data.zero ? 'Z' : '-');
                break;
            case CGBL_REGISTER_BC:
                TRACE_INFORMATION("BC: %04X (B: %02X, C: %02X)\n", data.word, data.high, data.low);
                break;
            case CGBL_REGISTER_DE:
                TRACE_INFORMATION("DE: %04X (D: %02X, E: %02X)\n", data.word, data.high, data.low);
                break;
            case CGBL_REGISTER_HL:
                TRACE_INFORMATION("HL: %04X (H: %02X, L: %02X)\n", data.word, data.high, data.low);
                break;
            case CGBL_REGISTER_PC:
                TRACE_INFORMATION("PC: %04X\n", data.word);
                break;
            case CGBL_REGISTER_SP:
                TRACE_INFORMATION("SP: %04X\n", data.word);
                break;
            default:
                break;
        }
    }
    return result;
}

static cgbl_error_e cgbl_debug_command_register_read(const char **const arguments, uint8_t length)
{
    cgbl_error_e result =CGBL_SUCCESS;
    cgbl_register_e reg = cgbl_debug_register(arguments[1]);
    cgbl_register_t data = {};
    if ((result = cgbl_processor_register_read(reg, &data)) == CGBL_SUCCESS)
    {
        switch (reg)
        {
            case CGBL_REGISTER_AF:
                TRACE_INFORMATION("%04X [%c%c%c%c]\n", data.word,
                    data.carry ? 'C' : '-', data.half_carry ? 'H' : '-', data.negative ? 'N' : '-', data.zero ? 'Z' : '-');
                break;
            case CGBL_REGISTER_BC:
            case CGBL_REGISTER_DE:
            case CGBL_REGISTER_HL:
            case CGBL_REGISTER_PC:
            case CGBL_REGISTER_SP:
                TRACE_INFORMATION("%04X\n", data.word);
                break;
            case CGBL_REGISTER_F:
                TRACE_INFORMATION("%02X [%c%c%c%c]\n", data.low,
                    data.carry ? 'C' : '-', data.half_carry ? 'H' : '-', data.negative ? 'N' : '-', data.zero ? 'Z' : '-');
                break;
            default:
                TRACE_INFORMATION("%02X\n", data.low);
                break;
        }
    }
    return result;
}

static cgbl_error_e cgbl_debug_command_register_write(const char **const arguments, uint8_t length)
{
    cgbl_register_e reg = cgbl_debug_register(arguments[1]);
    cgbl_register_t data = { .word = strtol(arguments[2], NULL, 16) };
    return cgbl_processor_register_write(reg, &data);
}

static cgbl_error_e cgbl_debug_command_reset(const char **const arguments, uint8_t length)
{
    return cgbl_bus_reset(debug.rom, debug.ram);
}

static cgbl_error_e cgbl_debug_command_run(const char **const arguments, uint8_t length)
{
    uint16_t breakpoint = 0xFFFF;
    cgbl_error_e result = CGBL_SUCCESS;
    if (length == OPTION[CGBL_COMMAND_RUN].max)
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
                    TRACE_WARNING("Breakpoint: %04X\n", breakpoint);
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
    cgbl_register_t reg = {};
    if (length == OPTION[CGBL_COMMAND_STEP].max)
    {
        breakpoint = strtol(arguments[1], NULL, 16);
    }
    if (cgbl_processor_register_read(CGBL_REGISTER_PC, &reg) == CGBL_SUCCESS)
    {
        cgbl_debug_disassemble(reg.word, 1);
    }
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
            TRACE_WARNING("Breakpoint: %04X\n", breakpoint);
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
    TRACE_INFORMATION("%u.%u-%x\n", version->major, version->minor, version->patch);
    return CGBL_SUCCESS;
}

static cgbl_error_e (*const COMMAND[CGBL_COMMAND_MAX])(const char **const arguments, uint8_t length) = {
    cgbl_debug_command_exit,
    cgbl_debug_command_disassemble,
    cgbl_debug_command_help,
    cgbl_debug_command_interrupt,
    cgbl_debug_command_memory_read,
    cgbl_debug_command_memory_write,
    cgbl_debug_command_processor,
    cgbl_debug_command_register_read,
    cgbl_debug_command_register_write,
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

static void cgbl_debug_header(void)
{
    const cgbl_version_t *const version = cgbl_version();
    TRACE_VERBOSE("CGBL %u.%u-%x\n", version->major, version->minor, version->patch);
    TRACE_VERBOSE("Path: %s\n", (debug.path && strlen(debug.path)) ? debug.path : "Undefined");
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
    memset(&debug, 0, sizeof (debug));
    debug.path = path;
    debug.ram = ram;
    debug.rom = rom;
    cgbl_debug_header();
    for (;;)
    {
        char *input = NULL;
        cgbl_command_e command = CGBL_COMMAND_MAX;
        rl_attempted_completion_function = cgbl_debug_completion;
        if ((input = readline(cgbl_debug_prompt())) && strlen(input))
        {
            uint8_t length = 0;
            char *argument = NULL;
            const char *arguments[5] = {};
            add_history(input);
            argument = strtok(input, " ");
            while (argument)
            {
                if (length >= CGBL_LENGTH(arguments))
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
                TRACE_ERROR("Command unsupported: \'%s\'\n", input);
                TRACE_WARNING("Type '%s' for more information\n", OPTION[CGBL_COMMAND_HELP].name);
            }
            else if ((length < OPTION[command].min) || (length > OPTION[command].max))
            {
                TRACE_ERROR("Command usage: %s\n", OPTION[command].usage);
                TRACE_WARNING("Type '%s' for more information\n", OPTION[CGBL_COMMAND_HELP].name);
            }
            else if ((result = COMMAND[command](arguments, length)) == CGBL_FAILURE)
            {
                TRACE_ERROR("Command failed: \'%s\'\n", input);
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
