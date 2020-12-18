#include "frame_reader.h"
#include "SD.h"

void key_reader_exit_cb(epdgui_args_vector_t &args)
{
    EPDGUI_PopFrame(true);
    *((int *)(args[0])) = 0;
}

void key_reader_next_cb(epdgui_args_vector_t &args)
{
    *((bool *)args[2]) = true;
    Serial.print("*((int *)args[0]): ");
    Serial.println(*((int *)args[0]));
    Serial.print("*((int *)args[1]): ");
    Serial.println(*((int *)args[1]));
    *((int *)args[0]) = std::min(*((int *)args[0]) + 1, 100);
}

void key_reader_prev_cb(epdgui_args_vector_t &args)
{
    Serial.print("*((int *)args[0]): ");
    Serial.println(*((int *)args[0]));
    *((bool *)args[1]) = true;
    *((int *)args[0]) = std::max(*((int *)args[0]) - 1, 0);
}

std::vector<String> getPagesPath(DynamicJsonDocument &doc, String novelName, String episode)
{
    std::vector<String> paths;
    JsonArray e = doc[novelName][episode]["files"];
    for (JsonVariant v : e)
    {
        paths.push_back(v.as<String>());
    }
    return paths;
}

Frame_Reader::Frame_Reader(String novel_name, String episode)
{
    _frame_name = "Frame_Reader";
    Serial.print("novel_name: ");
    Serial.println(novel_name);
    Serial.print("episode: ");
    Serial.println(episode);

    DynamicJsonDocument doc(2048);
    getDJD(doc);

    _paths = getPagesPath(doc, novel_name, episode);

    Serial.print("paths.size(): ");
    Serial.println(_paths.size());
    for (int i = 0; i < _paths.size(); i++)
    {
        Serial.print("path[");
        Serial.print(i);
        Serial.print("]: ");
        Serial.println(_paths[i]);
    }

    _canvas_picture = new M5EPD_Canvas(&M5.EPD);
    _canvas_picture->createCanvas(540, 960 - 72 - 72);
    _canvas_picture->setTextSize(26);
    _canvas_picture->setTextColor(0);
    _canvas_picture->setTextDatum(CC_DATUM);

    int max = 100;
    _key_next = new EPDGUI_Button("NEXT", 8, 990 - 72 - 72 / 2, 540 / 2 - 16, 72);
    _key_next->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, &_page);
    _key_next->AddArgs(EPDGUI_Button::EVENT_RELEASED, 1, &max);
    _key_next->AddArgs(EPDGUI_Button::EVENT_RELEASED, 2, &_page_changed);
    _key_next->Bind(EPDGUI_Button::EVENT_RELEASED, key_reader_next_cb);

    _key_prev = new EPDGUI_Button("PREV", 540 / 2 + 8, 990 - 72 - 72 / 2, 540 / 2 - 16, 72);
    _key_prev->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, &_page);
    _key_prev->AddArgs(EPDGUI_Button::EVENT_RELEASED, 1, &_page_changed);
    _key_prev->Bind(EPDGUI_Button::EVENT_RELEASED, key_reader_prev_cb);

    uint8_t language = GetLanguage();
    if (language == LANGUAGE_JA)
    {
        exitbtn("戻る");
    }
    else if (language == LANGUAGE_ZH)
    {
        exitbtn("返回");
    }
    else
    {
        exitbtn("Back");
    }

    _page = 0;
    _page_changed = true;

    _key_exit->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void *)(&_is_run));
    _key_exit->Bind(EPDGUI_Button::EVENT_RELEASED, &key_reader_exit_cb);
}

Frame_Reader::~Frame_Reader(void)
{
    delete _canvas_picture;
}

void Frame_Reader::err(String info)
{
    _canvas_picture->fillCanvas(0);
    _canvas_picture->fillRect(254 - 150, 500 - 50, 300, 100, 15);
    _canvas_picture->drawString(info, 150, 55);
}

void Frame_Reader::drawPicture()
{
    LoadingAnime_32x32_Start(254, 500);
 
    String title = String(_page) + "/" + String(_paths.size());
    _canvas_title->drawString(title, 270, 34);

    String draw_path = _paths[_page];
    String suffix = draw_path.substring(draw_path.lastIndexOf("."));
    if ((suffix.indexOf("bmp") >= 0) || (suffix.indexOf("BMP") >= 0))
    {
        bool ret = _canvas_picture->drawBmpFile(SD, draw_path.c_str(), 0, 0);
        if (ret == 0)
        {
            err("Error opening " + draw_path.substring(draw_path.lastIndexOf("/")));
        }
    }
    else if ((suffix.indexOf("png") >= 0) || (suffix.indexOf("PNG") >= 0))
    {
        bool ret = _canvas_picture->drawPngFile(SD, draw_path.c_str());
        if (ret == 0)
        {
            err("Error opening " + draw_path.substring(draw_path.lastIndexOf("/")));
        }
    }
    else if ((suffix.indexOf("jpg") >= 0) || (suffix.indexOf("JPG") >= 0))
    {
        bool ret = _canvas_picture->drawJpgFile(SD, draw_path.c_str());
        if (ret == 0)
        {
            err("Error opening " + draw_path.substring(draw_path.lastIndexOf("/")));
        }
    }
    LoadingAnime_32x32_Stop();
    _canvas_title->pushCanvas(0, 8, UPDATE_MODE_NONE);
    _canvas_picture->pushCanvas(0, 72, UPDATE_MODE_GC16);
}

int Frame_Reader::run()
{
    if (_page_changed)
    {
        _page_changed = false;
        drawPicture();
    }
    return 1;
}

int Frame_Reader::init(epdgui_args_vector_t &args)
{
    _is_run = 1;
    M5.EPD.Clear();
    EPDGUI_AddObject(_key_exit);
    EPDGUI_AddObject(_key_next);
    EPDGUI_AddObject(_key_prev);
    return 3;
}
