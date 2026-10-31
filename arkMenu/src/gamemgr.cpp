/* Game Manager class */

#include <sstream>
#include <fstream>
#include <unistd.h>
#include <algorithm>
#include <stdlib.h>
#include <systemctrl.h>
#include "system_mgr.h"
#include "gamemgr.h"
#include "music_player.h"
#include "osk.h"
#include "lang.h"
#include "texteditor.h"

static GameManager* self = NULL;

static bool loadingData = false;

ARKConfig* ark_config;

GameManager* GameManager::getInstance(){
    if (self == NULL) self = new GameManager();
    return self;
}

void GameManager::pause(){
    while (selectedCategory == -1){
        this->applyWorkerUpdates();
        sceKernelDelayThread(10000);
    }
    for (int i=MAX_CATEGORIES; i>=0; i--){
        SystemMgr::pauseDraw();
        this->maxDraw = i;
        SystemMgr::resumeDraw();
        sceKernelDelayThread(60000);
    }
    pauseIcons();
}

void GameManager::resume(){
    for (int i=0; i<=MAX_CATEGORIES; i++){
        SystemMgr::pauseDraw();
        this->maxDraw = i;
        SystemMgr::resumeDraw();
        sceKernelDelayThread(60000);
    }
    SystemMgr::pauseDraw();
    if (selectedCategory == -2) selectedCategory = -1;
    SystemMgr::resumeDraw();
    resumeIcons();
}

GameManager::GameManager(){
    // set the global self variable as this instance for the threads to use it
    self = this;

    scroll.w = 0;
    this->use_categories = true;
    this->scanning = true;
    this->optionsmenu = NULL;

    // initialize the categories
    this->selectedCategory = (common::config.main_menu)? -2 : -1;
    this->selectionPending = false;
    this->pendingCategory = -2;
    this->umdChangePending = false;
    this->pendingUmdPresent = false;
    for (int i=0; i<MAX_CATEGORIES; i++){
        this->categories[i] = new Menu((EntryType)i);
    }

    // start the multithreaded icon loading
    this->maxDraw = MAX_CATEGORIES;
    this->dynamicIconRunning = ICONS_LOADING;
    this->iconSema = sceKernelCreateSema("icon0_sema",  0, 1, 1, NULL);
    this->iconThread = sceKernelCreateThread("icon0_thread", GameManager::loadIcons, 0x10, 0x2000, PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, NULL);
    sceKernelStartThread(this->iconThread,  0, NULL);
}

GameManager::~GameManager(){
    for (int i=0; i<MAX_CATEGORIES; i++){
        delete this->categories[i];
    }
}

int GameManager::loadIcons(SceSize _args, void *_argp){

    while (true){
        sceKernelWaitSema(self->iconSema, 1, NULL);
        if (self->dynamicIconRunning == ICONS_STOPPED){
            sceKernelSignalSema(self->iconSema, 1);
            break;
        }

        // check UMD status
        sceKernelDelayThread(0);
        std::vector<Entry*>* game_entries = self->categories[GAME]->entries;
        bool has_umd = UMD::isUMD();
        bool umd_loaded = game_entries->size() > 0 && string("UMD") == game_entries->at(0)->getType();
        if (!self->umdChangePending && self->selectedCategory >= 0 &&
            (has_umd != umd_loaded)){
            self->pendingUmdPresent = has_umd;
            self->umdChangePending = true;
        }
        // load icons
        if (self->selectedCategory < 0){
            if (self->selectedCategory == -1 && !self->selectionPending)
                self->findEntries();
            sceKernelSignalSema(self->iconSema, 1);
            sceKernelDelayThread(0);
            continue;
        }
        for (int i=0; i<MAX_CATEGORIES; i++){
            self->categories[i]->loadIconsDynamic(i == self->selectedCategory);
            sceKernelDelayThread(0);
        }
        sceKernelSignalSema(self->iconSema, 1);
    }

    sceKernelExitDeleteThread(0);
    
    return 0;
}

