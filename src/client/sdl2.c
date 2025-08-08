/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifdef CLIENT_sdl2

#include <SDL.h>
#include "audio.h"
#include "cartridge.h"
#include "client.h"
#include "input.h"
#include "video.h"

static const SDL_GameControllerButton BUTTON[CGBL_BUTTON_MAX] = {
    SDL_CONTROLLER_BUTTON_A, SDL_CONTROLLER_BUTTON_B, SDL_CONTROLLER_BUTTON_BACK, SDL_CONTROLLER_BUTTON_START,
    SDL_CONTROLLER_BUTTON_DPAD_RIGHT, SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_CONTROLLER_BUTTON_DPAD_UP, SDL_CONTROLLER_BUTTON_DPAD_DOWN,
};

static const SDL_Scancode KEY[CGBL_BUTTON_MAX] = {
    SDL_SCANCODE_X, SDL_SCANCODE_Z, SDL_SCANCODE_C, SDL_SCANCODE_SPACE,
    SDL_SCANCODE_RIGHT, SDL_SCANCODE_LEFT, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN,
};

static struct {
    SDL_GameController *controller;
    struct {
        SDL_AudioDeviceID device;
        SDL_AudioSpec specification;
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
        .format = AUDIO_F32,
        .channels = 1,
    };
    if (!(client.audio.device = SDL_OpenAudioDevice(NULL, 0, &specification, &client.audio.specification, 0)))
    {
        return CGBL_ERROR("SDL_OpenAudioDevice failed: %s", SDL_GetError());
    }
    SDL_PauseAudioDevice(client.audio.device, 0);
    return CGBL_SUCCESS;
}

static void cgbl_client_audio_destroy(void)
{
    if (client.audio.device)
    {
        SDL_PauseAudioDevice(client.audio.device, 1);
        SDL_CloseAudioDevice(client.audio.device);
    }
}

static cgbl_error_e cgbl_client_audio_sync(void)
{
    if (SDL_QueueAudio(client.audio.device, cgbl_audio_sample(), CGBL_AUDIO_SAMPLES * sizeof (float)))
    {
        return CGBL_ERROR("SDL_QueueAudio failed: %s", SDL_GetError());
    }
    return CGBL_SUCCESS;
}

static void cgbl_client_controller_create(const SDL_ControllerDeviceEvent *const device)
{
    if (!client.controller)
    {
        client.controller = SDL_GameControllerOpen(device->which);
    }
}

static void cgbl_client_controller_destroy(const SDL_ControllerDeviceEvent *const device)
{
    if (client.controller && (!device || (device->which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(client.controller)))))
    {
        SDL_GameControllerClose(client.controller);
        client.controller = NULL;
    }
}

static void cgbl_client_controller_detect(void)
{
    if (!client.controller)
    {
        for (int32_t id = 0; id < SDL_NumJoysticks(); ++id)
        {
            if (SDL_IsGameController(id))
            {
                client.controller = SDL_GameControllerOpen(id);
                break;
            }
        }
    }
}

static cgbl_error_e cgbl_client_controller_sync(const SDL_ControllerDeviceEvent *const device, const SDL_ControllerButtonEvent *const button)
{
    if (client.controller && (device->which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(client.controller))))
    {
        if (button->button == SDL_CONTROLLER_BUTTON_GUIDE)
        {
            return CGBL_QUIT;
        }
        for (cgbl_button_e index = 0; index < CGBL_BUTTON_MAX; ++index)
        {
            if (button->button == BUTTON[index])
            {
                (*cgbl_input_button())[index] = (button->state == SDL_PRESSED);
                break;
            }
        }
    }
    return CGBL_SUCCESS;
}

static void cgbl_client_frame_begin(void)
{
    client.frame.begin = SDL_GetPerformanceCounter();
}

static float cgbl_client_frame_elapsed(void)
{
    return ((float)(SDL_GetPerformanceCounter() - client.frame.begin) / (float)SDL_GetPerformanceFrequency()) * 1000.0f;
}

