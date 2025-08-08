/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include "audio.h"

static const uint32_t DIVIDER[] = {
    8, 16, 32, 48, 64, 80, 96, 112,
};

static const float PULSE[][8] = {
    { -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f,  1.f, },
    { -1.f, -1.f, -1.f, -1.f, -1.f, -1.f,  1.f,  1.f, },
    { -1.f, -1.f, -1.f, -1.f,  1.f,  1.f,  1.f,  1.f, },
    {  1.f,  1.f,  1.f,  1.f,  1.f,  1.f, -1.f, -1.f, },
};

static const uint8_t SHIFT[] = {
    4, 0, 1, 2,
};

static struct {
    uint32_t cycle;
    uint16_t delay;
    uint32_t index;
    uint8_t ram[CGBL_AUDIO_RAM_WIDTH];
    float downsample[CGBL_AUDIO_SAMPLES];
    float sample[CGBL_AUDIO_SAMPLES + 63];
    struct {
        uint32_t delay;
        uint8_t position;
        uint8_t volume;
        union {
            struct {
                uint8_t period : 3;
                uint8_t direction : 1;
                uint8_t volume : 4;
            };
            uint8_t raw;
        } envelope;
        struct {
            uint8_t low;
            union {
                struct {
                    uint8_t period : 3;
                    uint8_t : 3;
                    uint8_t enabled : 1;
                    uint8_t trigger : 1;
                };
                uint8_t raw;
            } high;
        } frequency;
        union {
            struct {
                uint8_t timer : 6;
                uint8_t duty : 2;
            };
            uint8_t raw;
        } length;
        union {
            struct {
                uint8_t shift : 3;
                uint8_t direction : 1;
                uint8_t period : 3;
            };
            uint8_t raw;
        } sweep;
        struct {
            uint8_t length;
            struct {
                uint8_t period;
            } envelope;
            struct {
                bool enabled;
                uint16_t frequency;
                uint8_t period;
            } sweep;
        } timer;
    } channel_1;
    struct {
        uint32_t delay;
        uint8_t position;
        uint8_t volume;
        union {
            struct {
                uint8_t period : 3;
                uint8_t direction : 1;
                uint8_t volume : 4;
            };
            uint8_t raw;
        } envelope;
        struct {
            uint8_t low;
            union {
                struct {
                    uint8_t period : 3;
                    uint8_t : 3;
                    uint8_t enabled : 1;
                    uint8_t trigger : 1;
                };
                uint8_t raw;
            } high;
        } frequency;
        union {
            struct {
                uint8_t timer : 6;
                uint8_t duty : 2;
            };
            uint8_t raw;
        } length;
        struct {
            uint8_t length;
            struct {
                uint8_t period;
            } envelope;
        } timer;
    } channel_2;
    struct {
        uint32_t delay;
        uint8_t length;
        uint8_t position;
        union {
            struct {
                uint8_t : 7;
                uint8_t enabled : 1;
            };
            uint8_t raw;
        } control;
        struct {
            uint8_t low;
            union {
                struct {
                    uint8_t period : 3;
                    uint8_t : 3;
                    uint8_t enabled : 1;
                    uint8_t trigger : 1;
                };
                uint8_t raw;
            } high;
        } frequency;
        union {
            struct {
                uint8_t : 5;
                uint8_t output : 2;
            };
            uint8_t raw;
        } level;
        struct {
            uint16_t length;
        } timer;
    } channel_3;
    struct {
        uint32_t delay;
        uint16_t sample;
        uint8_t volume;
        union {
            struct {
                struct {
                    uint8_t : 6;
                    uint8_t enabled : 1;
                    uint8_t trigger : 1;
                };
            };
            uint8_t raw;
        } control;
        union {
            struct {
                uint8_t period : 3;
                uint8_t direction : 1;
                uint8_t volume : 4;
            };
            uint8_t raw;
        } envelope;
        union {
            struct {
                uint8_t divider : 3;
                uint8_t width : 1;
                uint8_t shift : 4;
            };
            uint8_t raw;
        } frequency;
        union {
            struct {
                uint8_t timer : 6;
                uint8_t : 2;
            };
            uint8_t raw;
        } length;
        struct {
            uint8_t length;
            struct {
                uint8_t period;
            } envelope;
        } timer;
    } channel_4;
    union {
        struct {
            uint8_t channel_1_enabled : 1;
            uint8_t channel_2_enabled : 1;
            uint8_t channel_3_enabled : 1;
            uint8_t channel_4_enabled : 1;
            uint8_t : 3;
            uint8_t enabled : 1;
        };
        uint8_t raw;
    } control;
    union {
        struct {
            uint8_t channel_1_right : 1;
            uint8_t channel_2_right : 1;
            uint8_t channel_3_right : 1;
            uint8_t channel_4_right : 1;
            uint8_t channel_1_left : 1;
            uint8_t channel_2_left : 1;
            uint8_t channel_3_left : 1;
            uint8_t channel_4_left : 1;
        };
        uint8_t raw;
    } mixer;
    union {
        struct {
            uint8_t right : 3;
            uint8_t : 1;
            uint8_t left : 3;
            uint8_t : 1;
        };
        uint8_t raw;
    } volume;
} audio = {};

