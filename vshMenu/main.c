#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <pspkernel.h>
#include <psputility.h>
#include <pspdisplay.h>
#include <psprtc.h>
#include <psppower.h>
#include <pspgu.h>

#include <cfwmacros.h>
#include <vshctrl.h>
#include <kubridge.h>
#include <systemctrl.h>
#include <systemctrl_ark.h>
#include <ya2d.h>
#include <tinyfont.h>

#include "fonts.h"

/* Define the module info section */
PSP_MODULE_INFO("VshCtrlSatelite", 0, 2, 2);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU);


#define CLEAR_COLOR 0x00000000
#define WHITE_COLOR 0xFFFFFFFF
#define BLACK_COLOR 0xFF000000
#define GRAY_COLOR 0xFFCCCCCC
#define BLUE_COLOR 0x00FF0000
#define YELLOW_COLOR 0x00FFFF00
#define GREEN_COLOR 0x0000FF00
#define RED_COLOR 0x000000FF

#define PSP_LINE_SIZE 512
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

struct {
    u32 cur_buttons;
    u32 button_on;
    int stop_flag;
    int menu_mode;
    int is_registered;
    int show_info;
} vshmenu;

SceUID thread_id = -1;

enum {
    OPT_SHUTDOWN,
    OPT_SUSPEND,
    OPT_HARDRESET,
    OPT_SOFTRESET,
    OPT_SUBMENU,
    OPT_EXIT,
};

const char* main_menu_opts[] = {
    "Shutdown Device",
    "Suspend Device",
    "Reset Device",
    "Reset VSH",
    "Options",
    "Exit"
};
const int main_menu_nopts = NELEMS(main_menu_opts);

enum {
    OPT_BGCOLOR,
    OPT_BGALPHA,
    OPT_TEXTCOLOR,
    OPT_TEXTFONT,
    OPT_SAVE,
    OPT_CANCEL,
};
const char* options_menu_opts[] = {
    "Background Color: ",
    "Background Transparency: ",
    "Text Color: ",
    "Text Font: ",
    "Accept",
    "Cancel"
};
const int options_menu_nopts = NELEMS(options_menu_opts);

u32 colors[] = {
    0x00808000,
    0x000000ff,
    0x0000a5ff,
    0x0000e6e6,
    0x0000ff00,
    0x00ff0000,
    0x0082004b,
    0x00ee82ee,
    0x00cbc0ff,
    0x00993366,
    0x00808000,
    0x00cccc00,
    0x00737373,
    0x00000000,
    0x00ffffff,
};
const char* colors_str[] = {
    "Default",
    "Red",
    "Orange",
    "Yellow",
    "Green",
    "Blue",
    "Indigo",
    "Violet",
    "Pink",
    "Purple",
    "Teal",
    "Aqua",
    "Grey",
    "Black",
    "White",
};
const static int ncolors = NELEMS(colors);

u32 bgalphas[] = {
    0xFF, 0xA0, 0x80, 0x50
};
const char* bgalphas_str[] = {
    "None", "Little", "Normal", "High"
};
const static int nalphas = NELEMS(bgalphas);

typedef struct{
    const char** opts;
    int* cur;
} OptionsMenu;

const char** cur_menu_opts = main_menu_opts;
int cur_menu_nopts = main_menu_nopts;
int cur_entry = 0;
int window_w = 0;
int window_h = 0;
int cur_bgcolor = 0;
int cur_bgalpha = nalphas-1;
int cur_textcolor = ncolors-1;
int cur_font = 0;
int nfonts = 1;
OptionsMenu* menu_subopts = NULL;

OptionsMenu options_menu_curopts[] = {
    {colors_str, &cur_bgcolor},
    {bgalphas_str, &cur_bgalpha},
    {colors_str, &cur_textcolor},
    {NULL, &cur_font},
    {NULL, NULL}, {NULL, NULL}
};

int main_menu_ctrl(u32 button_on);
int options_menu_ctrl(u32 button_on);
int (*menu_ctrl)(u32 button_on) = main_menu_ctrl;

TinyFontState header_state;
TinyFontState cur_entry_state;
u8* font = msx;

ArkMenuConf conf;
ARKConfig ark_config;

void loadFont(){
    release_font();
    conf.vsh_font = cur_font;
    if (cur_font){
        font = font_load(&conf);
    }
    else {
        font = msx;
    }
}

void loadSettings(){
    char path[ARK_PATH_SIZE];

    memset(&ark_config, 0, sizeof(ARKConfig));
    memset(&conf, 0, sizeof(ArkMenuConf));
    sctrlArkGetConfig(&ark_config);

    strcpy(path, ark_config.arkpath);
    strcat(path, MENU_SETTINGS);

    SceUID fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    int res = sceIoRead(fd, &conf, sizeof(ArkMenuConf));
    sceIoClose(fd);

    options_menu_curopts[OPT_TEXTFONT].opts = font_list(&nfonts);

    if (res == sizeof(ArkMenuConf)){
        cur_bgcolor = conf.vshgu_bgcolor;
        cur_bgalpha = conf.vshgu_bgalpha;
        cur_textcolor = conf.vshgu_textcolor;
        cur_font = conf.vsh_font;
        loadFont();
    }
}

