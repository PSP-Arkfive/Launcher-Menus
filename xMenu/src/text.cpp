#include "text.h"

TextAnim::TextAnim(string title, string subtitle){
    this->title = title;
    this->subtitle = subtitle;
    memset(&state, 0, sizeof(TinyFontState));
    this->state.scroll = 1;
    this->state.ix = 200;
    this->state.sk = 120;
}

TextAnim::~TextAnim(){
}
        
void TextAnim::draw(float y){
    tinyFontPrintTextScreen(msx, 200, y+30, title.c_str(), WHITE_COLOR, NULL);
    tinyFontPrintTextScreen(msx, 200, y+60, subtitle.c_str(), WHITE_COLOR, &state);
}