static void cgbl_audio_channel_1_envelope(void)
{
    if (audio.channel_1.timer.envelope.period && !--audio.channel_1.timer.envelope.period)
    {
        audio.channel_1.timer.envelope.period = audio.channel_1.envelope.period;
        if (audio.channel_1.envelope.direction)
        {
            if (audio.channel_1.volume < 15)
            {
                ++audio.channel_1.volume;
            }
        }
        else if (audio.channel_1.volume)
        {
            --audio.channel_1.volume;
        }
    }
}

static void cgbl_audio_channel_1_length(void)
{
    if (audio.channel_1.frequency.high.enabled && audio.channel_1.timer.length)
    {
        if (!--audio.channel_1.timer.length)
        {
            audio.control.channel_1_enabled = false;
        }
    }
}

static void cgbl_audio_channel_1_sample(float *const left, float *const right)
{
    if (audio.control.channel_1_enabled)
    {
        float sample = (PULSE[audio.channel_1.length.duty][audio.channel_1.position] * audio.channel_1.volume) / 15.f;
        if (audio.mixer.channel_1_left)
        {
            *left += sample;
        }
        if (audio.mixer.channel_1_right)
        {
            *right += sample;
        }
    }
}

static void cgbl_audio_channel_1_step(void)
{
    if (audio.control.channel_1_enabled)
    {
        if (!audio.channel_1.delay)
        {
            audio.channel_1.delay = (2048 - ((audio.channel_1.frequency.high.period << 8) | audio.channel_1.frequency.low)) * 4;
            audio.channel_1.position = (audio.channel_1.position + 1) & 7;
        }
        --audio.channel_1.delay;
    }
}

static void cgbl_audio_channel_1_sweep(void)
{
    if (audio.channel_1.timer.sweep.period && !--audio.channel_1.timer.sweep.period)
    {
        audio.channel_1.timer.sweep.period = !audio.channel_1.sweep.period ? 8 : audio.channel_1.sweep.period;
        if (audio.channel_1.timer.sweep.enabled && audio.channel_1.sweep.period)
        {
            uint16_t frequency = audio.channel_1.timer.sweep.frequency >> audio.channel_1.sweep.shift;
            if (audio.channel_1.sweep.direction)
            {
                frequency = audio.channel_1.timer.sweep.frequency - frequency;
            }
            else
            {
                frequency = audio.channel_1.timer.sweep.frequency + frequency;
            }
            if (frequency > 2047)
            {
                audio.channel_1.timer.sweep.enabled = false;
            }
            else if (audio.channel_1.sweep.shift)
            {
                audio.channel_1.frequency.high.period = frequency >> 8;
                audio.channel_1.frequency.low = frequency;
                audio.channel_1.timer.sweep.frequency = frequency;
            }
        }
    }
}

static void cgbl_audio_channel_1_trigger(void)
{
    if (audio.channel_1.frequency.high.trigger)
    {
        audio.channel_1.timer.envelope.period = audio.channel_1.envelope.period;
        audio.channel_1.timer.length = 64 - audio.channel_1.length.timer;
        audio.channel_1.timer.sweep.enabled = (audio.channel_1.sweep.period || audio.channel_1.sweep.shift);
        audio.channel_1.timer.sweep.frequency = (audio.channel_1.frequency.high.period << 8) | audio.channel_1.frequency.low;
        audio.channel_1.timer.sweep.period = !audio.channel_1.sweep.period ? 8 : audio.channel_1.sweep.period;
        audio.channel_1.volume = audio.channel_1.envelope.volume;
        audio.control.channel_1_enabled = true;
    }
}

