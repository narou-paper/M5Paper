#ifndef _FRAME_NOVELLIST_H_
#define _FRAME_NOVELLIST_H_

#include "frame_base.h"
#include "../epdgui/epdgui.h"
#include <SD.h>

class Frame_NovelList : public Frame_Base
{
public:
    Frame_NovelList(String novel);
    ~Frame_NovelList();
    void list();
    int init(epdgui_args_vector_t &arduino_phy_init);
    
private:
    std::vector<EPDGUI_Button*> _key_novels;
    String _novel;
};

#endif //_FRAME_NOVELLIST_H_