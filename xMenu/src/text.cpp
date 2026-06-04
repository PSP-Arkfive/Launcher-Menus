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
    state.scroll = 0;
    state.glow = 1;
    common::printText(200, y+30, title.c_str(), WHITE_COLOR, &state);
    state.scroll = 1;
    state.glow = 0;
    common::printText(230, y+60, subtitle.c_str(), WHITE_COLOR, &state);
}
