#include <sys/socket.h>
#include <pspkernel.h>
#include <psputility.h>
#include <sstream>

#include "debug.h"
#include "common.h"
#include "menu.h"

PSP_MODULE_INFO("XMENU", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU);


using namespace std;

string startup_txt = "Loading ";

static uint8_t dots = 0;

static volatile bool loading = true;

int startup_thread(int argc, void* argp){
    stringstream startup_runner;
    Menu::fadeIn();
    while (loading){
        common::clearScreen();
        ya2d_draw_texture(common::background, 0, 0);
        common::printText(180, 130, startup_runner.str().c_str(), WHITE_COLOR, NULL);
        common::flip();
        dots++;
        if(dots>3) {
            startup_runner.str(startup_txt);
            dots=0;
        }
        else {
            startup_runner.str(startup_txt + string(dots, '.'));
        }
        sceKernelDelayThread(200000);
    }
    return 0;
}

int main(int argc, char** argv){

    common::argc = argc;
    common::argv = argv;

    ya2d_init();
    intraFontInit();
    ya2d_set_vsync(1);

    // load data
    common::loadData();

    // start loading screen thread
    loading = true;
    int thid = sceKernelCreateThread("xmenu bootup", (SceKernelThreadEntry)startup_thread, 10, 2048, PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, NULL);
    sceKernelStartThread(thid, 0, NULL);

    // initialize menu, scanning eboots
    Menu* menu = new Menu();

    // finish loading screen thread
    loading = false;
    sceKernelWaitThreadEnd(thid, NULL);
    sceKernelDeleteThread(thid);

    // run menu
    menu->run();

    // cleanup
    delete menu;
    common::deleteData();
    intraFontShutdown();
    ya2d_shutdown();

    // exit
    sceKernelExitGame();
    return 0;
}
