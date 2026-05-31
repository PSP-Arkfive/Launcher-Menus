#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <pspgu.h>
#include <pspdisplay.h>
#include <pspkernel.h>

#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_ark.h>
#include <ya2d.h>
#include <intraFont.h>

#include "debug.h"

#define THREAD_DELAY 1000

// Colors
enum pspcolors {
    CLEAR_COLOR =        0x00000000,
    RED_COLOR =          0xFF0000FF,
    GREEN_COLOR =        0xFF00FF00,
    BLUE_COLOR =         0xFFFFFF00,
    YELLOW_COLOR =       0xFF00FFFF,
    WHITE_COLOR =        0xFFFFFFFF,
    LITEGRAY_COLOR =     0xFFBFBFBF,
    GRAY_COLOR =         0x007F7F7F,
    DARKGRAY_COLOR =     0xFF3F3F3F,
    BLACK_COLOR =        0xFF000000,
};

typedef struct TextState{
    int scroll;
    float x;
    float y;
    float tmp;
    float w;
    int glow;
} TextState;

namespace common{

    extern int argc;
    extern char** argv;
    extern ArkMenuConf config;
    extern ARKConfig ark_config;
    extern ya2d_texture* background;
    extern ya2d_texture* noicon;
    extern intraFont* font;
    extern intraFont* altFont;

    extern bool fileExists(const std::string &path);
    extern void loadData();
    extern void deleteData();
    extern void loadLanguageFont(const char* fontfile);
    extern void* readFile(const char* filename, unsigned* size);
    extern void* readFromPKG(const char* filename, unsigned* size, const char* pkgpath);
    extern SceOff findPkgOffset(const char* filename, unsigned* size, const char* pkgpath);
    extern int calcTextWidth(const char* text);
    extern void printText(float x, float y, const char *text, u32 color = WHITE_COLOR, TextState* state = NULL);
    extern void clearScreen();
    extern void flip();
    extern void saveConf();
    extern void resetConf();
    extern void loadConf();
    extern void rebootMenu();
}

#endif