void GameManager::pauseIcons(){
    if (self->dynamicIconRunning == ICONS_PAUSED) return; // already paused
    sceKernelWaitSema(iconSema, 1, NULL);
    self->dynamicIconRunning = ICONS_PAUSED;
}

void GameManager::resumeIcons(){
    if (self->dynamicIconRunning == ICONS_LOADING) return; // already running
    for (int i=0; i<MAX_CATEGORIES; i++)
        categories[i]->resumeIconLoading();
    sceKernelSignalSema(iconSema, 1);
    sceKernelDelayThread(0);
    self->dynamicIconRunning = ICONS_LOADING;
}

bool GameManager::waitIconsLoad(bool forceQuit){
    int icon_status = this->dynamicIconRunning;
    if (icon_status == ICONS_LOADING)
        this->pauseIcons();

    for (int i = 0; i<MAX_CATEGORIES; i++){
        if (!this->categories[i]->waitIconsLoad(i == selectedCategory, forceQuit))
            return false;
    }

    if (icon_status == ICONS_LOADING)
        this->resumeIcons();
    return true;
}

Menu* GameManager::getMenu(EntryType t){
    return this->categories[(int)t];
}

void GameManager::findEntries(){

    int ms_is_ef = sctrlKernelMsIsEf();

    // clear entries
    this->categories[0]->clearEntries();
    this->categories[1]->clearEntries();
    this->categories[2]->clearEntries();
    
    // scan eboots
    this->findEboots("ms0:/PSP/VHBL/");
    this->findEboots("ms0:/PSP/APPS/");
    this->findEboots("ms0:/PSP/GAME/");
    this->findEboots("ms0:/PSP/GAME150/");
    if (!ms_is_ef) {
        this->findEboots("ef0:/PSP/VHBL/");
        this->findEboots("ef0:/PSP/APPS/");
        this->findEboots("ef0:/PSP/GAME/");
    }
    // scan ISOs
    this->findISOs("ms0:/ISO/");
    if (!ms_is_ef) this->findISOs("ef0:/ISO/");

    if (common::config.sort_entries){
        std::sort(this->categories[0]->entries->begin(), this->categories[0]->entries->end(), Entry::cmpEntriesForSort);
        std::sort(this->categories[1]->entries->begin(), this->categories[1]->entries->end(), Entry::cmpEntriesForSort);
        std::sort(this->categories[2]->entries->begin(), this->categories[2]->entries->end(), Entry::cmpEntriesForSort);
    }

    // find the first category with entries
    this->pendingCategory = -2;
    for (int i=0; i<MAX_CATEGORIES && pendingCategory < 0; i++){
        if (!this->categories[i]->empty())
            this->pendingCategory = i;
    }

    this->selectionPending = true;
}

void GameManager::applyWorkerUpdates(){
    if (!this->selectionPending && !this->umdChangePending)
        return;

    sceKernelWaitSema(this->iconSema, 1, NULL);
    bool umdChanged = false;
    if (this->selectionPending || this->umdChangePending){
        SystemMgr::pauseDraw();
        if (this->selectionPending){
            this->selectedCategory = this->pendingCategory;
            this->selectionPending = false;
            this->scanning = false;
        }
        if (this->umdChangePending){
            std::vector<Entry*>* game_entries = this->categories[GAME]->entries;
            if (this->pendingUmdPresent){
                if (game_entries->empty() ||
                    string("UMD") != game_entries->at(0)->getType()){
                    game_entries->insert(game_entries->begin(), new UMD());
                    umdChanged = true;
                }
            }
            else if (!game_entries->empty() &&
                     string("UMD") == game_entries->at(0)->getType()){
                UMD* umd = (UMD*)game_entries->at(0);
                game_entries->erase(game_entries->begin());
                delete umd;
                umdChanged = true;
                if (game_entries->empty() && this->selectedCategory == GAME)
                    this->selectedCategory = HOMEBREW;
            }
            this->umdChangePending = false;
        }
        SystemMgr::resumeDraw();
    }
    sceKernelSignalSema(this->iconSema, 1);
    if (umdChanged)
        common::sound_mp3->play();
}

