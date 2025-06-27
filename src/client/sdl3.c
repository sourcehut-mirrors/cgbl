/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifdef CLIENT_sdl3

#include <SDL3/SDL.h>
#include "audio.h"
#include "cartridge.h"
#include "client.h"
#include "input.h"
#include "video.h"

static const SDL_GamepadButton BUTTON[CGBL_BUTTON_MAX] = {
    SDL_GAMEPAD_BUTTON_EAST, SDL_GAMEPAD_BUTTON_SOUTH, SDL_GAMEPAD_BUTTON_BACK, SDL_GAMEPAD_BUTTON_START,
    SDL_GAMEPAD_BUTTON_DPAD_RIGHT, SDL_GAMEPAD_BUTTON_DPAD_LEFT, SDL_GAMEPAD_BUTTON_DPAD_UP, SDL_GAMEPAD_BUTTON_DPAD_DOWN,
};

static const SDL_Scancode KEY[CGBL_BUTTON_MAX] = {
    SDL_SCANCODE_X, SDL_SCANCODE_Z, SDL_SCANCODE_C, SDL_SCANCODE_SPACE,
    SDL_SCANCODE_RIGHT, SDL_SCANCODE_LEFT, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN,
};

static struct {
    SDL_Gamepad *gamepad;
    struct {
        SDL_AudioStream *stream;
    } audio;
    struct {
        uint64_t begin;
        float remaining;
    } frame;
    struct {
        SDL_Renderer *renderer;
        SDL_Texture *texture;
        SDL_Window *window;
    } video;
} client = {};

static cgbl_error_e cgbl_client_audio_create(void)
{
    SDL_AudioSpec specification = {
        .freq = CGBL_AUDIO_SAMPLES * CGBL_CLIENT_FRAME_RATE,
        .format = SDL_AUDIO_F32LE,
        .channels = 1,
    };
    if (!(client.audio.stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &specification, NULL, NULL)))
    {
        return CGBL_ERROR("SDL_OpenAudioDeviceStream failed -- %s", SDL_GetError());
    }
    if (!SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(client.audio.stream)))
    {
        return CGBL_ERROR("SDL_ResumeAudioDevice failed -- %s", SDL_GetError());
    }
    return CGBL_SUCCESS;
}

static void cgbl_client_audio_destroy(void)
{
    if (client.audio.stream)
    {
        SDL_PauseAudioDevice(SDL_GetAudioStreamDevice(client.audio.stream));
        SDL_CloseAudioDevice(SDL_GetAudioStreamDevice(client.audio.stream));
    }
}

static cgbl_error_e cgbl_client_audio_sync(void)
{
    if (!SDL_PutAudioStreamData(client.audio.stream, cgbl_audio_sample(), CGBL_AUDIO_SAMPLES * sizeof (float)))
    {
        return CGBL_ERROR("SDL_PutAudioStreamData failed -- %s", SDL_GetError());
    }
    return CGBL_SUCCESS;
}

static void cgbl_client_frame_begin(void)
{
    client.frame.begin = SDL_GetPerformanceCounter();
}

static float cgbl_client_frame_elapsed(void)
{
    return ((SDL_GetPerformanceCounter() - client.frame.begin) / (float)SDL_GetPerformanceFrequency()) * 1000.f;
}

static void cgbl_client_frame_end(void)
{
    while ((cgbl_client_frame_elapsed() + client.frame.remaining) < CGBL_CLIENT_FRAME_DURATION)
    {
        SDL_Delay(1);
    }
    if (cgbl_client_frame_elapsed() < CGBL_CLIENT_FRAME_DURATION)
    {
        client.frame.remaining -= CGBL_CLIENT_FRAME_DURATION - cgbl_client_frame_elapsed();
    }
    else
    {
        client.frame.remaining += cgbl_client_frame_elapsed() - CGBL_CLIENT_FRAME_DURATION;
    }
}

static void cgbl_client_gamepad_create(const SDL_GamepadDeviceEvent *const device)
{
    if (!client.gamepad)
    {
        client.gamepad = SDL_OpenGamepad(device->which);
    }
}

