#include "debug.h"

void debugScreen(const char* text, uint16_t w, uint16_t h){
    clearScreen(CLEAR_COLOR);
    if((w && h) == NULL)
        printTextScreen(0, 0, text, WHITE_COLOR);
    else
        //printTextScreen(180, 130, text, WHITE_COLOR);
        printTextScreen(w, h, text, WHITE_COLOR);
    flipScreen();
}


void debugFile(const char* text){
    SceUID fp = sceIoOpen("DEBUG.TXT", PSP_O_WRONLY|PSP_O_CREAT|PSP_O_APPEND, 0777);
    sceIoWrite(fp, text, strlen(text));
    sceIoClose(fp);
}
