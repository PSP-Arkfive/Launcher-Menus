#ifndef VSHMENU_FONTS
#define VSHMENU_FONTS

#include <systemctrl_ark.h>

const char** font_list(int* nfonts);
void* font_load(ArkMenuConf* conf);
void release_font(void);

#endif