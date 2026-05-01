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

const char** cur_menu_opts = main_menu_opts;
int cur_menu_nopts = main_menu_nopts;
int cur_entry = 0;
int window_w = 0;
int window_h = 0;

TinyFontState header_state;
TinyFontState cur_entry_state;

extern u8 msx[];

int calcWindowWidth(){
    int max_w = 50;
    for (int i=0; i<cur_menu_nopts; i++){
        int sw = 8*strlen(cur_menu_opts[i]);
        if (sw > max_w) max_w = sw;
    }
    return max_w + 20;
}

int calcWindowHeight(){
    return 8*cur_menu_nopts + 30; 
}

void vshmenu_draw(void* frame){

    if (vshmenu.stop_flag) return;

    int w = window_w, h = window_h;
    int x = (480-w)/2;
    int y = (272-h)/2;
    

    ya2d_draw_rect(x+15, y-15, 88, 15, 0x8000ff00, 1);
    ya2d_draw_rect(x, y, w, h, 0x80808000, 1);

    header_state.ix = x+20;
    tinyFontPrintText(frame, msx, header_state.ix, y-12, "VSHGU Menu", WHITE_COLOR, &header_state);

    for (int i=0; i<cur_menu_nopts; i++){
        cur_entry_state.glow = (i==cur_entry)?1:0;
        tinyFontPrintText(frame, msx, x+10, y+(10*(i+1)), cur_menu_opts[i], WHITE_COLOR, &cur_entry_state);
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

int menu_ctrl(u32 button_on)
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
                case OPT_SUBMENU: break;
                case OPT_EXIT: return 1;
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
