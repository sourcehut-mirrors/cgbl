/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include "audio.h"
#include "bootloader.h"
#include "cartridge.h"
#include "infrared.h"
#include "input.h"
#include "processor.h"
#include "serial.h"
#include "timer.h"
#include "video.h"

static struct {
    union {
        struct {
            uint8_t : 2;
            uint8_t dmg : 1;
        };
        uint8_t raw;
    } mode;
    union {
        struct {
            uint8_t dmg : 1;
        };
        uint8_t raw;
    } priority;
    union {
        struct {
            uint8_t armed : 1;
            uint8_t : 6;
            uint8_t doubled : 1;
        };
        uint8_t raw;
    } speed;
} bus = {};

cgbl_mode_e cgbl_bus_mode(void)
{
    return bus.mode.dmg ? CGBL_MODE_DMG : CGBL_MODE_CGB;
}

cgbl_priority_e cgbl_bus_priority(void)
{
    return bus.priority.dmg ? CGBL_PRIORITY_DMG : CGBL_PRIORITY_CGB;
}

uint8_t cgbl_bus_read(uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case CGBL_AUDIO_CHANNEL_1_ENVELOPE:
        case CGBL_AUDIO_CHANNEL_1_FREQUENCY_HIGH:
        case CGBL_AUDIO_CHANNEL_1_FREQUENCY_LOW:
        case CGBL_AUDIO_CHANNEL_1_LENGTH:
        case CGBL_AUDIO_CHANNEL_1_SWEEP:
        case CGBL_AUDIO_CHANNEL_2_ENVELOPE:
        case CGBL_AUDIO_CHANNEL_2_FREQUENCY_HIGH:
        case CGBL_AUDIO_CHANNEL_2_FREQUENCY_LOW:
        case CGBL_AUDIO_CHANNEL_2_LENGTH:
        case CGBL_AUDIO_CHANNEL_3_CONTROL:
        case CGBL_AUDIO_CHANNEL_3_FREQUENCY_HIGH:
        case CGBL_AUDIO_CHANNEL_3_FREQUENCY_LOW:
        case CGBL_AUDIO_CHANNEL_3_LENGTH:
        case CGBL_AUDIO_CHANNEL_3_LEVEL:
        case CGBL_AUDIO_CHANNEL_4_CONTROL:
        case CGBL_AUDIO_CHANNEL_4_ENVELOPE:
        case CGBL_AUDIO_CHANNEL_4_FREQUENCY:
        case CGBL_AUDIO_CHANNEL_4_LENGTH:
        case CGBL_AUDIO_CONTROL:
        case CGBL_AUDIO_MIXER:
        case CGBL_AUDIO_RAM_BEGIN ... CGBL_AUDIO_RAM_END:
        case CGBL_AUDIO_VOLUME:
            result = cgbl_audio_read(address);
            break;
        case CGBL_BUS_MODE:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                result = bus.mode.raw;
            }
            break;
        case CGBL_BUS_PRIORITY:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                result = bus.priority.raw;
            }
            break;
        case CGBL_BUS_SPEED:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                result = bus.speed.raw;
            }
            break;
        case CGBL_INFRARED_CONTROL:
            result = cgbl_infrared_read(address);
            break;
        case CGBL_INPUT_STATE:
            result = cgbl_input_read(address);
            break;
        case CGBL_PROCESSOR_INTERRUPT_ENABLE:
        case CGBL_PROCESSOR_INTERRUPT_FLAG:
            result = cgbl_processor_read(address);
            break;
        case CGBL_SERIAL_CONTROL:
        case CGBL_SERIAL_DATA:
            result = cgbl_serial_read(address);
            break;
        case CGBL_TIMER_CONTROL:
        case CGBL_TIMER_COUNTER:
        case CGBL_TIMER_DIVIDER:
        case CGBL_TIMER_MODULO:
            result = cgbl_timer_read(address);
            break;
        case CGBL_VIDEO_CONTROL:
        case CGBL_VIDEO_LINE_Y:
        case CGBL_VIDEO_LINE_Y_COINCIDENCE:
        case CGBL_VIDEO_PALETTE_BACKGROUND:
        case CGBL_VIDEO_PALETTE_BACKGROUND_CONTROL:
        case CGBL_VIDEO_PALETTE_BACKGROUND_DATA:
        case CGBL_VIDEO_PALETTE_OBJECT_0:
        case CGBL_VIDEO_PALETTE_OBJECT_1:
        case CGBL_VIDEO_PALETTE_OBJECT_CONTROL:
        case CGBL_VIDEO_PALETTE_OBJECT_DATA:
        case CGBL_VIDEO_RAM_BEGIN ... CGBL_VIDEO_RAM_END:
        case CGBL_VIDEO_RAM_OBJECT_BEGIN ... CGBL_VIDEO_RAM_OBJECT_END:
        case CGBL_VIDEO_RAM_SELECT:
        case CGBL_VIDEO_SCROLL_X:
        case CGBL_VIDEO_SCROLL_Y:
        case CGBL_VIDEO_STATUS:
        case CGBL_VIDEO_TRANSFER_CONTROL:
        case CGBL_VIDEO_TRANSFER_DESTINATION_HIGH:
        case CGBL_VIDEO_TRANSFER_DESTINATION_LOW:
        case CGBL_VIDEO_TRANSFER_OBJECTS:
        case CGBL_VIDEO_TRANSFER_SOURCE_HIGH:
        case CGBL_VIDEO_TRANSFER_SOURCE_LOW:
        case CGBL_VIDEO_WINDOW_X:
        case CGBL_VIDEO_WINDOW_Y:
            result = cgbl_video_read(address);
            break;
        default:
            result = cgbl_memory_read(address);
            break;
    }
    return result;
}