static void cgbl_audio_channel_2_envelope(void)
{
    if (audio.channel_2.timer.envelope.period && !--audio.channel_2.timer.envelope.period)
    {
        audio.channel_2.timer.envelope.period = audio.channel_2.envelope.period;
        if (audio.channel_2.envelope.direction)
        {
            if (audio.channel_2.volume < 15)
            {
                ++audio.channel_2.volume;
            }
        }
        else if (audio.channel_2.volume)
        {
            --audio.channel_2.volume;
        }
    }
}

static void cgbl_audio_channel_2_length(void)
{
    if (audio.channel_2.frequency.high.enabled && audio.channel_2.timer.length)
    {
        if (!--audio.channel_2.timer.length)
        {
            audio.control.channel_2_enabled = false;
        }
    }
}

static void cgbl_audio_channel_2_sample(float *const left, float *const right)
{
    if (audio.control.channel_2_enabled)
    {
        float sample = (PULSE[audio.channel_2.length.duty][audio.channel_2.position] * audio.channel_2.volume) / 15.f;
        if (audio.mixer.channel_2_left)
        {
            *left += sample;
        }
        if (audio.mixer.channel_2_right)
        {
            *right += sample;
        }
    }
}

static void cgbl_audio_channel_2_step(void)
{
    if (audio.control.channel_2_enabled)
    {
        if (!audio.channel_2.delay)
        {
            audio.channel_2.delay = (2048 - ((audio.channel_2.frequency.high.period << 8) | audio.channel_2.frequency.low)) * 4;
            audio.channel_2.position = (audio.channel_2.position + 1) & 7;
        }
        --audio.channel_2.delay;
    }
}

static void cgbl_audio_channel_2_trigger(void)
{
    if (audio.channel_2.frequency.high.trigger)
    {
        audio.channel_2.timer.envelope.period = audio.channel_2.envelope.period;
        audio.channel_2.timer.length = 64 - audio.channel_2.length.timer;
        audio.channel_2.volume = audio.channel_2.envelope.volume;
        audio.control.channel_2_enabled = true;
    }
}

static void cgbl_audio_channel_3_length(void)
{
    if (audio.channel_3.frequency.high.enabled && audio.channel_3.timer.length)
    {
        if (!--audio.channel_3.timer.length)
        {
            audio.control.channel_3_enabled = false;
        }
    }
}

static void cgbl_audio_channel_3_sample(float *const left, float *const right)
{
    if (audio.control.channel_3_enabled)
    {
        float sample = 0.f;
        uint8_t data = audio.ram[audio.channel_3.position / 2];
        if (!(audio.channel_3.position % 2))
        {
            data >>= 4;
        }
        data &= 15;
        data >>= SHIFT[audio.channel_3.level.output];
        sample = data / 15.f;
        if (audio.mixer.channel_3_left)
        {
            *left += sample;
        }
        if (audio.mixer.channel_3_right)
        {
            *right += sample;
        }
    }
}

static void cgbl_audio_channel_3_step(void)
{
    if (audio.control.channel_3_enabled)
    {
        if (!audio.channel_3.delay)
        {
            audio.channel_3.delay = (2048 - ((audio.channel_3.frequency.high.period << 8) | audio.channel_3.frequency.low)) * 2;
            audio.channel_3.position = (audio.channel_3.position + 1) & 31;
        }
        --audio.channel_3.delay;
    }
}

static void cgbl_audio_channel_3_trigger(void)
{
    if (audio.channel_3.frequency.high.trigger)
    {
        audio.channel_3.position = 0;
        audio.channel_3.timer.length = 256 - audio.channel_3.length;
        audio.control.channel_3_enabled = true;
    }
}

