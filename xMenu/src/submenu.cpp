/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>

#include <systemctrl.h>
#include <systemctrl_se.h>

#include "menu.h"
#include "common.h"
#include "lang.h"


extern string ark_version;
static string save_status;
static int status_frame_count = 0; // a few seconds
                    			   //

SubMenu::SubMenu(Menu* menu) {
    this->w = 260;
    this->h = 130;
    this->index = 0;
    this->menu = menu;
    this->getItems();
}

SubMenu::~SubMenu() {
    for (int i=0; i<MAX_SUBMENU_ENTRIES; i++){
        free(options[i]);
    }
}

void SubMenu::getItems() {
    common::loadConf();

    options[0] = strdup(TR("Display Battery Percent").c_str());
    options[1] = strdup(TR("Sort Entries by Name").c_str());
    options[2] = strdup(TR("Skip Gameboot").c_str());
    options[3] = strdup(TR("Scan Category entries").c_str());
    options[4] = strdup(TR("Swap X and O Buttons").c_str());
    options[5] = strdup(TR("Restart").c_str());
    options[6] = strdup(TR("Exit").c_str());

    opt_values[0] = common::config.battery_percent;
    opt_values[1] = common::config.sort_entries;
    opt_values[2] = common::config.fast_gameboot;
    opt_values[3] = common::config.scan_cat;
    opt_values[4] = common::config.swap_buttons;

    // calculate window width
    int max_w = 50;
    for (int i=0; i<MAX_SUBMENU_ENTRIES; i++){
        int sw = common::calcTextWidth(options[i]);
        if (sw > max_w) max_w = sw;
    }
    this->w = max_w + 36;
    if (this->w > 480) this->w = 480;
}

void SubMenu::updateScreen(){
    common::clearScreen();
    
    // draw main menu first
    menu->draw();

    // now draw our stuff
    int n = sizeof(options)/sizeof(options[0]);
    int x = (480-w)/2 + 25;
    int y = ((272-h)/2);
    u32 color = 0xa0808000;

    // draw ARK version and info
    {
    int dx = (w-common::calcTextWidth(ark_version.c_str()))/2;
    ya2d_draw_rect(x+dx, y-10, common::calcTextWidth(ark_version.c_str()) + 10, 10, 0x8000ff00, 1);
    common::printText(x+dx + 5, y-1, ark_version.c_str());
    }

    // menu window
    ya2d_draw_rect(x, y, w, h, color, 1);

    // menu items
    int cur_x;
    int cur_y = y + 20;
    for (int i=0; i<n; i++){
        cur_x = x + ((w-(common::calcTextWidth(options[i])))/2);
        u32 color = WHITE_COLOR;
        if (i < MAX_SUBMENU_CONFIGS){
            color = opt_values[i]? GREEN_COLOR : YELLOW_COLOR;
        }
        if (i == index){
            static TextState state = {.glow = 1};
            common::printText(cur_x, cur_y+4, options[i], color, &state);
        }
        else
            common::printText(cur_x, cur_y+5, options[i], color);
        cur_y += 15;
    }

    // draw save status
    if (save_status.length() > 1){
        common::printText(RIGHT, TOP+15, save_status.c_str(), GREEN_COLOR, NULL);

        if (status_frame_count) status_frame_count--;
        else save_status = "";
    }

    common::flip();
}

void SubMenu::run() {
    
    save_status = "";

    Controller control;
    control.update();
    while(1) {

        updateScreen();

        control.update();
        if (control.decline() || control.triangle())
            break;
        else if (control.accept() || control.left() || control.right()){
            switch (index){
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                    changeSetting(index); getItems(); 
                    break;
                case 5: 
                    menu->fadeOut(); common::rebootMenu(); break;
                case 6: 
                    menu->fadeOut(); sceKernelExitGame(); break;
            }
        }
        else if (control.up()){
            if (index > 0) index--;
        }
        else if (control.down()){
            if (index < (sizeof(options)/sizeof(options[0])-1)) index++;
        }

    }
    control.update();
}

void SubMenu::changeSetting(int setting){
    switch (setting){
        case 0: common::config.battery_percent = !common::config.battery_percent; break;
        case 1: common::config.sort_entries = !common::config.sort_entries; break;
        case 2: common::config.fast_gameboot = !common::config.fast_gameboot; break;
        case 3: common::config.scan_cat = !common::config.scan_cat; break;
        case 4: common::config.swap_buttons = !common::config.swap_buttons; break;
    }
    common::saveConf();
    save_status = TR("Settings Saved");
    status_frame_count = 100;
}
