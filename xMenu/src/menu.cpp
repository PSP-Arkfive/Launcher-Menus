#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <algorithm>
#include <pspsdk.h>
#include <psppower.h>

#include <systemctrl_ark.h>

#include "menu.h"
#include "lang.h"



string ark_version;
static std::string toggle;
static char* opt_close_menu = "Triangle -> Close Options Menu";
static char* opt_open_menu = "Triangle -> Open Options Menu";

Menu::Menu(){

    this->index = 0;
    this->start = 0;
    this->txt = NULL;

    toggle = TR(opt_open_menu);

    this->readEbootList("ms0:/PSP/GAME/");
    this->readEbootList("ms0:/PSP/APPS/");
    this->readEbootList("ef0:/PSP/GAME/");
    if (common::config.sort_entries){
        std::sort(eboots.begin(), eboots.end(), Entry::cmpEntriesForSort);
    }
    loadIcons();
}

void Menu::readEbootList(string path){

    SceIoDirent dit; memset(&dit, 0, sizeof(dit));
    SceUID dir = sceIoDopen(path.c_str());
    
    if (dir < 0)
        return;
        
    while (sceIoDread(dir, &dit) > 0){

        if (strcmp(dit.d_name, ".") == 0) continue;
        if (strcmp(dit.d_name, "..") == 0) continue;
        if (strcmp(dit.d_name, "SCPS10084") == 0) continue;
        if (strcmp(dit.d_name, "NPUZ01234") == 0) continue;
        if (strcmp(dit.d_name, "ARK_Loader") == 0) continue;
        if (common::fileExists(path+dit.d_name)) continue;
        
        string fullpath = fullPath(path, dit.d_name);
        if (fullpath.size() == 0){
            if (common::config.scan_cat){
                readEbootList(path+dit.d_name+"/");
            }
            continue;
        }
        
        Entry* e = Entry::createIfPops(fullpath);
        if (e) eboots.push_back(e);
    }
    sceIoDclose(dir);
}

string Menu::fullPath(string path, string app){
    // Return the full path of a homebrew given only the homebrew name
    if (common::fileExists(app))
        return app; // it's already a full path

    else if (common::fileExists(path+app+"/VBOOT.PBP"))
        return path+app+"/VBOOT.PBP";

    else if (common::fileExists(path+app+"/EBOOT.PBP"))
        return path+app+"/EBOOT.PBP";

    else if (common::fileExists(path+app+"/FBOOT.PBP"))
        return path+app+"/FBOOT.PBP";

    return "";
}

void Menu::loadIcons(){
    int start = this->start;
    int end = min(start+3, (int)eboots.size());

    if (start-1 >= 0) eboots[start-1]->unloadIcon();
    if (end < eboots.size()) eboots[end]->unloadIcon();

    for (int i=start; i<end; i++){
        eboots[i]->loadIcon();
    }
}

static void drawBattery(){
    if (scePowerIsBatteryExist()) {
        int percent = scePowerGetBatteryLifePercent();
        
        if (percent < 0)
            return;

        u32 color;
        if (scePowerIsBatteryCharging()){
            color = BLUE_COLOR;
        } else if (percent == 100){
            color = GREEN_COLOR;
        } else if (percent >= 17){
            color = LITEGRAY_COLOR;
        } else{
            color = RED_COLOR;
        }

        if (common::config.battery_percent) {
            char batteryPercent[13];
            snprintf(batteryPercent, sizeof(batteryPercent), "%d%%", percent);
            common::printText(420, 13, batteryPercent, color);
        }

        ya2d_draw_rect(455, 6, 20, 8, color, 0);
        ya2d_draw_rect(454, 8, 1, 5, color, 1);
        ya2d_draw_pixel(475, 14, color);
        
        if (percent >= 5){
            int width = percent*17/100;
            ya2d_draw_rect(457+(17-width), 8, width, 5, color, 1);
        }
    }
}

