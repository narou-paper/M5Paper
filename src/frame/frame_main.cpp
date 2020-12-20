#include "frame_main.h"
#include "frame_setting.h"
#include "frame_fileindex.h"
#include "frame_receive.h"
#include "frame_novellist.h"

enum
{
    kKeyNovelList = 0,
    kKeyReceive,
    kKeySetting,
    kKeySDFile,
};

#define KEY_W 92
#define KEY_H 92

void key_setting_cb(epdgui_args_vector_t &args)
{
    Frame_Base *frame = EPDGUI_GetFrame("Frame_Setting");
    if (frame == NULL)
    {
        frame = new Frame_Setting();
        EPDGUI_AddFrame("Frame_Setting", frame);
    }
    EPDGUI_PushFrame(frame);
    *((int *)(args[0])) = 0;
}

void key_sdfile_cb(epdgui_args_vector_t &args)
{
    Frame_Base *frame = new Frame_FileIndex("/");
    EPDGUI_PushFrame(frame);
    *((int *)(args[0])) = 0;
}

void key_receive_cb(epdgui_args_vector_t &args)
{
    Frame_Base *frame = EPDGUI_GetFrame("Frame_Receive");
    if (frame == NULL)
    {
        frame = new Frame_Receive();
        EPDGUI_AddFrame("Frame_Receive", frame);
    }
    EPDGUI_PushFrame(frame);
    *((int *)(args[0])) = 0;
}

void key_novellist_cb(epdgui_args_vector_t &args)
{
    Frame_Base *frame = EPDGUI_GetFrame("Frame_NovelList");
    frame = new Frame_NovelList("");
    EPDGUI_PushFrame(frame);
    *((int *)(args[0])) = 0;
}

Frame_Main::Frame_Main(void) : Frame_Base(false)
{
    _frame_name = "Frame_Main";
    _frame_id = 1;

    _bar = new M5EPD_Canvas(&M5.EPD);
    _bar->createCanvas(540, 44);
    _bar->setTextSize(26);

    _key[kKeyNovelList] = new EPDGUI_Button("読む", 20, 80, 500, 350);
    _key[kKeyNovelList]->CanvasNormal()->drawPngFile(SPIFFS, "/1.png");
    *(_key[kKeyNovelList]->CanvasPressed()) = *(_key[kKeyNovelList]->CanvasNormal());
    _key[kKeyNovelList]->CanvasPressed()->ReverseColor();
    _key[kKeyNovelList]->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void *)(&_is_run));
    _key[kKeyNovelList]->Bind(EPDGUI_Button::EVENT_RELEASED, key_novellist_cb);

    _key[kKeyReceive] = new EPDGUI_Button("入れる", 20, 450, 500, 350);
    _key[kKeyReceive]->CanvasNormal()->drawPngFile(SPIFFS, "/2.png");
    *(_key[kKeyReceive]->CanvasPressed()) = *(_key[kKeyReceive]->CanvasNormal());
    _key[kKeyReceive]->CanvasPressed()->ReverseColor();
    _key[kKeyReceive]->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void *)(&_is_run));
    _key[kKeyReceive]->Bind(EPDGUI_Button::EVENT_RELEASED, key_receive_cb);

    _key[kKeySetting] = new EPDGUI_Button("設定", 20, 820, 240, 120);
    _key[kKeySetting]->CanvasNormal()->drawPngFile(SPIFFS, "/3.png");
    *(_key[kKeySetting]->CanvasPressed()) = *(_key[kKeySetting]->CanvasNormal());
    _key[kKeySetting]->CanvasPressed()->ReverseColor();
    _key[kKeySetting]->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void *)(&_is_run));
    _key[kKeySetting]->Bind(EPDGUI_Button::EVENT_RELEASED, key_setting_cb);

    _key[kKeySDFile] = new EPDGUI_Button("設定", 280, 820, 240, 120);
    _key[kKeySDFile]->CanvasNormal()->drawPngFile(SPIFFS, "/4.png");
    *(_key[kKeySDFile]->CanvasPressed()) = *(_key[kKeySDFile]->CanvasNormal());
    _key[kKeySDFile]->CanvasPressed()->ReverseColor();
    _key[kKeySDFile]->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void *)(&_is_run));
    _key[kKeySDFile]->Bind(EPDGUI_Button::EVENT_RELEASED, key_sdfile_cb);

    _time = 0;
    _next_update_time = 0;
}

Frame_Main::~Frame_Main(void)
{
    for (int i = 0; i < ICON_NUM; i++)
    {
        delete _key[i];
    }
}

void Frame_Main::StatusBar(m5epd_update_mode_t mode)
{
    if ((millis() - _time) < _next_update_time)
    {
        return;
    }
    char buf[20];
    _bar->fillCanvas(0);
    _bar->drawFastHLine(0, 43, 540, 15);
    _bar->setTextDatum(CL_DATUM);
    _bar->drawString("M5Paper", 10, 27);

    // Battery
    _bar->setTextDatum(CR_DATUM);
    _bar->pushImage(498, 8, 32, 32, ImageResource_status_bar_battery_32x32);
    uint32_t vol = M5.getBatteryVoltage();

    if (vol < 3300)
    {
        vol = 3300;
    }
    else if (vol > 4350)
    {
        vol = 4350;
    }
    float battery = (float)(vol - 3300) / (float)(4350 - 3300);
    if (battery <= 0.01)
    {
        battery = 0.01;
    }
    if (battery > 1)
    {
        battery = 1;
    }
    uint8_t px = battery * 25;
    sprintf(buf, "%d%%", (int)(battery * 100));
    _bar->drawString(buf, 498 - 10, 27);
    _bar->fillRect(498 + 3, 8 + 10, px, 13, 15);
    // _bar->pushImage(498, 8, 32, 32, 2, ImageResource_status_bar_battery_charging_32x32);

    // Time
    rtc_time_t time_struct;
    rtc_date_t date_struct;
    M5.RTC.getTime(&time_struct);
    M5.RTC.getDate(&date_struct);
    sprintf(buf, "%2d:%02d", time_struct.hour, time_struct.min);
    _bar->setTextDatum(CC_DATUM);
    _bar->drawString(buf, 270, 27);
    _bar->pushCanvas(0, 0, mode);

    _time = millis();
    _next_update_time = (60 - time_struct.sec) * 1000;
}

int Frame_Main::init(epdgui_args_vector_t &args)
{
    _is_run = 1;
    // M5.EPD.WriteFullGram4bpp(GetWallpaper());
    M5.EPD.Clear();
    for (int i = 0; i < ICON_NUM; i++)
    {
        EPDGUI_AddObject(_key[i]);
    }
    _time = 0;
    _next_update_time = 0;
    StatusBar(UPDATE_MODE_NONE);
    return 9;
}

int Frame_Main::run()
{
    StatusBar(UPDATE_MODE_GL16);
    return 1;
}