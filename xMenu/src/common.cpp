#include <math.h>
#include <sys/stat.h>
#include <pspsdk.h>
#include <psputility.h>

#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_ark.h>

#include "common.h"
#include "entry.h"
#include "lang.h"

using namespace common;

struct tm today;

// custom languages
static char* lang_files[] = {
    "en",
    "es",
    "de",
    "fr",
    "pt",
    "it",
    "nl",
    "ru",
    "ukr",
    "ro",
    "lat",
    "jp",
    "ko",
    "cht",
    "chs",
    "pol",
    "latgr",
    "tr",
    //"grk",
    //"thai",
};

// system languages
static char* system_lang[] = {
    "jp",
    "en",
    "fr",
    "es",
    "de",
    "it",
    "nl",
    "pt",
    "ru",
    "ko",
    "cht",
    "chs"
};

int common::argc;
char** common::argv;
ArkMenuConf common::config;
ARKConfig common::ark_config;
ya2d_texture* common::background;
ya2d_texture* common::noicon;
intraFont* common::font;
intraFont* common::altFont;


static char* getLangFile(){
    static char file[20];
    char* lang = NULL;
    if (config.syslang){
        int id; sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &id);
        lang = system_lang[id];
    }
    else {
        lang = lang_files[config.language];
    }
    sprintf(file, "lang_%s.json", lang);
    return file;
}

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
    sctrlArkGetConfig(&ark_config);
    loadConfig();
    
    PBPHeader header;
    SceUID fp = sceIoOpen(argv[0], PSP_O_RDONLY, 0777);
    sceIoRead(fp, &header, sizeof(PBPHeader));
    sceIoClose(fp);

    font = intraFontLoad("flash0:/font/ltn0.pgf", INTRAFONT_CACHE_ALL);
    intraFontSetEncoding(font, INTRAFONT_STRING_UTF8);
    
    background = ya2d_load_PNG_file_offset(argv[0], YA2D_PLACE_RAM, header.pic1_offset);
    noicon = ya2d_load_PNG_file_offset(argv[0], YA2D_PLACE_RAM, header.icon0_offset);

    Translations::loadLanguage(getLangFile());

}

void common::deleteData(){
    ya2d_free_texture(background);
    ya2d_free_texture(noicon);
    intraFontUnload(font);
    if (altFont) intraFontUnload(altFont);
}

void common::loadLanguageFont(const char* fontfile){
    intraFont* newfont = NULL;
    SceIoStat stat;
    if (sceIoGetstat(fontfile, &stat)>=0){
        newfont = intraFontLoad(fontfile, INTRAFONT_CACHE_ALL);
    }
    else {
        unsigned size = 0, offset = 0;
        string lang_pkg = string(ark_config.arkpath)+"LANG.ARK";
        offset = findPkgOffset(fontfile, &size, lang_pkg.c_str());
        if (offset && size){
            newfont = intraFontLoadEx(fontfile, INTRAFONT_CACHE_ALL, offset, size);
        }
    }
    if (newfont){
        altFont = font;
        font = newfont;
        intraFontSetEncoding(newfont, INTRAFONT_STRING_UTF8);
        intraFontSetAltFont(font, altFont);
    }
}

void* common::readFile(const char* filename, unsigned* size){
    SceUID fd = sceIoOpen(filename, PSP_O_RDONLY, 0777);
    if (fd < 0) return NULL;

    *size = sceIoLseek32(fd, 0, PSP_SEEK_END);

    if (*size == 0){
        sceIoClose(fd);
        return NULL;
    }

    sceIoLseek32(fd, 0, PSP_SEEK_SET);

    void* buf = malloc(*size);
    sceIoRead(fd, buf, *size);
    sceIoClose(fd);

    return buf;
}

