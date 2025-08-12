/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgbl.h"

static const char *DESCRIPTION[] = {
    "Enable debug mode",
    "Set window fullscreen",
    "Show help information",
    "Set window scale",
    "Show version information",
};

static const struct option OPTION[] = {
    { "debug", no_argument, NULL, 'd', },
    { "fullscreen", no_argument, NULL, 'f', },
    { "help", no_argument, NULL, 'h', },
    { "scale", required_argument, NULL, 's', },
    { "version", no_argument, NULL, 'v', },
    { NULL, 0, NULL, 0, },
};

static void usage(void)
{
    uint32_t index = 0;
    fprintf(stdout, "Usage: cgbl [options] [file]\n\n");
    fprintf(stdout, "Options:\n");
    while (OPTION[index].name)
    {
        char buffer[22] = {};
        snprintf(buffer, sizeof (buffer), "   -%c, --%s", OPTION[index].val, OPTION[index].name);
        for (uint32_t offset = strlen(buffer); offset < sizeof (buffer); ++offset)
        {
            buffer[offset] = (offset == (sizeof (buffer) - 1)) ? '\0' : ' ';
        }
        fprintf(stdout, "%s%s\n", buffer, DESCRIPTION[index]);
        ++index;
    }
}

static void version(void)
{
    const cgbl_version_t *const version = cgbl_version();
    fprintf(stdout, "%u.%u-%x\n", version->major, version->minor, version->patch);
}

int main(int argc, char *argv[])
{
    int index = 0;
    const char *path = NULL;
    cgbl_error_e result = CGBL_SUCCESS;
    cgbl_option_t option = {
        .debug = false,
        .fullscreen = false,
        .scale = 2,
    };
    while ((index = getopt_long(argc, argv, "dfhs:v", OPTION, NULL)) != -1)
    {
        switch (index)
        {
            case 'd':
                option.debug = true;
                break;
            case 'f':
                option.fullscreen = true;
                break;
            case 'h':
                usage();
                return CGBL_SUCCESS;
            case 's':
                option.scale = strtol(optarg, NULL, 10);
                break;
            case 'v':
                version();
                return CGBL_SUCCESS;
            case '?':
            default:
                usage();
                return CGBL_FAILURE;
        }
    }
    for (index = optind; index < argc; ++index)
    {
        path = argv[index];
        break;
    }
    if ((result = cgbl_entry(path, &option)) != CGBL_SUCCESS)
    {
        fprintf(stderr, "%s\n", cgbl_error());
    }
    return result;
}
