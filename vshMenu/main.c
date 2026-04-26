#include <stdio.h>
#include <string.h>
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

/* Define the module info section */
PSP_MODULE_INFO("VshCtrlSatelite", 0, 2, 2);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

struct {
    u32 cur_buttons;
    u32 button_on;
    int stop_flag;
    int menu_mode;
    int is_registered;
    int show_info;
} vshmenu;


void vshmenu_draw(void* frame){
    int w = 100, h = 70;
    int x = (480-w)/2;
    int y = (272-h)/2;
    ya2d_draw_rect(x, y, w, h, 0x80808000, 1);
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
        if( sceDisplayWaitVblankStart() < 0)
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

    SceUID thread_id = sceKernelCreateThread("VshMenu_Thread", (void*)KERNELIFY(TSRThread), 16 , 0x1000 , 0 , 0);
    sceKernelStartThread(thread_id, 0, 0);

    return 0;
}