void GameManager::findEboots(const char* path){ 

    SceIoDirent dit; memset(&dit, 0, sizeof(dit));
    SceUID dir = sceIoDopen(path);
    
    if (dir < 0)
        return;
        
    while (sceIoDread(dir, &dit) > 0){
        if (strstr(dit.d_name, "%") != NULL) continue; // ignore 1.50 kxploit format
        if (strcmp(dit.d_name, ".") == 0) continue; // ignore "cur dir"
        if (strcmp(dit.d_name, "..") == 0) continue; // ignore "parent dir"
        if (!FIO_SO_ISDIR(dit.d_stat.st_attr)) continue; // ignore files
        if (dit.d_name[0] == '.' && !common::config.show_hidden) continue; // ignore hidden?
        if (strcasecmp(dit.d_name, "NPUZ01234") == 0
            || strcasecmp(dit.d_name, "SCPS10084") == 0
            || strcasecmp(dit.d_name, "ARK_Loader") == 0)
            continue; // ignore ARK launchers
        
        string fullpath = Eboot::fullEbootPath(path, dit.d_name, common::config.show_dlc);
        if (fullpath.size() == 0){
            if (common::config.scan_cat){
                findEboots((string(path) + dit.d_name + "/").c_str());
            }
            continue;
        }

        Eboot* e = new Eboot(fullpath);
        switch (Eboot::getEbootType(fullpath.c_str())){
        case TYPE_PSN:         this->categories[GAME]->addEntry(e);        break;
        case TYPE_POPS:        this->categories[POPS]->addEntry(e);        break;
        default:               this->categories[HOMEBREW]->addEntry(e);    break;
        }
    }
    sceIoDclose(dir);
}

void GameManager::findISOs(const char* path){

    SceIoDirent dit; memset(&dit, 0, sizeof(dit));
    int dir = sceIoDopen(path);

    if (dir < 0)
        return;
        
    while (sceIoDread(dir, &dit) > 0){

        if (strcmp(dit.d_name, ".") == 0) continue;
        if (strcmp(dit.d_name, "..") == 0) continue;
        if (dit.d_name[0] == '.' && !common::config.show_hidden) continue;

        string fullpath = string(path)+string(dit.d_name);

        if (FIO_SO_ISDIR(dit.d_stat.st_attr)){
            if (common::config.scan_cat && string(dit.d_name) != string("VIDEO")){
                findISOs((string(path) + dit.d_name + "/").c_str());
            }
            continue;
        }
        if (Iso::isISO(fullpath.c_str())) this->categories[GAME]->addEntry(new Iso(fullpath));
    }
    sceIoDclose(dir);
}

Entry* GameManager::getEntry(){
    if (selectedCategory < 0)
        return NULL;
    return this->categories[this->selectedCategory]->getEntry();
}

int GameManager::getPreviousCategory(int current){
    do { 
        if (current > 0){
            current--;
        }
        else{
            current = MAX_CATEGORIES-1;
        }
    } while (this->categories[current]->empty());
    return current;
}

int GameManager::getNextCategory(int current){
    do {
        if (current < MAX_CATEGORIES-1){
            current++;
        }
        else{
            current = 0;
        }
    } while (this->categories[current]->empty());
    return current;
}

void GameManager::moveLeft(){
    if (selectedCategory < 0)
        return;

    SystemMgr::pauseDraw();
    int auxCategory = getPreviousCategory(selectedCategory);
    if (auxCategory != selectedCategory){
        this->categories[selectedCategory]->animStart(-2);
        this->categories[auxCategory]->animStart(-2);
        selectedCategory = auxCategory;
        SystemMgr::resumeDraw();
        common::sound_mp3->play();
    }
    else SystemMgr::resumeDraw();
    sceKernelDelayThread(100000);
}

