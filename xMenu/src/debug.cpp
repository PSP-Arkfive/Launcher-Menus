#include "debug.h"
#include "common.h"

void debugScreen(const char* text, uint16_t x, uint16_t y){
    common::clearScreen();
    common::printText(x, y, text, WHITE_COLOR, NULL);
    common::flip();
}


void debugFile(const char* text){
    SceUID fp = sceIoOpen("DEBUG.TXT", PSP_O_WRONLY|PSP_O_CREAT|PSP_O_APPEND, 0777);
    sceIoWrite(fp, text, strlen(text));
    sceIoClose(fp);
}
