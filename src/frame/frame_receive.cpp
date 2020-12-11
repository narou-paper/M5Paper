#include "frame_receive.h"

M5EPD_Canvas receive_canvas(&M5.EPD);

Frame_Receive::Frame_Receive(void)
{
    _frame_name = "Frame_Receive";

    exitbtn("Home");

    _canvas_title->drawString("Receive", 270, 34);

    _key_exit->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void*)(&_is_run));
    _key_exit->Bind(EPDGUI_Button::EVENT_RELEASED, &Frame_Base::exit_cb);
};

Frame_Receive::~Frame_Receive(void)
{

};

int Frame_Receive::run()
{
    receive_canvas.pushCanvas(120, 430, UPDATE_MODE_DU4);
    return 1;
};

void Frame_Receive::exit()
{
    receive_canvas.deleteCanvas();
}

int Frame_Receive::init(epdgui_args_vector_t &args)
{
    _is_run = 1;

    M5.EPD.WriteFullGram4bpp(GetWallpaper());

    receive_canvas.createCanvas(300,100);
    receive_canvas.fillCanvas(15);
    receive_canvas.setTextSize(26);
    receive_canvas.setTextColor(0);
    receive_canvas.setTextDatum(CC_DATUM);
    receive_canvas.drawString("Hello. Send jpg image!", 150, 55);

    _canvas_title->pushCanvas(0, 8, UPDATE_MODE_NONE);

    EPDGUI_AddObject(_key_exit);

    return 3;
};