void GameManager::moveRight(){
    if (selectedCategory < 0)
        return;

    SystemMgr::pauseDraw();
    int auxCategory = getNextCategory(selectedCategory);
    
    if (auxCategory != selectedCategory){
        this->categories[selectedCategory]->animStart(2);
        this->categories[auxCategory]->animStart(2);
        selectedCategory = auxCategory;
        SystemMgr::resumeDraw();
        common::sound_mp3->play();
    }
    else SystemMgr::resumeDraw();
    sceKernelDelayThread(100000);
}

void GameManager::moveUp(){
    if (selectedCategory < 0)
        return;
    scroll.tmp = scroll.x; // reset text scroll
    SystemMgr::pauseDraw();
    this->categories[this->selectedCategory]->moveUp();
    SystemMgr::resumeDraw();
}

void GameManager::moveDown(){
    if (selectedCategory < 0)
        return;
    scroll.tmp = scroll.x; // reset text scroll
    SystemMgr::pauseDraw();
    this->categories[this->selectedCategory]->moveDown();
    SystemMgr::resumeDraw();
}

void GameManager::stopFastScroll(){
    if (selectedCategory < 0)
        return;
    SystemMgr::pauseDraw();
    this->categories[this->selectedCategory]->stopFastScroll();
    SystemMgr::resumeDraw();
}

string GameManager::getFooter(){
    return "";
}

string GameManager::getInfo(){
    if (selectedCategory >= 0){
        Entry* e = getEntry();
        if (common::config.show_path){
            return e->getName() + " <" + e->path + ">";
        }
        return e->getName();
    }
    else if (selectedCategory == -1) return "Loading games...";
    else if (selectedCategory == -2) return "No games available";
    return "Unknown Menu State";
}

void GameManager::draw(){
    if (this->selectedCategory >= 0){
        for (int i=0; i<this->maxDraw; i++){
            if (i == (int)this->selectedCategory)
                continue;
            this->categories[i]->draw(false);
            sceKernelDelayThread(0);
        }
        if (this->selectedCategory < this->maxDraw)
            this->categories[this->selectedCategory]->draw(true);
        if (optionsmenu) optionsmenu->draw();
    }
    if (loadingData || selectedCategory == -1){
        static float angle = 1.0;
        Image* img = common::getImage(IMAGE_WAITICON);
        img->draw_rotate(
            (480-img->texture->width)/2,
            (272-img->texture->height)/2,
            angle
        );
        angle+=0.2;
    }
}

void GameManager::endAllThreads(){
    if (dynamicIconRunning == ICONS_PAUSED){
        dynamicIconRunning = ICONS_STOPPED;
        sceKernelSignalSema(iconSema, 1);
    }
    else{
        sceKernelWaitSema(iconSema, 1, NULL);
        dynamicIconRunning = ICONS_STOPPED;
        sceKernelSignalSema(iconSema, 1);
    }
    sceKernelWaitThreadEnd(iconThread, 0);
}


void GameManager::control(Controller* pad){
    this->applyWorkerUpdates();

    if (pad->down()){
        this->moveDown();
    }
    else if (pad->up()){
        this->moveUp();
    }
    else {
        this->stopFastScroll();
    }
    
    if (pad->left())
        this->moveLeft();
    else if (pad->right())
        this->moveRight();
    else if (pad->accept()){
        if (selectedCategory >= 0 && !categories[selectedCategory]->isAnimating()){
            common::sound_mp3->play();
            this->execApp();
        }
    }
    else if (pad->start()){
        if (selectedCategory >= 0 && !categories[selectedCategory]->isAnimating()){
            this->startBoot();
        }
    }
    else if (pad->select()){
        if (selectedCategory != -1){
            common::sound_mp3->play();
            this->waitIconsLoad();
            GameManager::updateGameList(NULL);
            this->waitIconsLoad();
        }
    }
    else if (pad->LT()){
        if (selectedCategory >= 0 && !categories[selectedCategory]->isAnimating()){
            common::sound_mp3->play();
            this->gameOptionsMenu();
        }
    }
}

