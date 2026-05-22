#ifndef DEBUG_H
#define DEBUG_H

#include <cstdio>
#include <cstring>
#include <pspiofilemgr.h>

void debugScreen(const char* text, uint16_t x, uint16_t y);
void debugFile(const char* text);

#endif