static void cgbl_audio_channel_4_envelope(void)
{
    if (audio.channel_4.timer.envelope.period && !--audio.channel_4.timer.envelope.period)
    {
        audio.channel_4.timer.envelope.period = audio.channel_4.envelope.period;
        if (audio.channel_4.envelope.direction)
        {
            if (audio.channel_4.volume < 15)
            {
                ++audio.channel_4.volume;
            }
        }
        else if (audio.channel_4.volume)
        {
            --audio.channel_4.volume;
        }
    }
}

static void cgbl_audio_channel_4_length(void)
{
    if (audio.channel_4.control.enabled && audio.channel_4.timer.length)
    {
        if (!--audio.channel_4.timer.length)
        {
            audio.control.channel_4_enabled = false;
        }
    }
}

static void cgbl_audio_channel_4_sample(float *const left, float *const right)
{
    if (audio.control.channel_4_enabled)
    {
        float sample = (((audio.channel_4.sample & 1) ? 1.f : -1.f) * audio.channel_4.volume) / 15.f;
        if (audio.mixer.channel_4_left)
        {
            *left += sample;
        }
        if (audio.mixer.channel_4_right)
        {
            *right += sample;
        }
    }
}

static void cgbl_audio_channel_4_step(void)
{
    if (audio.control.channel_4_enabled)
    {
        if (!audio.channel_4.delay)
        {
            uint16_t sample = 0;
            audio.channel_4.delay = DIVIDER[audio.channel_4.frequency.divider] << audio.channel_4.frequency.shift;
            sample = !((audio.channel_4.sample & 1) ^ ((audio.channel_4.sample & 2) >> 1));
            audio.channel_4.sample = (audio.channel_4.sample >> 1) | (sample << 14);
            if (audio.channel_4.frequency.width)
            {
                audio.channel_4.sample &= ~(1 << 6);
                audio.channel_4.sample |= (sample << 6);
            }
        }
        --audio.channel_4.delay;
    }
}

static void cgbl_audio_channel_4_trigger(void)
{
    if (audio.channel_4.control.trigger)
    {
        audio.channel_4.sample = 0;
        audio.channel_4.timer.envelope.period = audio.channel_4.envelope.period;
        audio.channel_4.timer.length = 64 - audio.channel_4.length.timer;
        audio.channel_4.volume = audio.channel_4.envelope.volume;
        audio.control.channel_4_enabled = true;
    }
}

static void cgbl_audio_downsample(void)
{
    uint32_t destination_length = CGBL_LENGTH(audio.downsample), source_length = CGBL_LENGTH(audio.sample);
    for (uint32_t index = 0; index < destination_length; ++index)
    {
        float delta = 0.f, source_index = (float)index * (source_length - 1) / (destination_length - 1);
        uint32_t index_0 = (uint32_t)source_index, index_1 = index_0 + 1;
        if (index_1 >= source_length)
        {
            index_1 = source_length - 1;
            index_0 = index_1;
        }
        delta = source_index - index_1;
        audio.downsample[index] = (audio.sample[index_0] * (1.f - delta)) + (audio.sample[index_1] * delta);
    }
}

uint8_t cgbl_audio_read(uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case CGBL_AUDIO_CHANNEL_1_ENVELOPE:
            result = audio.channel_1.envelope.raw;
            break;
        case CGBL_AUDIO_CHANNEL_1_FREQUENCY_HIGH:
            result = audio.channel_1.frequency.high.raw;
            break;
        case CGBL_AUDIO_CHANNEL_1_LENGTH:
            result = audio.channel_1.length.raw;
            break;
        case CGBL_AUDIO_CHANNEL_1_SWEEP:
            result = audio.channel_1.sweep.raw;
            break;
        case CGBL_AUDIO_CHANNEL_2_ENVELOPE:
            result = audio.channel_2.envelope.raw;
            break;
        case CGBL_AUDIO_CHANNEL_2_FREQUENCY_HIGH:
            result = audio.channel_2.frequency.high.raw;
            break;
        case CGBL_AUDIO_CHANNEL_2_LENGTH:
            result = audio.channel_2.length.raw;
            break;
        case CGBL_AUDIO_CHANNEL_3_CONTROL:
            result = audio.channel_3.control.raw;
            break;
        case CGBL_AUDIO_CHANNEL_3_FREQUENCY_HIGH:
            result = audio.channel_3.frequency.high.raw;
            break;
        case CGBL_AUDIO_CHANNEL_3_LENGTH:
            result = audio.channel_3.length;
            break;
        case CGBL_AUDIO_CHANNEL_3_LEVEL:
            result = audio.channel_3.level.raw;
            break;
        case CGBL_AUDIO_CHANNEL_4_CONTROL:
            result = audio.channel_4.control.raw;
            break;
        case CGBL_AUDIO_CHANNEL_4_ENVELOPE:
            result = audio.channel_4.envelope.raw;
            break;
        case CGBL_AUDIO_CHANNEL_4_FREQUENCY:
            result = audio.channel_4.frequency.raw;
            break;
        case CGBL_AUDIO_CHANNEL_4_LENGTH:
            result = audio.channel_4.length.raw;
            break;
        case CGBL_AUDIO_CONTROL:
            result = audio.control.raw;
            break;
        case CGBL_AUDIO_MIXER:
            result = audio.mixer.raw;
            break;
        case CGBL_AUDIO_RAM_BEGIN ... CGBL_AUDIO_RAM_END:
            result = audio.ram[address - CGBL_AUDIO_RAM_BEGIN];
            break;
        case CGBL_AUDIO_VOLUME:
            result = audio.volume.raw;
            break;
        default:
            break;
    }
    return result;
}