void GameManager::updateGameList(const char* path){
    if (path == NULL 
        || strncmp(path, "ms0:/PSP/GAME/", 14) == 0 || strncmp(path, "ms0:/ISO/", 9) == 0
        || strncmp(path, "ef0:/PSP/GAME/", 14) == 0 || strncmp(path, "ef0:/ISO/", 9) == 0
        || strncmp(path, "ms0:/PSP/VHBL/", 14) == 0 || strncmp(path, "ms0:/PSP/APPS/", 14) == 0
        || strncmp(path, "ef0:/PSP/VHBL/", 14) == 0 || strncmp(path, "ef0:/PSP/APPS/", 14) == 0
        || strncmp(path, "ms0:/PSP/GAME150/", 17) == 0
      ){
        int icon_status = self->dynamicIconRunning;
        if (icon_status == ICONS_LOADING){
            self->pauseIcons();
        }
        SystemMgr::pauseDraw();
        self->selectedCategory = -1;
        self->scanning = true;
        self->selectionPending = false;
        self->umdChangePending = false;
        for (int i=0; i<MAX_CATEGORIES; i++){
            self->categories[i]->clearEntries();
        }
        SystemMgr::resumeDraw();
        if (icon_status == ICONS_LOADING){
            self->resumeIcons();
        }
    }
}

void GameManager::execApp(){
    if (common::config.fast_gameboot){
        this->endAllThreads();
        this->getEntry()->execute();
    }

    SystemMgr::pauseDraw();
    loadingData = true;
    SystemMgr::resumeDraw();
    this->waitIconsLoad();
    this->getEntry()->loadPics();
    SystemMgr::pauseDraw();
    loadingData = false;
    SystemMgr::resumeDraw();
    if (this->getEntry()->pmfPrompt()){
        this->endAllThreads();
        this->getEntry()->execute();
    }
    this->getEntry()->freeTempData();
    sceKernelDelayThread(0);
}

