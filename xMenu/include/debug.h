#ifndef DEBUG_H
#define DEBUG_H

#include <cstdio>
#include <cstring>
#include <pspiofilemgr.h>

#include <mini2d.h>

void debugScreen(const char* text, uint16_t w, uint16_t h);
void debugFile(const char* text);

#endif
