#ifndef _FRAME_RECEIVE_H_
#define _FRAME_RECEIVE_H_

#include "frame_base.h"
#include "../epdgui/epdgui.h"

class Frame_Receive : public Frame_Base
{
public:
    Frame_Receive();
    ~Frame_Receive();
    int run();
    void exit();
    int init(epdgui_args_vector_t &args);
};

#endif //_FRAME_RECEIVE_H_