void GameManager::gameOptionsMenu(){
    t_options_entry options_entries[] = {
        {OPTIONS_CANCELLED, "Cancel"},
        {0, "View Info"},
        {1, "Rename"},
        {2, "Delete"},
    };
    optionsmenu = new OptionsMenu("Game Options", sizeof(options_entries)/sizeof(t_options_entry), options_entries);
    int ret = optionsmenu->control();
    OptionsMenu* aux = optionsmenu;
    optionsmenu = NULL;
    delete aux;

    switch (ret){

    case 0:{
        // create a new options menu but each entry is some info about the game
        Entry* e = this->getEntry();
        string path = e->path;
        SfoInfo info = e->getSfoInfo();
        string game_info[] = {
            info.title,
            info.gameid,
            path,
            common::beautifySize(common::fileSize(path))
        };
        string fullname = TR("Name") + " - " + game_info[0];
        string gameid = TR("Game ID") + " - " + game_info[1];
        string fullpath = TR("Path") + " - " + game_info[2];
        string size = TR("Size") + " - " + game_info[3];
        t_options_entry gameinfo_entries[] = {
            {-1, "Cancel"},
            {0, (char*)fullname.c_str()},
            {1, (char*)gameid.c_str()},
            {3, (char*)size.c_str()},
            {2, (char*)fullpath.c_str()},
        };
        optionsmenu = new OptionsMenu("Game Info", sizeof(gameinfo_entries)/sizeof(t_options_entry), gameinfo_entries);
        int res = optionsmenu->control();
        OptionsMenu* aux = optionsmenu;
        optionsmenu = NULL;
        delete aux;
        if (res >= 0){
            TextEditor::clipboard = game_info[res];
        }
    } break;

    case 1:{
        // rename the ISO or Eboot folder name
        int icon_status = this->dynamicIconRunning;
        if (icon_status == ICONS_LOADING)
            this->pauseIcons();
        SystemMgr::pauseDraw();
        Entry* e = this->getEntry();
        string name = e->getName();
        OSK osk;

        char parent[128];
        strcpy(parent, e->path.c_str());
        char* pname = strstr(parent, name.c_str());
        *pname = 0;
        
        osk.init("New name for Game", name.c_str(), 50);
        osk.loop();
        if(osk.getResult() != OSK_CANCEL)
        {
            char tmpText[51];
            osk.getText((char*)tmpText);
            string newpath = (string(parent)+string(tmpText));
            string oldpath = (string(parent)+name);
            
            sceIoRename(oldpath.c_str(), newpath.c_str());
            e->name = tmpText;
            e->path = newpath;
        }
        osk.end();
        SystemMgr::resumeDraw();
        if (icon_status == ICONS_LOADING)
            this->resumeIcons();
    } break;

    case 2:{
        // remove current entry from list (adjusting index and selectedCategory accordingly), delete file or folder depending on ISO/EBOOT
        // make checks to prevent deleting stuff like "UMD Drive" and "Recovery" entries
        t_options_entry opts[] = {
            {OPTIONS_CANCELLED, "Cancel"},
            {0, "Confirm"},
        };
        optionsmenu = new OptionsMenu("Confirm Deletion?", sizeof(opts)/sizeof(t_options_entry), opts);
        int pret = optionsmenu->control();
        OptionsMenu* aux = optionsmenu;
        optionsmenu = NULL;
        delete aux;

        if (pret == OPTIONS_CANCELLED) return;

        Entry* e = this->getEntry();
        string name = e->getName();
        if (name != "UMD Drive" && name != "Recovery Menu"){
            int icon_status = this->dynamicIconRunning;
            if (icon_status == ICONS_LOADING)
                this->pauseIcons();
            // pause drawing so we don't crash due to race condition
            SystemMgr::pauseDraw();
            // get current menu and it's game list
            Menu* category = categories[selectedCategory];
            std::vector<Entry*>* entries = category->entries;
            // remove entry from list
            entries->erase(entries->begin()+category->index);
            // adjust selectedCategory
            int retries = 0;
            while (categories[selectedCategory]->entries->size() == 0){
                if (selectedCategory < POPS) selectedCategory++;
                else selectedCategory = GAME;
                if (++retries >= 4){ // can't find a valid menu
                    selectedCategory = -2; // no games available
                    break;
                }
            }
            // resume drawing
            SystemMgr::resumeDraw();

            // delete file/folder
            if (e->getType() == string("ISO")){
                sceIoRemove(e->path.c_str());
            }
            else if (e->getType() == string("EBOOT")){
                string path = e->path;
                if (strstr(path.c_str(), "/PSP/SAVEDATA/") != NULL){
                    // remove eboot only
                    sceIoRemove(e->path.c_str());
                }
                else{
                    string path = e->path;
                    // remove entire folder
                    string folder = path.substr(0, path.rfind("/")+1);
                    Browser::recursiveFolderDelete(folder);
                    if(strstr(path.c_str(), "%/") != NULL) {
                        // remove 1.50 kxploit folder
                        path.erase(path.find("%/"));
                        path += '/';
                        folder = path;
                        Browser::recursiveFolderDelete(folder);
                    }
                }
            }
            // free resources
            delete e;
            if (icon_status == ICONS_LOADING)
                this->resumeIcons();
        }
    }break;

    default: break;
    }
}

void GameManager::startBoot(){
    switch (common::config.startbtn){
    case 0: { // Default Start Button (Current)
        this->endAllThreads();
        this->getEntry()->execute();
    } break;
    case 1: { // Last game
        const char* last_game = common::config.last_game;
        if (last_game[0] == 0) break;
        if (Eboot::isEboot(last_game)){
            this->endAllThreads();
            Eboot* eboot = new Eboot(last_game);
            eboot->execute();
        }
        else if (Iso::isISO(last_game)){
            this->endAllThreads();
            Iso* iso = new Iso(last_game);
            iso->execute();
        }
    } break;
    case 2: { // Random ISO
        if (this->categories[GAME]->entries->size() > 0){
            this->endAllThreads();
            int rand_idx = rand() % this->categories[GAME]->entries->size();
            Entry* e = this->categories[GAME]->getEntry(rand_idx);
            e->execute();
        }
    } break;
    default: break;
    }
}
