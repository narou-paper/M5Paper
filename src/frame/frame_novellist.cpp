#include "frame_novellist.h"
#include "frame_reader.h"
#include <ArduinoJson.h>

#define MAX_BTN_NUM 13

void key_novellist_novel_cb(epdgui_args_vector_t &args)
{
    Frame_Base *frame = new Frame_NovelList(((EPDGUI_Button *)(args[0]))->GetCustomString());
    EPDGUI_PushFrame(frame);
    *((int *)(args[1])) = 0;
    log_d("%s", ((EPDGUI_Button *)(args[0]))->GetCustomString().c_str());
}

void key_novellist_episode_cb(epdgui_args_vector_t &args)
{
    Frame_Base *frame = new Frame_Reader(*((String *)args[2]), ((EPDGUI_Button *)(args[0]))->GetCustomString());
    EPDGUI_PushFrame(frame);
    *((int *)(args[1])) = 0;
    log_d("%s", ((EPDGUI_Button *)(args[0]))->GetCustomString().c_str());
}

void key_novellist_exit_cb(epdgui_args_vector_t &args)
{
    EPDGUI_PopFrame(true);
    *((int *)(args[0])) = 0;
}

Frame_NovelList::Frame_NovelList(String novel)
{
    _frame_name = "Frame_NovelList";

    _novel = novel;

    _canvas_title->setTextDatum(CC_DATUM);
    if (novel == "")
    {
        exitbtn("Home");
        _canvas_title->drawString("Novel List", 275, 34);
    }
    else
    {
        exitbtn("../");
        String title = novel;
        if (novel.length() > 20)
        {
            title = novel.substring(0, 20) + "...";
        }
        _canvas_title->drawString(title, 275, 34);
    }
    _key_exit->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void *)(&_is_run));
    _key_exit->Bind(EPDGUI_Button::EVENT_RELEASED, &key_novellist_exit_cb);
};

std::vector<String> getNovels(DynamicJsonDocument &doc)
{
    std::vector<String> novels;
    for (JsonPair p : doc.as<JsonObject>())
    {
        Serial.println(p.key().c_str());
        novels.push_back(p.key().c_str());
    }
    return novels;
}

std::vector<String> getEpisodes(DynamicJsonDocument &doc, String novelName)
{
    std::vector<String> episodes;
    JsonObject novel = doc[novelName];
    for (JsonPair p : novel)
    {
        episodes.push_back(p.key().c_str());
    }
    return episodes;
}

Frame_NovelList::~Frame_NovelList()
{
    for (int i = 0; i < _key_novels.size(); i++)
    {
        delete _key_novels[i];
    }
};

void Frame_NovelList::list()
{
    DynamicJsonDocument doc(5*1024);
    getDJD(doc);

    std::vector<String> list = _novel == "" ? getNovels(doc)
                                            : getEpisodes(doc, _novel);

    for (int i = 0; i < list.size(); i++)
    {
        if (_key_novels.size() > MAX_BTN_NUM)
            break;
        EPDGUI_Button *btn = new EPDGUI_Button(4, 100 + _key_novels.size() * 60, 532, 61);
        _key_novels.push_back(btn);

        String novelName(list[i]);
        if (novelName.length() > 19)
        {
            novelName = novelName.substring(0, 19) + "...";
        }
        btn->CanvasNormal()->fillCanvas(0);
        btn->CanvasNormal()->drawRect(0, 0, 532, 61, 15);
        btn->CanvasNormal()->setTextSize(26);
        btn->CanvasNormal()->setTextDatum(CL_DATUM);
        btn->CanvasNormal()->setTextColor(15);
        btn->CanvasNormal()->drawString(novelName, 47 + 13, 35);
        btn->SetCustomString(list[i]);
        btn->CanvasNormal()->setTextDatum(CR_DATUM);
        btn->CanvasNormal()->pushImage(15, 14, 32, 32, ImageResource_item_icon_file_floder_32x32);
        btn->CanvasNormal()->pushImage(532 - 15 - 32, 14, 32, 32, ImageResource_item_icon_arrow_r_32x32);
        *(btn->CanvasPressed()) = *(btn->CanvasNormal());
        btn->CanvasPressed()->ReverseColor();

        btn->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, btn);
        btn->AddArgs(EPDGUI_Button::EVENT_RELEASED, 1, (void *)(&_is_run));
        if (_novel == "")
        {
            btn->Bind(EPDGUI_Button::EVENT_RELEASED, key_novellist_novel_cb);
        }
        else
        {
            btn->AddArgs(EPDGUI_Button::EVENT_RELEASED, 2, &_novel);
            btn->Bind(EPDGUI_Button::EVENT_RELEASED, key_novellist_episode_cb);
        }
    }
};

int Frame_NovelList::init(epdgui_args_vector_t &arduino_phy_init)
{
    _is_run = 1;

    for(int i=0;i<_key_novels.size();i++){
        delete _key_novels[i];
        _key_novels.clear();
    }
    list();

    M5.EPD.WriteFullGram4bpp(GetWallpaper());
    _canvas_title->pushCanvas(0, 8, UPDATE_MODE_NONE);
    EPDGUI_AddObject(_key_exit);

    for (int i = 0; i < _key_novels.size(); i++)
    {
        EPDGUI_AddObject(_key_novels[i]);
    }

    return 3;
};