#include <systemctrl.h>
#include <systemctrl_se.h>
#include <sys/stat.h>
#include "common.h"
#include "entry.h"

using namespace common;

static ArkMenuConf config;

struct tm today;

static ARKConfig _ark_conf;
ARKConfig* ark_config = &_ark_conf;

static SEConfig _se_conf;
SEConfigARK* se_config = (SEConfigARK*)&_se_conf;


void common::resetConf(){
    memset(&config, 0, sizeof(config));
    config.font = 1;
    config.sort_entries = 1;
    config.show_recovery = 1;
    config.text_glow = 3;
    config.screensaver = 2;
    config.redirect_ms0 = 1;
    config.menusize = 2;
    config.browser_icon0 = 1;
}

void loadConfig(){
    SceUID fp = sceIoOpen(MENU_SETTINGS, PSP_O_RDONLY, 0777);
    if (fp < 0){
        resetConf();
        return;
    }   
    memset(&config, 0, sizeof(ArkMenuConf));
    sceIoLseek(fp, 0, PSP_SEEK_SET);
    sceIoRead(fp, &config, sizeof(ArkMenuConf));
    sceIoClose(fp);
}

void common::loadConf() {
    loadConfig();
}

void common::setArgs(int c, char** v){
    argc = c;
    argv = v;
}

bool common::fileExists(const std::string &path){
    struct stat sb;
    return (stat(path.c_str(), &sb) == 0 && S_ISREG(sb.st_mode));
}

void common::saveConf() {
    SceUID fp = sceIoOpen(MENU_SETTINGS, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
    sceIoWrite(fp, &config, sizeof(ArkMenuConf));
    sceIoClose(fp);
}

void common::loadData(){
    loadConfig();
    PBPHeader header;
    
    SceUID fp = sceIoOpen(argv[0], PSP_O_RDONLY, 0777);
    sceIoRead(fp, &header, sizeof(PBPHeader));
    sceIoClose(fp);
    
    background = ya2d_load_PNG_file_offset(argv[0], YA2D_PLACE_VRAM, header.pic1_offset);
    noicon = ya2d_load_PNG_file_offset(argv[0], YA2D_PLACE_VRAM, header.icon0_offset);
}

void common::deleteData(){
    ya2d_free_texture(background);
    ya2d_free_texture(noicon);
}

ya2d_texture* common::getBG(){
    return background;
}

ya2d_texture* common::getNoIcon(){
    return noicon;
}

ArkMenuConf* common::getConf() {
    return &config;
}


void common::printText(float x, float y, const char *text, u32 color){
    tinyFontPrintTextScreen(msx, x, y, text, color, NULL);
}

void common::clearScreen(){
    ya2d_start_drawing();
    ya2d_clear_screen(CLEAR_COLOR);
}

void common::flip(){
    ya2d_finish_drawing();
    ya2d_swapbuffers();
    tinyFontSwapBuffers();
};

void common::rebootMenu(){

    struct SceKernelLoadExecVSHParam param;
    memset(&param, 0, sizeof(SceKernelLoadExecVSHParam));

    char path[256];
    strcpy(path, ark_config->arkpath);
    strcat(path, ARK_XMENU);

    int runlevel = 0x141;
    
    param.args = strlen(path) + 1;
    param.argp = path;
    param.key = "game";
    sctrlKernelLoadExecVSHWithApitype(runlevel, path, &param);
}