#ifndef TEXT_H
#define TEXT_H

#include <string>
#include <time.h>
#include <intraFont.h>

#include "common.h"


using namespace std;

class TextAnim{

    private:
    
        TextState state;
        string title;
        string subtitle;
        
    public:
    
        TextAnim(string title, string subtitle);
        ~TextAnim();
        
        void draw(float y);
        
};

#endif