void saveSettings(){
    SceUID fd;
    char path[ARK_PATH_SIZE];

    memset(&conf, 0, sizeof(ArkMenuConf));
    sctrlArkGetConfig(&ark_config);

    strcpy(path, ark_config.arkpath);
    strcat(path, MENU_SETTINGS);

    fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    sceIoRead(fd, &conf, sizeof(ArkMenuConf));
    sceIoClose(fd);

    conf.vshgu_bgcolor = cur_bgcolor;
    conf.vshgu_bgalpha = cur_bgalpha;
    conf.vshgu_textcolor = cur_textcolor;
    conf.vsh_font = cur_font;

    fd = sceIoOpen(path, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
    sceIoWrite(fd, &conf, sizeof(ArkMenuConf));
    sceIoClose(fd);
}

int calcWindowWidth(){
    int max_w = 50;
    for (int i=0; i<cur_menu_nopts; i++){
        int sw = 8*strlen(cur_menu_opts[i]);
        if (menu_subopts && menu_subopts[i].opts && menu_subopts[i].cur){
            int idx = *(menu_subopts[i].cur);
            sw += 8*strlen(menu_subopts[i].opts[idx]);
        }
        if (sw > max_w) max_w = sw;
    }
    int res = max_w + 20;
    if (res > 480) res = 480;
    return res;
}

int calcWindowHeight(){
    int res = 8*cur_menu_nopts + 30;
    if (res > 272) res = 272;
    return res;
}

void switchMainMenu(){
    cur_menu_opts = main_menu_opts;
    cur_menu_nopts = main_menu_nopts;
    menu_subopts = NULL;
    cur_entry = 0;
    menu_ctrl = main_menu_ctrl;
    window_w = calcWindowWidth();
    window_h = calcWindowHeight();
    vshmenu.menu_mode = 1;
}

void switchOptionsMenu(){
    cur_menu_opts = options_menu_opts;
    cur_menu_nopts = options_menu_nopts;
    menu_subopts = options_menu_curopts;
    cur_entry = 0;
    menu_ctrl = options_menu_ctrl;
    window_w = calcWindowWidth();
    window_h = calcWindowHeight();
    vshmenu.menu_mode = 1;
}

void vshmenu_draw(void* frame){

    if (vshmenu.stop_flag) return;

    int w = window_w, h = window_h;
    int x = (480-w)/2;
    int y = (272-h)/2;
    u32 bgcolor = (bgalphas[cur_bgalpha]<<24) | colors[cur_bgcolor];

    ya2d_draw_rect(x+15, y-15, 88, 15, 0x8000ff00, 1);
    ya2d_draw_rect(x, y, w, h, bgcolor, 1);

    header_state.ix = x+20;
    tinyFontPrintTextScreenBuf(frame, font, header_state.ix, y-12, "VSHGU Menu", WHITE_COLOR, &header_state);

    for (int i=0; i<cur_menu_nopts; i++){
        char text[128];
        strcpy(text, cur_menu_opts[i]);
        if (menu_subopts && menu_subopts[i].opts){
            char* opt_txt = menu_subopts[i].opts[*(menu_subopts[i].cur)];
            /*
            int padding = (h - 10)/8 - 8*strlen(opt_txt);
            for (int p=0; p<padding; p++) strcat(text, " ");
            */
            strcat(text, opt_txt);
        }
        cur_entry_state.glow = (i==cur_entry)?1:0;
        tinyFontPrintTextScreenBuf(frame, font, x+10, y+(10*(i+1)), text, colors[cur_textcolor], &cur_entry_state);
    }
}

int EatKey(SceCtrlData *pad_data, int count)
{
    // buttons check
    vshmenu.button_on   = ~vshmenu.cur_buttons & pad_data[0].Buttons;
    vshmenu.cur_buttons = pad_data[0].Buttons;

    // mask buttons for LOCK VSH control
    for (int i=0; i<count; i++) {
        pad_data[i].Buttons &= ~(
                PSP_CTRL_SELECT|PSP_CTRL_START|
                PSP_CTRL_UP|PSP_CTRL_RIGHT|PSP_CTRL_DOWN|PSP_CTRL_LEFT|
                PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|
                PSP_CTRL_TRIANGLE|PSP_CTRL_CIRCLE|PSP_CTRL_CROSS|PSP_CTRL_SQUARE|
                PSP_CTRL_HOME|PSP_CTRL_NOTE);

    }

    return 0;
}

int main_menu_ctrl(u32 button_on)
{
    if ((button_on & PSP_CTRL_SELECT) || (button_on & PSP_CTRL_HOME)) {
        return 1;
    }
    else if (button_on & PSP_CTRL_DOWN){
        if (cur_entry < cur_menu_nopts-1) cur_entry++;
        else cur_entry = 0;
    }
    else if (button_on & PSP_CTRL_UP){
        if (cur_entry > 0) cur_entry--;
        else cur_entry = cur_menu_nopts-1;
    }
    else if (button_on & PSP_CTRL_CROSS){
        switch (cur_entry){
            case OPT_SHUTDOWN: scePowerRequestStandby(); break;
            case OPT_SUSPEND: scePowerRequestSuspend(); break;
            case OPT_HARDRESET: scePowerRequestColdReset(0); break;
            case OPT_SOFTRESET: sctrlKernelExitVSH(NULL); break;
            case OPT_SUBMENU: switchOptionsMenu(); break;
            case OPT_EXIT: return 1;
        }
    }
    return 0; // continue
}

int options_menu_ctrl(u32 button_on)
{
    if ((button_on & PSP_CTRL_SELECT) || (button_on & PSP_CTRL_HOME)) {
        return 1;
    }
    else if (button_on & PSP_CTRL_DOWN){
        if (cur_entry < cur_menu_nopts-1) cur_entry++;
        else cur_entry = 0;
    }
    else if (button_on & PSP_CTRL_UP){
        if (cur_entry > 0) cur_entry--;
        else cur_entry = cur_menu_nopts-1;
    }
    else if (button_on & PSP_CTRL_RIGHT){
        switch (cur_entry){
            case OPT_BGCOLOR:
                if (cur_bgcolor < ncolors-1) cur_bgcolor++;
                else cur_bgcolor = 0;
                break;
            case OPT_BGALPHA:
                if (cur_bgalpha < nalphas-1) cur_bgalpha++;
                else cur_bgalpha = 0;
                break;
            case OPT_TEXTCOLOR:
                if (cur_textcolor < ncolors-1) cur_textcolor++;
                else cur_textcolor = 0;
                break;
            case OPT_TEXTFONT:
                if (cur_font < nfonts-1) cur_font++;
                else cur_font = 0;
                loadFont();
                break;
        }
        window_w = calcWindowWidth();
        window_h = calcWindowHeight();
    }
    else if (button_on & PSP_CTRL_LEFT){
        switch (cur_entry){
            case OPT_BGCOLOR:
                if (cur_bgcolor > 0) cur_bgcolor--;
                else cur_bgcolor = ncolors-1;
                break;
            case OPT_BGALPHA:
                if (cur_bgalpha > 0) cur_bgalpha--;
                else cur_bgalpha = nalphas-1;
                break;
            case OPT_TEXTCOLOR:
                if (cur_textcolor > 0) cur_textcolor--;
                else cur_textcolor = ncolors-1;
                break;
            case OPT_TEXTFONT:
                if (cur_font > 0) cur_font--;
                else cur_font = nfonts-1;
                loadFont();
                break;
        }
        window_w = calcWindowWidth();
        window_h = calcWindowHeight();
    }
    else if (button_on & PSP_CTRL_CROSS){
        switch (cur_entry){
            case OPT_BGCOLOR: break;
            case OPT_BGALPHA: break;
            case OPT_TEXTCOLOR: break;
            case OPT_TEXTFONT: break;
            case OPT_SAVE: saveSettings();
            case OPT_CANCEL: switchMainMenu();
        }
    }
    return 0; // continue
}

static void button_func(void)
{
    // menu controll
    switch (vshmenu.menu_mode) {
        case 0:    
            if ((vshmenu.cur_buttons & ALL_CTRL) == 0) {
                vshmenu.menu_mode = 1;
            }
            break;
        case 1:
            if (menu_ctrl(vshmenu.button_on))
				vshmenu.menu_mode = 2;
            break;
		case 2:
			if ((vshmenu.cur_buttons & ALL_CTRL) == 0)
				vshmenu.stop_flag = 1;
			break;
    }
}

int TSRThread(SceSize args, void *argp)
{
    sceKernelChangeThreadPriority(0, 8);
    vctrlVSHRegisterVshMenu(EatKey);
    vctrlVSHRegisterVshGuMenu(vshmenu_draw);

    vshmenu.is_registered = 1;
    while (!vshmenu.stop_flag) {
        if (sceDisplayWaitVblankStart() < 0)
            break; // end of VSH ?

        button_func();
    }
    vshmenu.is_registered = 0;

    vctrlVSHExitVSHMenu(NULL, NULL, 0);
    return sceKernelExitDeleteThread(0);
}

int module_start(int argc, void* argv){
    memset(&vshmenu, 0, sizeof(vshmenu));
    vshmenu.cur_buttons = 0xFFFFFFFF;

    header_state.scroll = 1;
    header_state.sk = 150;
    window_w = calcWindowWidth();
    window_h = calcWindowHeight();
    loadSettings();

    thread_id = sceKernelCreateThread("VshMenu_Thread", TSRThread, 16, 0x1000, PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, 0);
    sceKernelStartThread(thread_id, 0, 0);

    return 0;
}

int module_stop(){
    vshmenu.stop_flag = 1;
    sceKernelWaitThreadEnd(thread_id, NULL);

    return 0;
}

void _exit(){
    sceKernelStopUnloadSelfModule(0, NULL, NULL, NULL);
}