void cgbl_audio_reset(void)
{
    memset(&audio, 0, sizeof (audio));
    audio.channel_1.frequency.high.raw = 0x38;
    audio.channel_1.sweep.raw = 0x80;
    audio.channel_2.frequency.high.raw = 0x38;
    audio.channel_3.control.raw = 0x7F;
    audio.channel_3.frequency.high.raw = 0x38;
    audio.channel_3.level.raw = 0x9F;
    audio.channel_4.control.raw = 0x3F;
    audio.channel_4.length.raw = 0xC0;
    audio.control.raw = 0x70;
    audio.volume.raw = 0x88;
}

const float (*cgbl_audio_sample(void))[CGBL_AUDIO_SAMPLES]
{
    return &audio.downsample;
}

void cgbl_audio_signal(void)
{
    cgbl_audio_channel_1_length();
    cgbl_audio_channel_2_length();
    cgbl_audio_channel_3_length();
    cgbl_audio_channel_4_length();
    if (!(audio.cycle % 2))
    {
        cgbl_audio_channel_1_sweep();
    }
    if (!(audio.cycle % 4))
    {
        cgbl_audio_channel_1_envelope();
        cgbl_audio_channel_2_envelope();
        cgbl_audio_channel_4_envelope();
    }
    if (++audio.cycle >= 4)
    {
        audio.cycle = 0;
    }
}

void cgbl_audio_step(void)
{
    cgbl_audio_channel_1_step();
    cgbl_audio_channel_2_step();
    cgbl_audio_channel_3_step();
    cgbl_audio_channel_4_step();
    if (!audio.delay)
    {
        float left = 0.f, right = 0.f;
        if (audio.control.enabled)
        {
            cgbl_audio_channel_1_sample(&left, &right);
            cgbl_audio_channel_2_sample(&left, &right);
            cgbl_audio_channel_3_sample(&left, &right);
            cgbl_audio_channel_4_sample(&left, &right);
            left *= (audio.volume.left + 1.f) / 8.f;
            right *= (audio.volume.right + 1.f) / 8.f;
        }
        audio.sample[audio.index++] = ((left / 4.f) + (right / 4.f)) / 2.f;
        if (audio.index >= CGBL_LENGTH(audio.sample))
        {
            cgbl_audio_downsample();
            audio.index = 0;
        }
        audio.delay = 88;
    }
    --audio.delay;
}

