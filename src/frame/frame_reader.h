#ifndef _FRAME_READER_H_
#define _FRAME_READER_H_

#include "frame_base.h"
#include "../epdgui/epdgui.h"

class Frame_Reader : public Frame_Base
{
public:
    Frame_Reader(String novel_name, String episode);
    ~Frame_Reader();
    int init(epdgui_args_vector_t &args);
    int run();
    void err(String info);
    void drawPicture();

private:
    M5EPD_Canvas *_canvas_picture;
    EPDGUI_Button *_key_next;
    EPDGUI_Button *_key_prev;
    std::vector<String> _paths;
    int _page;
    int _page_size;
    bool _page_changed;
};

#endif //_FRAME_READER_H_