cgbl_error_e cgbl_bus_reset(const cgbl_bank_t *const rom, cgbl_bank_t *const ram)
{
    cgbl_error_e result = CGBL_SUCCESS;
    memset(&bus, 0, sizeof (bus));
    bus.mode.raw = 0xFB;
    bus.priority.raw = 0xFE;
    bus.speed.raw = 0x7E;
    if ((result = cgbl_memory_reset(rom, ram)) == CGBL_SUCCESS)
    {
        cgbl_audio_reset();
        cgbl_infrared_reset();
        cgbl_input_reset();
        cgbl_processor_reset();
        cgbl_serial_reset();
        cgbl_timer_reset();
        cgbl_video_reset();
    }
    return result;
}

cgbl_error_e cgbl_bus_run(void)
{
    cgbl_error_e result = CGBL_SUCCESS;
    for (;;)
    {
        if ((result = cgbl_processor_step()) != CGBL_SUCCESS)
        {
            return result;
        }
        cgbl_audio_step();
        cgbl_cartridge_step();
        cgbl_infrared_step();
        cgbl_input_step();
        cgbl_serial_step();
        cgbl_timer_step();
        if ((result = cgbl_video_step()) != CGBL_SUCCESS)
        {
            return result;
        }
    }
    return result;
}

cgbl_speed_e cgbl_bus_speed(void)
{
    return bus.speed.doubled ? CGBL_SPEED_DOUBLE : CGBL_SPEED_NORMAL;
}

bool cgbl_bus_speed_change(void)
{
    if (bus.speed.armed)
    {
        bus.speed.armed = false;
        bus.speed.doubled = !bus.speed.doubled;
        return true;
    }
    return false;
}