void cgbl_audio_write(uint16_t address, uint8_t data)
{
    switch (address)
    {
        case CGBL_AUDIO_CHANNEL_1_ENVELOPE:
            if (audio.control.enabled)
            {
                audio.channel_1.envelope.raw = data;
            }
            break;
        case CGBL_AUDIO_CHANNEL_1_FREQUENCY_HIGH:
            if (audio.control.enabled)
            {
                audio.channel_1.frequency.high.raw = data | 0x38;
                cgbl_audio_channel_1_trigger();
            }
            break;
        case CGBL_AUDIO_CHANNEL_1_FREQUENCY_LOW:
            if (audio.control.enabled)
            {
                audio.channel_1.frequency.low = data;
            }
            break;
        case CGBL_AUDIO_CHANNEL_1_LENGTH:
            if (audio.control.enabled)
            {
                audio.channel_1.length.raw = data;
            }
            break;
        case CGBL_AUDIO_CHANNEL_1_SWEEP:
            if (audio.control.enabled)
            {
                audio.channel_1.sweep.raw = data | 0x80;
            }
            break;
        case CGBL_AUDIO_CHANNEL_2_ENVELOPE:
            if (audio.control.enabled)
            {
                audio.channel_2.envelope.raw = data;
            }
            break;
        case CGBL_AUDIO_CHANNEL_2_FREQUENCY_HIGH:
            if (audio.control.enabled)
            {
                audio.channel_2.frequency.high.raw = data | 0x38;
                cgbl_audio_channel_2_trigger();
            }
            break;
        case CGBL_AUDIO_CHANNEL_2_FREQUENCY_LOW:
            if (audio.control.enabled)
            {
                audio.channel_2.frequency.low = data;
            }
            break;
        case CGBL_AUDIO_CHANNEL_2_LENGTH:
            if (audio.control.enabled)
            {
                audio.channel_2.length.raw = data;
            }
            break;
        case CGBL_AUDIO_CHANNEL_3_CONTROL:
            if (audio.control.enabled)
            {
                audio.channel_3.control.raw = data | 0x7F;
                if (!audio.channel_3.control.enabled)
                {
                    audio.control.channel_3_enabled = false;
                }
            }
            break;
        case CGBL_AUDIO_CHANNEL_3_FREQUENCY_HIGH:
            if (audio.control.enabled)
            {
                audio.channel_3.frequency.high.raw = data | 0x38;
                cgbl_audio_channel_3_trigger();
            }
            break;
        case CGBL_AUDIO_CHANNEL_3_FREQUENCY_LOW:
            if (audio.control.enabled)
            {
                audio.channel_3.frequency.low = data;
            }
            break;
        case CGBL_AUDIO_CHANNEL_3_LENGTH:
            if (audio.control.enabled)
            {
                audio.channel_3.length = data;
            }
            break;
        case CGBL_AUDIO_CHANNEL_3_LEVEL:
            if (audio.control.enabled)
            {
                audio.channel_3.level.raw = data | 0x9F;
            }
            break;
        case CGBL_AUDIO_CHANNEL_4_CONTROL:
            if (audio.control.enabled)
            {
                audio.channel_4.control.raw = data | 0x3F;
                cgbl_audio_channel_4_trigger();
            }
            break;
        case CGBL_AUDIO_CHANNEL_4_ENVELOPE:
            if (audio.control.enabled)
            {
                audio.channel_4.envelope.raw = data;
            }
            break;
        case CGBL_AUDIO_CHANNEL_4_FREQUENCY:
            if (audio.control.enabled)
            {
                audio.channel_4.frequency.raw = data;
            }
            break;
        case CGBL_AUDIO_CHANNEL_4_LENGTH:
            if (audio.control.enabled)
            {
                audio.channel_4.length.raw = data | 0xC0;
            }
            break;
        case CGBL_AUDIO_CONTROL:
            audio.control.raw |= data & 0x80;
            if (!audio.control.enabled)
            {
                memset(&audio.channel_1, 0, sizeof (audio.channel_1));
                memset(&audio.channel_2, 0, sizeof (audio.channel_2));
                memset(&audio.channel_3, 0, sizeof (audio.channel_3));
                memset(&audio.channel_4, 0, sizeof (audio.channel_4));
                memset(audio.ram, 0, sizeof (audio.ram));
                memset(audio.sample, 0, sizeof (audio.sample));
            }
            break;
        case CGBL_AUDIO_MIXER:
            if (audio.control.enabled)
            {
                audio.mixer.raw = data;
            }
            break;
        case CGBL_AUDIO_RAM_BEGIN ... CGBL_AUDIO_RAM_END:
            if (audio.control.enabled)
            {
                audio.ram[address - CGBL_AUDIO_RAM_BEGIN] = data;
            }
            break;
        case CGBL_AUDIO_VOLUME:
            if (audio.control.enabled)
            {
                audio.volume.raw = data;
            }
            break;
        default:
            break;
    }
}
