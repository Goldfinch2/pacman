#include "settings.h"
#include "advmame_rc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

namespace settings
{
    char serialPort[256]  = "";
    char sounds[256]      = "sounds";
    char marquee[256]     = "artwork";
    char advmamerc[256]   = "";
    int  dvgRefreshRate   = 40;
    bool mirror           = false;

    // Default keyboard bindings — overridden by loadAdvmameRC()
    AdvInputAction upAction    = { {{ AdvInputBinding::KEYBOARD, 0, SDL_SCANCODE_UP    }} };
    AdvInputAction downAction  = { {{ AdvInputBinding::KEYBOARD, 0, SDL_SCANCODE_DOWN  }} };
    AdvInputAction leftAction  = { {{ AdvInputBinding::KEYBOARD, 0, SDL_SCANCODE_LEFT  }} };
    AdvInputAction rightAction = { {{ AdvInputBinding::KEYBOARD, 0, SDL_SCANCODE_RIGHT }} };
    AdvInputAction startAction = { {{ AdvInputBinding::KEYBOARD, 0, SDL_SCANCODE_RETURN }} };
    AdvInputAction coinAction  = { {{ AdvInputBinding::KEYBOARD, 0, SDL_SCANCODE_5     }} };
    AdvInputAction exitAction  = { {{ AdvInputBinding::KEYBOARD, 0, SDL_SCANCODE_ESCAPE }} };
}

// ─── Config file parser ──────────────────────────────────────────────────────

// Section tags:
//   [sdl] — keys in this section only apply when USE_DVG_RENDERER is NOT defined
//   [dvg] — keys in this section only apply when USE_DVG_RENDERER IS defined
//   No section or unknown section — keys always apply

enum Section { SECTION_GLOBAL, SECTION_SDL, SECTION_DVG };

static void trimTrailingWhitespace(char* s)
{
    int len = (int)strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' ||
                       s[len - 1] == '\r' || s[len - 1] == '\n'))
        s[--len] = '\0';
}

static bool parseBool(const char* val)
{
    return (strcmp(val, "true") == 0 || strcmp(val, "1") == 0 || strcmp(val, "yes") == 0);
}

static bool sectionActive(Section section)
{
#ifdef USE_DVG_RENDERER
    return (section == SECTION_GLOBAL || section == SECTION_DVG);
#else
    return (section == SECTION_GLOBAL || section == SECTION_SDL);
#endif
}

static void applyKey(const char* key, const char* value)
{
    if (strcmp(key, "sounds") == 0)
        strncpy(settings::sounds, value, sizeof(settings::sounds) - 1);
    else if (strcmp(key, "marquee") == 0)
        strncpy(settings::marquee, value, sizeof(settings::marquee) - 1);
    else if (strcmp(key, "advmamerc") == 0)
        strncpy(settings::advmamerc, value, sizeof(settings::advmamerc) - 1);
    else if (strcmp(key, "dvg_refresh_rate") == 0)
        settings::dvgRefreshRate = atoi(value);
    else if (strcmp(key, "mirror") == 0)
        settings::mirror = parseBool(value);
}

void settings::load(const char* filename)
{
    FILE* f = fopen(filename, "r");
    if (!f)
    {
        fprintf(stderr, "settings: could not open %s, using defaults\n", filename);
        loadAdvmameRC();
        return;
    }

    Section section = SECTION_GLOBAL;
    char line[1024];

    while (fgets(line, sizeof(line), f))
    {
        trimTrailingWhitespace(line);

        // Skip empty lines and comments
        const char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '#')
            continue;

        // Section header
        if (*p == '[')
        {
            if (strncmp(p, "[sdl]", 5) == 0)
                section = SECTION_SDL;
            else if (strncmp(p, "[dvg]", 5) == 0)
                section = SECTION_DVG;
            else
                section = SECTION_GLOBAL;
            continue;
        }

        if (!sectionActive(section))
            continue;

        // Parse "key value" — split on first whitespace
        char key[256] = {0};
        char value[768] = {0};
        int i = 0;
        while (*p && *p != ' ' && *p != '\t' && i < 255)
            key[i++] = *p++;
        key[i] = '\0';

        while (*p == ' ' || *p == '\t') p++;
        strncpy(value, p, sizeof(value) - 1);

        if (key[0] && value[0])
            applyKey(key, value);
    }

    fclose(f);
    fprintf(stderr, "settings: loaded %s\n", filename);

    loadAdvmameRC();
}

void settings::loadAdvmameRC()
{
    const char* rcPath = advmamerc[0] ? advmamerc : "/home/pi/.advance/advmame.rc";

    AdvmameRC rc;
    if (!advmameLoad(rcPath, rc)) {
        fprintf(stderr, "settings: cannot open '%s', using default bindings\n", rcPath);
        return;
    }

#ifdef USE_DVG_RENDERER
    if (rc.config.isDvgEnabled())
    {
        strncpy(serialPort, rc.config.rendererPort.c_str(), sizeof(serialPort) - 1);
        fprintf(stderr, "settings: serial port from advmame.rc: %s\n", serialPort);
    }
#endif

    static const struct { const char* name; AdvInputAction* action; } actionMap[] = {
        { "p1_up",    &upAction    },
        { "p1_down",  &downAction  },
        { "p1_left",  &leftAction  },
        { "p1_right", &rightAction },
        { "start1",   &startAction },
        { "coin1",    &coinAction  },
        { "ui_cancel",&exitAction  },
    };
    for (const auto& m : actionMap) {
        auto it = rc.inputActions.find(m.name);
        if (it != rc.inputActions.end())
            *m.action = it->second;
    }

    fprintf(stderr, "settings: loaded bindings from '%s'\n", rcPath);
}
