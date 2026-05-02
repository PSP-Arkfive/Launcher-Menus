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
#include <tinyfont.h>

#include "debug.h"

#define THREAD_DELAY 1000

#define CLEAR_COLOR 0x00000000
#define WHITE_COLOR 0xFFFFFFFF
#define RED_COLOR    0x000000FF
#define GREEN_COLOR  0xFF00FF00
#define YELLOW_COLOR 0x00FFFF00

extern SEConfigARK* se_config;
extern ARKConfig* ark_config;

namespace common{

    static int argc;
    static char** argv;
    static ya2d_texture* background;
    static ya2d_texture* noicon;

    extern void setArgs(int c, char** v);
    extern bool fileExists(const std::string &path);
    extern void loadData();
    extern void deleteData();
    extern ya2d_texture* getBG();
    extern ya2d_texture* getNoIcon();
    extern void printText(float x, float y, const char *text, u32 color = WHITE_COLOR);
    extern void clearScreen();
    extern void flip();
    extern void saveConf();
    extern ArkMenuConf* getConf();
    extern void resetConf();
    extern void loadConf();
    extern void rebootMenu();
}

#endif