void Menu::draw(){
    ya2d_draw_texture(common::background, 0, 0);
    
    if(eboots.size()>0) {
        // draw all image stuff
        for (int i=this->start; i<min(this->start+3, (int)eboots.size()); i++){
            int offset = 1 + (80 * (i-this->start));
            ya2d_draw_texture(eboots[i]->getIcon(), 25, offset);
            if (i == this->index){
                static u32 alpha = 0;
                static u32 delta = 5;
                u32 color = WHITE_COLOR | (alpha<<24);
                ya2d_draw_rect(200, offset+30+TEXT_HEIGHT, min((int)eboots[i]->getPath().size()*TEXT_WIDTH, 280), 1, color, 1);
                if (alpha == 0) delta = 5;
                else if (alpha == 255) delta = -5;
                alpha += delta;
            }
        }

        // draw scrollbar
        {
            int height = 230/eboots.size();
            int x = 10;
            int y = 10;
            ya2d_draw_rect(x+2, y, 3, height*eboots.size(), DARKGRAY_COLOR, 1);
            ya2d_draw_rect(x+1, y + index*height, 5, height, DARKGRAY_COLOR, 1);
            ya2d_draw_rect(x+3, y, 1, height*eboots.size(), LITEGRAY_COLOR, 1);
            ya2d_draw_rect(x+2, y + index*height, 3, height, LITEGRAY_COLOR, 1);
        }

        // draw all text stuff
        for (int i=this->start; i<min(this->start+3, (int)eboots.size()); i++){
            int offset = 1 + (80 * (i-this->start));
            if (i == this->index)
                this->txt->draw(offset);
            else
                common::printText(200, offset+30, eboots[i]->getName().c_str());
        }
    }
    else {
        common::printText(20, 10, TR("No games available").c_str());
    }

    // draw help text
    static TextState title_state = {.scroll = 1, .w = 190};
    common::printText(210, 10, toggle.c_str(), BLUE_COLOR, &title_state);
    drawBattery();
}

void Menu::updateScreen(){
    // clear framebuffer and draw background image
    common::clearScreen();
    draw();
    common::flip();
}

void Menu::updateTextAnim(){
    if(eboots.size() == 0)
        return;
    if (this->txt != NULL)
        delete this->txt;
    this->txt = new TextAnim(eboots[this->index]->getName(), eboots[this->index]->getPath());
}

void Menu::moveDown(){
    if (this->index == eboots.size())
        return;
    else if (this->index-this->start == 2){
        if (this->index+1 <= eboots.size()-1) {
            this->index++;
            this->start++;
        }
        if (this->start+4 < eboots.size())
            this->start++;
    }
    else if (this->index+1 < eboots.size())
        this->index++;
    updateTextAnim();
    loadIcons();
}

void Menu::moveUp(){
    if (this->index == 0)
        return;
    else if (this->index == this->start){
        this->index--;
        if (this->start>0)
            this->start--;
    }
    else
        this->index--;
    updateTextAnim();
    loadIcons();
}

void Menu::control(){
    Controller control;
    while(1){
        updateScreen();
        control.update();
        if (control.down())
            moveDown();
        else if (control.up())
            moveUp();
        else if (control.accept()){
            if(eboots.size() == 0)
                continue;
            else if (eboots[this->index]->run()){
                loadGame();
            }
        }
        else if (control.start()){
            loadGame();
        }
        else if (control.triangle()){
            openSubMenu();
        }
        else if (control.select()){
            this->fadeOut();
            common::rebootMenu();
            break;
        }
        else if (control.decline()){
            break;
        }
    }
    fadeOut();
}

void Menu::openSubMenu(){
    SubMenu* submenu = new SubMenu(this);
    toggle = TR(opt_close_menu);
    submenu->run();
    toggle = TR(opt_open_menu);
}

void Menu::loadGame(){

    struct SceKernelLoadExecVSHParam param;
    memset(&param, 0, sizeof(SceKernelLoadExecVSHParam));
    
    static char path[256];
    string spath = eboots[index]->getPath();
    strncpy(path, spath.c_str(), sizeof(path));
    
    int runlevel = (path[0]=='e')? POPS_RUNLEVEL_GO : POPS_RUNLEVEL;
    
    param.args = strlen(path) + 1;
    param.argp = (void*)path;
    param.key = "pops";
    fadeOut();
    sctrlKernelLoadExecVSHWithApitype(runlevel, path, &param);
}

void Menu::run(){
    // get ARK version
    u32 major = sctrlSEGetVersion();
    u32 minor = sctrlHENGetVersion();
    u32 micro = sctrlHENGetMinorVersion();

    stringstream version;
    version << "ARK " << major << "." << minor << "." << micro;
    version << " " << common::ark_config.exploit_id;

    ark_version = version.str();

    updateTextAnim();
    control();
}

void Menu::fadeIn(){
    int alpha = 255;
    while (alpha>0){
        u32 color = alpha << 24;
        common::clearScreen();
        ya2d_draw_texture(common::background, 0, 0);
        ya2d_draw_rect(0, 0, 480, 272, color, 1);
        common::flip();
        alpha -= 15;
    }
}

void Menu::fadeOut(){
    int alpha = 0;
    while (alpha<255){
        u32 color = alpha << 24;
        common::clearScreen();
        ya2d_draw_texture(common::background, 0, 0);
        ya2d_draw_rect(0, 0, 480, 272, color, 1);
        common::flip();
        alpha += 15;
    }
}

Menu::~Menu(){
    delete this->txt;
    this->eboots.clear();
}