static void cgbl_client_gamepad_destroy(const SDL_GamepadDeviceEvent *const device)
{
    if (client.gamepad && (!device || (device->which == SDL_GetJoystickID(SDL_GetGamepadJoystick(client.gamepad)))))
    {
        SDL_CloseGamepad(client.gamepad);
        client.gamepad = NULL;
    }
}

static void cgbl_client_gamepad_detect(void)
{
    if (!client.gamepad)
    {
        int32_t count = 0;
        SDL_JoystickID *joystick = SDL_GetJoysticks(&count);
        for (int32_t id = 0; id < count; ++id)
        {
            if (SDL_IsGamepad(joystick[id]))
            {
                client.gamepad = SDL_OpenGamepad(joystick[id]);
                break;
            }
        }
    }
}

static cgbl_error_e cgbl_client_gamepad_sync(const SDL_GamepadDeviceEvent *const device, const SDL_GamepadButtonEvent *const button)
{
    if (client.gamepad && (device->which == SDL_GetJoystickID(SDL_GetGamepadJoystick(client.gamepad))))
    {
        if (button->button == SDL_GAMEPAD_BUTTON_GUIDE)
        {
            return CGBL_QUIT;
        }
        for (cgbl_button_e index = 0; index < CGBL_BUTTON_MAX; ++index)
        {
            if (button->button == BUTTON[index])
            {
                (*cgbl_input_button())[index] = button->down;
                break;
            }
        }
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_client_keyboard_sync(const SDL_KeyboardEvent *const key)
{
    if (!key->repeat)
    {
        if (key->scancode == SDL_SCANCODE_ESCAPE)
        {
            return CGBL_QUIT;
        }
        for (cgbl_button_e index = 0; index < CGBL_BUTTON_MAX; ++index)
        {
            if (key->scancode == KEY[index])
            {
                (*cgbl_input_button())[index] = key->down;
                break;
            }
        }
    }
    return CGBL_SUCCESS;
}

static cgbl_error_e cgbl_client_video_create(uint8_t scale, bool fullscreen)
{
    if ((scale < CGBL_CLIENT_SCALE_MIN) || (scale > CGBL_CLIENT_SCALE_MAX))
    {
        return CGBL_ERROR("Unsupported scale -- %u", scale);
    }
    if (!SDL_CreateWindowAndRenderer(cgbl_cartridge_title(), CGBL_VIDEO_WIDTH * scale, CGBL_VIDEO_HEIGHT * scale, SDL_WINDOW_RESIZABLE,
        &client.video.window, &client.video.renderer))
    {
        return CGBL_ERROR("SDL_CreateWindowAndRenderer failed -- %s", SDL_GetError());
    }
    if (!SDL_SetRenderLogicalPresentation(client.video.renderer, CGBL_VIDEO_WIDTH, CGBL_VIDEO_HEIGHT, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE))
    {
        return CGBL_ERROR("SDL_SetRenderLogicalPresentation failed -- %s", SDL_GetError());
    }
    if (!SDL_SetRenderDrawColor(client.video.renderer, 0, 0, 0, 0))
    {
        return CGBL_ERROR("SDL_SetRenderDrawColor failed -- %s", SDL_GetError());
    }
    if (!SDL_SetRenderVSync(client.video.renderer, 1))
    {
        return CGBL_ERROR("SDL_SetRenderVSync failed -- %s", SDL_GetError());
    }
    if (!(client.video.texture = SDL_CreateTexture(client.video.renderer, SDL_PIXELFORMAT_XBGR1555, SDL_TEXTUREACCESS_STREAMING,
        CGBL_VIDEO_WIDTH, CGBL_VIDEO_HEIGHT)))
    {
        return CGBL_ERROR("SDL_CreateTexture failed -- %s", SDL_GetError());
    }
    if (!SDL_SetTextureScaleMode(client.video.texture, SDL_SCALEMODE_NEAREST))
    {
        return CGBL_ERROR("SDL_SetTextureScaleMode failed -- %s", SDL_GetError());
    }
    if (!SDL_SetWindowFullscreen(client.video.window, fullscreen))
    {
        return CGBL_ERROR("SDL_SetWindowFullscreen failed -- %s", SDL_GetError());
    }
    if (fullscreen && !SDL_HideCursor())
    {
        return CGBL_ERROR("SDL_HideCursor failed -- %s", SDL_GetError());
    }
    return CGBL_SUCCESS;
}

static void cgbl_client_video_destroy(void)
{
    if (client.video.texture)
    {
        SDL_DestroyTexture(client.video.texture);
    }
    if (client.video.renderer)
    {
        SDL_DestroyRenderer(client.video.renderer);
    }
    if (client.video.window)
    {
        SDL_DestroyWindow(client.video.window);
    }
}

static cgbl_error_e cgbl_client_video_sync(void)
{
    if (!SDL_UpdateTexture(client.video.texture, NULL, cgbl_video_color(), CGBL_VIDEO_WIDTH * sizeof (uint16_t)))
    {
        return CGBL_ERROR("SDL_UpdateTexture failed -- %s", SDL_GetError());
    }
    if (!SDL_RenderClear(client.video.renderer))
    {
        return CGBL_ERROR("SDL_RenderClear failed -- %s", SDL_GetError());
    }
    if (!SDL_RenderTexture(client.video.renderer, client.video.texture, NULL, NULL))
    {
        return CGBL_ERROR("SDL_RenderTexture failed -- %s", SDL_GetError());
    }
    if (!SDL_RenderPresent(client.video.renderer))
    {
        return CGBL_ERROR("SDL_RenderPresent failed -- %s", SDL_GetError());
    }
    return CGBL_SUCCESS;
}

cgbl_error_e cgbl_client_create(uint8_t scale, bool fullscreen)
{
    cgbl_error_e result = CGBL_SUCCESS;
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_GAMEPAD | SDL_INIT_VIDEO))
    {
        return CGBL_ERROR("SDL_Init failed -- %s", SDL_GetError());
    }
    cgbl_client_frame_begin();
    if (((result = cgbl_client_video_create(scale, fullscreen)) == CGBL_SUCCESS)
        && ((result = cgbl_client_audio_create()) == CGBL_SUCCESS))
    {
        cgbl_client_gamepad_detect();
    }
    return result;
}

