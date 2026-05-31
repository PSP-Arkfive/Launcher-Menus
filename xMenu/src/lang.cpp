#include <cjson/cJSON.h>
#include <string>

#include "lang.h"
#include "common.h"


static cJSON* cur_lang = NULL;

using namespace std;

bool Translations::loadLanguage(string lang_file){

    // cleanup old language and font
    if (cur_lang){
        cJSON* aux = cur_lang;
        cur_lang = NULL;
        cJSON_Delete(aux);
    }

    // read language file from PKG
    unsigned size = 0;
    void* buf = NULL;
    SceIoStat stat;

    if (sceIoGetstat(lang_file.c_str(), &stat)>=0){
        buf = common::readFile(lang_file.c_str(), &size);
    }
    else {
        buf = common::readFromPKG(lang_file.c_str(), &size, "LANG.ARK");
    }

    if (buf && size){
        // parse new language file
        cJSON* val;
        cur_lang = cJSON_ParseWithLength((const char*)buf, size);

        // check if language file requires a font
        val = cJSON_GetObjectItem(cur_lang, "__font__");
        if (val){
            char* fontfile = cJSON_GetStringValue(val);
            if (fontfile){
                common::loadLanguageFont(fontfile);
            }
        }

        // free resources
        free(buf);
    }

    return (cur_lang!=NULL);
}

string Translations::translate(string orig){

    if (cur_lang != NULL){

        cJSON* val = cJSON_GetObjectItem(cur_lang, orig.c_str());
        
        if (val != NULL){
            char* s = cJSON_GetStringValue(val);
            return string(s);
        }

    }

    return orig;
}