static void cgbl_client_frame_end(void)
{
    while((cgbl_client_frame_elapsed() + client.frame.remaining) < CGBL_CLIENT_FRAME_DURATION)
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

static cgbl_error_e cgbl_client_keyboard_sync(const SDL_KeyboardEvent *const key)
{
    if (!key->repeat)
    {
        if (key->keysym.scancode == SDL_SCANCODE_ESCAPE)
        {
            return CGBL_QUIT;
        }
        for (cgbl_button_e index = 0; index < CGBL_BUTTON_MAX; ++index)
        {
            if (key->keysym.scancode == KEY[index])
            {
                (*cgbl_input_button())[index] = (key->state == SDL_PRESSED);
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
        return CGBL_ERROR("Unsupported scale: %u", scale);
    }
    if (!(client.video.window = SDL_CreateWindow(cgbl_cartridge_title(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        CGBL_VIDEO_WIDTH * scale, CGBL_VIDEO_HEIGHT * scale, SDL_WINDOW_RESIZABLE)))
    {
        return CGBL_ERROR("SDL_CreateWindow failed: %s", SDL_GetError());
    }
    if (!(client.video.renderer = SDL_CreateRenderer(client.video.window, -1, SDL_RENDERER_ACCELERATED)))
    {
        return CGBL_ERROR("SDL_CreateRenderer failed: %s", SDL_GetError());
    }
    if (SDL_RenderSetLogicalSize(client.video.renderer, CGBL_VIDEO_WIDTH, CGBL_VIDEO_HEIGHT))
    {
        return CGBL_ERROR("SDL_RenderSetLogicalSize failed: %s", SDL_GetError());
    }
    if (SDL_SetRenderDrawColor(client.video.renderer, 0, 0, 0, 0))
    {
        return CGBL_ERROR("SDL_SetRenderDrawColor failed: %s", SDL_GetError());
    }
    if (SDL_SetHint(SDL_HINT_RENDER_VSYNC, "0") == SDL_FALSE)
    {
        return CGBL_ERROR("SDL_SetHint failed: %s", SDL_GetError());
    }
    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0") == SDL_FALSE)
    {
        return CGBL_ERROR("SDL_SetHint failed: %s", SDL_GetError());
    }
    if (!(client.video.texture = SDL_CreateTexture(client.video.renderer, SDL_PIXELFORMAT_XBGR1555, SDL_TEXTUREACCESS_STREAMING,
        CGBL_VIDEO_WIDTH, CGBL_VIDEO_HEIGHT)))
    {
        return CGBL_ERROR("SDL_CreateTexture failed: %s", SDL_GetError());
    }
    if (fullscreen)
    {
        if (SDL_SetWindowFullscreen(client.video.window, SDL_WINDOW_FULLSCREEN_DESKTOP))
        {
            return CGBL_ERROR("SDL_SetWindowFullscreen failed: %s", SDL_GetError());
        }
        if (SDL_ShowCursor(SDL_DISABLE) < 0)
        {
            return CGBL_ERROR("SDL_ShowCursor failed: %s", SDL_GetError());
        }
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
    if (SDL_UpdateTexture(client.video.texture, NULL, cgbl_video_color(), CGBL_VIDEO_WIDTH * sizeof (uint16_t)))
    {
        return CGBL_ERROR("SDL_UpdateTexture failed: %s", SDL_GetError());
    }
    if (SDL_RenderClear(client.video.renderer))
    {
        return CGBL_ERROR("SDL_RenderClear failed: %s", SDL_GetError());
    }
    if (SDL_RenderCopy(client.video.renderer, client.video.texture, NULL, NULL))
    {
        return CGBL_ERROR("SDL_RenderCopy failed: %s", SDL_GetError());
    }
    SDL_RenderPresent(client.video.renderer);
    return CGBL_SUCCESS;
}

cgbl_error_e cgbl_client_create(uint8_t scale, bool fullscreen)
{
    cgbl_error_e result = CGBL_SUCCESS;
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_VIDEO))
    {
        return CGBL_ERROR("SDL_Init failed: %s", SDL_GetError());
    }
    cgbl_client_frame_begin();
    if (((result = cgbl_client_video_create(scale, fullscreen)) == CGBL_SUCCESS)
        && ((result = cgbl_client_audio_create()) == CGBL_SUCCESS))
    {
        cgbl_client_controller_detect();
    }
    return result;
}

void cgbl_client_destroy(void)
{
    cgbl_client_controller_destroy(NULL);
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
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                if ((result = cgbl_client_controller_sync(&event.cdevice, &event.cbutton)) != CGBL_SUCCESS)
                {
                    return result;
                }
                break;
            case SDL_CONTROLLERDEVICEADDED:
                cgbl_client_controller_create(&event.cdevice);
                break;
            case SDL_CONTROLLERDEVICEREMOVED:
                cgbl_client_controller_destroy(&event.cdevice);
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                if ((result = cgbl_client_keyboard_sync(&event.key)) != CGBL_SUCCESS)
                {
                    return result;
                }
                break;
            case SDL_QUIT:
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

#endif /* CLIENT_sdl2 */