void cgbl_bus_write(uint16_t address, uint8_t data)
{
    switch (address)
    {
        case CGBL_AUDIO_CHANNEL_1_ENVELOPE:
        case CGBL_AUDIO_CHANNEL_1_FREQUENCY_HIGH:
        case CGBL_AUDIO_CHANNEL_1_FREQUENCY_LOW:
        case CGBL_AUDIO_CHANNEL_1_LENGTH:
        case CGBL_AUDIO_CHANNEL_1_SWEEP:
        case CGBL_AUDIO_CHANNEL_2_ENVELOPE:
        case CGBL_AUDIO_CHANNEL_2_FREQUENCY_HIGH:
        case CGBL_AUDIO_CHANNEL_2_FREQUENCY_LOW:
        case CGBL_AUDIO_CHANNEL_2_LENGTH:
        case CGBL_AUDIO_CHANNEL_3_CONTROL:
        case CGBL_AUDIO_CHANNEL_3_FREQUENCY_HIGH:
        case CGBL_AUDIO_CHANNEL_3_FREQUENCY_LOW:
        case CGBL_AUDIO_CHANNEL_3_LENGTH:
        case CGBL_AUDIO_CHANNEL_3_LEVEL:
        case CGBL_AUDIO_CHANNEL_4_CONTROL:
        case CGBL_AUDIO_CHANNEL_4_ENVELOPE:
        case CGBL_AUDIO_CHANNEL_4_FREQUENCY:
        case CGBL_AUDIO_CHANNEL_4_LENGTH:
        case CGBL_AUDIO_CONTROL:
        case CGBL_AUDIO_MIXER:
        case CGBL_AUDIO_RAM_BEGIN ... CGBL_AUDIO_RAM_END:
        case CGBL_AUDIO_VOLUME:
            cgbl_audio_write(address, data);
            break;
        case CGBL_BUS_MODE:
            if (cgbl_bootloader_enabled() && (cgbl_bus_mode() == CGBL_MODE_CGB))
            {
                bus.mode.dmg = (data & 4) >> 2;
            }
            break;
        case CGBL_BUS_PRIORITY:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                bus.priority.dmg = data & 1;
            }
            break;
        case CGBL_BUS_SPEED:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                bus.speed.armed = data & 1;
            }
            break;
        case CGBL_INFRARED_CONTROL:
            cgbl_infrared_write(address, data);
            break;
        case CGBL_INPUT_STATE:
            cgbl_input_write(address, data);
            break;
        case CGBL_PROCESSOR_INTERRUPT_ENABLE:
        case CGBL_PROCESSOR_INTERRUPT_FLAG:
            cgbl_processor_write(address, data);
            break;
        case CGBL_SERIAL_CONTROL:
        case CGBL_SERIAL_DATA:
            cgbl_serial_write(address, data);
            break;
        case CGBL_TIMER_CONTROL:
        case CGBL_TIMER_COUNTER:
        case CGBL_TIMER_DIVIDER:
        case CGBL_TIMER_MODULO:
            cgbl_timer_write(address, data);
            break;
        case CGBL_VIDEO_CONTROL:
        case CGBL_VIDEO_LINE_Y:
        case CGBL_VIDEO_LINE_Y_COINCIDENCE:
        case CGBL_VIDEO_PALETTE_BACKGROUND:
        case CGBL_VIDEO_PALETTE_BACKGROUND_CONTROL:
        case CGBL_VIDEO_PALETTE_BACKGROUND_DATA:
        case CGBL_VIDEO_PALETTE_OBJECT_0:
        case CGBL_VIDEO_PALETTE_OBJECT_1:
        case CGBL_VIDEO_PALETTE_OBJECT_CONTROL:
        case CGBL_VIDEO_PALETTE_OBJECT_DATA:
        case CGBL_VIDEO_RAM_BEGIN ... CGBL_VIDEO_RAM_END:
        case CGBL_VIDEO_RAM_OBJECT_BEGIN ... CGBL_VIDEO_RAM_OBJECT_END:
        case CGBL_VIDEO_RAM_SELECT:
        case CGBL_VIDEO_SCROLL_X:
        case CGBL_VIDEO_SCROLL_Y:
        case CGBL_VIDEO_STATUS:
        case CGBL_VIDEO_TRANSFER_CONTROL:
        case CGBL_VIDEO_TRANSFER_DESTINATION_HIGH:
        case CGBL_VIDEO_TRANSFER_DESTINATION_LOW:
        case CGBL_VIDEO_TRANSFER_OBJECTS:
        case CGBL_VIDEO_TRANSFER_SOURCE_HIGH:
        case CGBL_VIDEO_TRANSFER_SOURCE_LOW:
        case CGBL_VIDEO_WINDOW_X:
        case CGBL_VIDEO_WINDOW_Y:
            cgbl_video_write(address, data);
            break;
        default:
            cgbl_memory_write(address, data);
            break;
    }
}
