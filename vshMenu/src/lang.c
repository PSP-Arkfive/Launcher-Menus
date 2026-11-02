#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <psputility_sysparam.h>

#include <cfwmacros.h>
#include <systemctrl_ark.h>

#include "lang.h"


#define LINE_BUFFER_SIZE 256

extern ARKConfig ark_config;

static char* language_strings[LANG_MAX];
static const char*  default_strings[LANG_MAX] = {
    "VSH-GU Menu",
    "Shutdown Device",
    "Suspend Device",
    "Reset Device",
    "Reset VSH",
    "Options",
    "Exit",
    "Background Color:",
    "Background Transparency:",
    "Text Color:",
    "Text Font:",
    "Accept",
    "Cancel",
    "None",
    "Little", "Normal", "High",
    "Default",
    "Red", "Orange", "Yellow", "Green",
    "Blue", "Indigo", "Violet", "Pink",
    "Purple", "Teal", "Aqua", "Grey",
    "Black", "White",
};

extern SceOff findPkgOffset(const char* filename, unsigned* size, const char* pkgpath);

const char* getString(int strid){
    if (language_strings[strid]) return language_strings[strid];
    return default_strings[strid];
}

int readLine(char* source, char *str)
{
    u8 ch = 0;
    int n = 0;
    int i = 0;
    while(1)
    {
        if( (ch = source[i]) == 0){
            *str = 0;
            return n;
        }
        n++; i++;
        if(ch < 0x20)
        {
            *str = 0;
            return n;
        }
        else
        {
            *str++ = ch;
        }
    }
}

int loadLanguage(){
    static char *languages[] = { "jp", "en", "fr", "es", "de", "it", "nl", "pt", "ru", "ko", "cht", "chs" };

    int id; sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &id);

    for (int i=0; i<LANG_MAX; i++){
        if (language_strings[i]) free(language_strings[i]);
        language_strings[i] = NULL;
    }

    SceUID fd = -1;
    SceOff offset = 0;
    unsigned size = 0;
    if (id < NELEMS(languages)){
        char file[64];
        char pkgpath[ARK_PATH_SIZE];
   
        sprintf(file, "satelite_%s.txt", languages[id]);

        strcpy(pkgpath, ark_config.arkpath);
        strcat(pkgpath, file);

        fd = sceIoOpen(pkgpath, PSP_O_RDONLY, 0);
        
        if (fd >= 0){
            size = sceIoLseek32(fd, 0, PSP_SEEK_END);
        }
        else {
            strcpy(pkgpath, ark_config.arkpath);
            strcat(pkgpath, "LANG.ARK");
            offset = findPkgOffset(file, &size, pkgpath);
            if (!offset && !size)
                pkgpath[0] = 0;
            
            fd = sceIoOpen(pkgpath, PSP_O_RDONLY, 0);
        }
    }

    if(fd < 0) return 0;

    u8* buf = malloc(size+1);
    sceIoLseek(fd, offset, PSP_SEEK_SET);
    sceIoRead(fd, buf, size);
    sceIoClose(fd);
    buf[size] = 0;
    

    int counter = 0;
    char line[LINE_BUFFER_SIZE];
    int buf_pos = 0;

    // Skip UTF8 magic
    u32 magic = *(u32*)buf;
    if ((magic & 0xFFFFFF) == 0xBFBBEF){
        buf_pos = 3;
    }

    while (counter < LANG_MAX)
    {
        if (buf_pos >= size) break;

        int n_read = readLine((char*)buf+buf_pos, line);
        buf_pos += n_read;

        if (n_read == 0) break;
        language_strings[counter] = malloc(strlen(line)+1);
        strcpy(language_strings[counter], line);
        counter++;

    }

    free(buf);

    return 1;
}
