#include "text.h"

TextAnim::TextAnim(string title, string subtitle){
    this->title = title;
    this->subtitle = subtitle;
    memset(&state, 0, sizeof(TextState));
    this->state.scroll = 1;
}

TextAnim::~TextAnim(){
}
        
void TextAnim::draw(float y){
    common::printText(200, y+30, title.c_str(), WHITE_COLOR, NULL);
    common::printText(200, y+60, subtitle.c_str(), WHITE_COLOR, &state);
}
