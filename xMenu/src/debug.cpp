#include "debug.h"
#include "common.h"

void debugScreen(const char* text, uint16_t w, uint16_t h){
    common::clearScreen();
    if ((w && h) == NULL)
        tinyFontPrintTextScreen(msx, 0, 0, text, WHITE_COLOR, NULL);
    else
        tinyFontPrintTextScreen(msx, w, h, text, WHITE_COLOR, NULL);
    common::flip();
}


void debugFile(const char* text){
    SceUID fp = sceIoOpen("DEBUG.TXT", PSP_O_WRONLY|PSP_O_CREAT|PSP_O_APPEND, 0777);
    sceIoWrite(fp, text, strlen(text));
    sceIoClose(fp);
}