SceOff common::findPkgOffset(const char* filename, unsigned* size, const char* pkgpath) {

    int pkg = sceIoOpen(pkgpath, PSP_O_RDONLY, 0777);
    if (pkg < 0)
        return 0;
     
    unsigned pkgsize = sceIoLseek32(pkg, 0, PSP_SEEK_END);
    unsigned size2 = 0;
     
    sceIoLseek32(pkg, 0, PSP_SEEK_SET);

    if (size != NULL)
        *size = 0;

    unsigned offset = 0;
    char name[64];
           
    while (offset != 0xFFFFFFFF) {
        sceIoRead(pkg, &offset, 4);
        if (offset == 0xFFFFFFFF) {
        	sceIoClose(pkg);
        	return 0;
        }
        unsigned namelength;
        sceIoRead(pkg, &namelength, 4);
        sceIoRead(pkg, name, namelength+1);
        		   
        if (!strncmp(name, filename, namelength)){
        	sceIoRead(pkg, &size2, 4);
    
        	if (size2 == 0xFFFFFFFF)
        		size2 = pkgsize;

        	if (size != NULL)
        		*size = size2 - offset;
     
        	sceIoClose(pkg);
        	return offset;
        }
    }
    return 0;
}

void* common::readFromPKG(const char* filename, unsigned* size, const char* pkgpath){

    unsigned mySize, offset;
    void* data;
    SceUID fp;

    offset = findPkgOffset(filename, size, pkgpath);
    if (offset == 0) goto err_readFromPKG;
    
    fp = sceIoOpen(pkgpath, PSP_O_RDONLY, 0777);
    if (fp < 0) goto err_readFromPKG;
    
    data = malloc(*size);
    sceIoLseek(fp, offset, PSP_SEEK_SET);
    sceIoRead(fp, data, *size);
    sceIoClose(fp);
    return data;

    err_readFromPKG:
    *size = 0;
    return NULL;
}

int common::calcTextWidth(const char* text){
    intraFontSetStyle(font, 0.5f, 0, 0, 0.f, INTRAFONT_WIDTH_VAR);
    float w = intraFontMeasureText(font, text);
    return (int)ceil(w);
}

void common::printText(float x, float y, const char *text, u32 color, TextState* state){
    if (font == NULL)
        return;

    u32 secondColor = BLACK_COLOR;
    u32 arg5 = INTRAFONT_WIDTH_VAR;
    
    if (state && state->glow){
        int val = 0;
        float t = (float)((float)(clock() % CLOCKS_PER_SEC)) / ((float)CLOCKS_PER_SEC);
        if(state->glow == 1) {
            val = (t < 0.5f) ? t*311 : (1.0f-t)*311;
        }
        else if(state->glow == 2) {
            val = (t < 0.5f) ? t*411 : (1.0f-t)*411;
        }
        else {
            val = (t < 0.5f) ? t*511 : (1.0f-t)*511;
        }
        secondColor = (0xFF<<24)+(val<<16)+(val<<8)+(val);
    }

    if (state && state->scroll){
        arg5 = INTRAFONT_SCROLL_LEFT;
    }
    
    intraFontSetStyle(font, 0.5f, color, secondColor, 0.f, arg5);

    if (state && state->scroll){
        if (x != state->x || y != state->y){
            state->x = x;
            state->tmp = x;
            state->y = y;
        }
        if (state->w <= 0 || state->w >= 480) state->w = 200;
        state->tmp = intraFontPrintColumn(font, state->tmp, y, state->w, text);
    }
    else
        intraFontPrint(font, x, y, text);
}

void common::clearScreen(){
    ya2d_start_drawing();
    ya2d_clear_screen(CLEAR_COLOR);
}

void common::flip(){
    ya2d_finish_drawing();
    ya2d_swapbuffers();
};

void common::rebootMenu(){

    struct SceKernelLoadExecVSHParam param;
    memset(&param, 0, sizeof(SceKernelLoadExecVSHParam));

    char path[256];
    strcpy(path, ark_config.arkpath);
    strcat(path, ARK_XMENU);

    int runlevel = 0x141;
    
    param.args = strlen(path) + 1;
    param.argp = path;
    param.key = "game";
    sctrlKernelLoadExecVSHWithApitype(runlevel, path, &param);
}