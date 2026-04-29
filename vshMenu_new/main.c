#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <pspkernel.h>
#include <psputility.h>
#include <pspdisplay.h>
#include <psprtc.h>
#include <psppower.h>

#include <systemctrl_ark.h>
#include <cfwmacros.h>
#include <vshctrl.h>
#include <kubridge.h>
#include <systemctrl.h>
#include <ya2d.h>
#include <intraFont.h>

/* Define the module info section */
PSP_MODULE_INFO("VshCtrlSatelite", 0, 2, 2);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);


#define CLEAR_COLOR 0x00000000
#define WHITE_COLOR 0xFFFFFFFF
#define BLACK_COLOR 0xFF000000
#define GRAY_COLOR 0xFFCCCCCC
#define BLUE_COLOR 0x00FF0000
#define YELLOW_COLOR 0x00FFFF00
#define GREEN_COLOR 0x0000FF00
#define RED_COLOR 0x000000FF

struct {
    u32 cur_buttons;
    u32 button_on;
    int stop_flag;
    int menu_mode;
    int is_registered;
    int show_info;
} vshmenu;

typedef struct TextScroll{
    float x;
    float y;
    float tmp;
    float w;
}TextScroll;

intraFont* font = NULL;
SceUID thread_id = -1;

const char* main_menu_opts[] = {
    "VSHGU Menu Options",
    "Shutdown Device",
    "Suspend Device",
    "Reset Device",
    "Reset VSH",
    "Exit"
};
const int main_menu_nopts = NELEMS(main_menu_opts);

const char** cur_menu_opts = main_menu_opts;
int cur_menu_nopts = main_menu_nopts;
int cur_entry = 0;



void printText(float x, float y, const char* text, u32 color, float size, int glow, TextScroll* scroll){

    if (font == NULL)
        return;

    intraFont* textFont = font;

    u32 secondColor = BLACK_COLOR;
    u32 arg5 = INTRAFONT_WIDTH_VAR;
    
    if (glow){
        int val = 0;
        float t = (float)((float)(clock() % CLOCKS_PER_SEC)) / ((float)CLOCKS_PER_SEC);
        if (glow == 1) {
            val = (t < 0.5f) ? t*311 : (1.0f-t)*311;
        }
        else if (glow == 2) {
            val = (t < 0.5f) ? t*411 : (1.0f-t)*411;
        }
        else {
            val = (t < 0.5f) ? t*511 : (1.0f-t)*511;
        }
        secondColor = (0xFF<<24)+(val<<16)+(val<<8)+(val);
    }
    if (scroll){
        arg5 = INTRAFONT_SCROLL_LEFT;
    }
    
    intraFontSetStyle(textFont, size, color, secondColor, 0.f, arg5);

    if (scroll){
        if (x != scroll->x || y != scroll->y){
            scroll->x = x;
            scroll->tmp = x;
            scroll->y = y;
        }
        if (scroll->w <= 0 || scroll->w >= 480) scroll->w = 200;
        scroll->tmp = intraFontPrintColumn(textFont, scroll->tmp, y, scroll->w, text);
    }
    else
        intraFontPrint(textFont, x, y, text);   
}

void vshmenu_draw(void* frame){
    int w = 300, h = 100;
    int x = (480-w)/2;
    int y = (272-h)/2;
    ya2d_draw_rect(x, y, w, h, 0x80808000, 1);

    intraFontActivate(font);
    printText(x+20, y-15, "VSHGU Menu", WHITE_COLOR, 1.f, 0, NULL);    
    printText(x+20, y+15, "Opt 1", WHITE_COLOR, 0.6f, 0, NULL);
    printText(x+20, y+30, "Opt 2", WHITE_COLOR, 0.6f, 0, NULL);
    printText(x+20, y+45, "Opt 3", WHITE_COLOR, 0.6f, 0, NULL);
    sceDisplayWaitVblankStart();

    /*for (int i=0; i<cur_menu_nopts; i++){
        printText(x+10, y+(10*(i+1)), cur_menu_opts[i], WHITE_COLOR, 0.7f, (i==cur_entry)?3:0, NULL);
        //intraFontPrint(font, x+10, y+(20*(i+1)), cur_menu_opts[i]);
    }*/
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

int menu_ctrl(u32 button_on)
{
    if ((button_on & PSP_CTRL_SELECT) || (button_on & PSP_CTRL_HOME)) {
        return 1;
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

    //intraFontInit();
    font = intraFontLoad("flash0:/font/ltn0.pgf", INTRAFONT_CACHE_MED);
    //intraFontActivate(font);

    thread_id = sceKernelCreateThread("VshMenu_Thread", TSRThread, 16, 0x1000, PSP_THREAD_ATTR_VSH|PSP_THREAD_ATTR_VFPU, 0);
    sceKernelStartThread(thread_id, 0, 0);

    return 0;
}

int module_stop(){
    vshmenu.stop_flag = 1;
    intraFontUnload(font);
    //intraFontShutdown();
    sceKernelWaitThreadEnd(thread_id, NULL);
    vctrlVSHExitVSHMenu(NULL, NULL, 0);
    return 0;
}

void _exit(){
    module_stop();
    sceKernelStopUnloadSelfModule(0, NULL, NULL, NULL);
}