void cgbl_client_destroy(void)
{
    cgbl_client_gamepad_destroy(NULL);
    cgbl_client_audio_destroy();
    cgbl_client_video_destroy();
    SDL_Quit();
    memset(&client, 0, sizeof (client));
}

cgbl_error_e cgbl_client_poll(void)
{
    SDL_Event event;
    cgbl_error_e result = CGBL_SUCCESS;
    cgbl_client_frame_begin();
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            case SDL_EVENT_GAMEPAD_BUTTON_UP:
                if ((result = cgbl_client_gamepad_sync(&event.gdevice, &event.gbutton)) != CGBL_SUCCESS)
                {
                    return result;
                }
                break;
            case SDL_EVENT_GAMEPAD_ADDED:
                cgbl_client_gamepad_create(&event.gdevice);
                break;
            case SDL_EVENT_GAMEPAD_REMOVED:
                cgbl_client_gamepad_destroy(&event.gdevice);
                break;
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                if ((result = cgbl_client_keyboard_sync(&event.key)) != CGBL_SUCCESS)
                {
                    return result;
                }
                break;
            case SDL_EVENT_QUIT:
                return CGBL_QUIT;
            default:
                break;
        }
    }
    return result;
}

cgbl_error_e cgbl_client_sync(void)
{
    cgbl_error_e result = CGBL_SUCCESS;
    if ((result = cgbl_client_video_sync()) == CGBL_SUCCESS)
    {
        result = cgbl_client_audio_sync();
    }
    cgbl_client_frame_end();
    return result;
}

#endif /* CLIENT_sdl3 */
