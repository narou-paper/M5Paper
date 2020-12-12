#include "frame_receive.h"
#include "WiFi.h"
#include "WebServer.h"

WebServer _web_server(80);
M5EPD_Canvas _receive_canvas(&M5.EPD);
xTaskHandle xHandle;

void webOpenHandler(void *pvParameter){
    while(1){
        _web_server.handleClient();
    }
}

void webOpen()
{
    WiFi.softAP("Narou-Paper-M5Paper","mokemoke");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    _web_server.begin();
    xTaskCreatePinnedToCore(webOpenHandler, "webOpen", 4096, NULL, 1, &xHandle, 0);
};

void webClose()
{
    _web_server.stop();
    vTaskDelete(xHandle);
    WiFi.softAPdisconnect(true);
};

void handleRoot(){
    _web_server.send(200, "text/plain", "Hello");
};

void handleNotFound()
{
    _web_server.send(404, "text/plain", "Oh No.");
};

Frame_Receive::Frame_Receive(void)
{
    _frame_name = "Frame_Receive";

    exitbtn("Home");

    _canvas_title->drawString("Receive", 270, 34);

    _key_exit->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void*)(&_is_run));
    _key_exit->Bind(EPDGUI_Button::EVENT_RELEASED, &Frame_Base::exit_cb);

    _web_server.on("/", handleRoot);
    _web_server.onNotFound(handleNotFound);
};

Frame_Receive::~Frame_Receive(void)
{

};

int Frame_Receive::run()
{
    return 1;
};

void Frame_Receive::exit()
{
    webClose();
    _receive_canvas.deleteCanvas();
}

int Frame_Receive::init(epdgui_args_vector_t &args)
{
    _is_run = 1;

    M5.EPD.WriteFullGram4bpp(GetWallpaper());

    _receive_canvas.createCanvas(300,100);
    _receive_canvas.fillCanvas(15);
    _receive_canvas.setTextSize(26);
    _receive_canvas.setTextColor(0);
    _receive_canvas.setTextDatum(CC_DATUM);
    _receive_canvas.drawString("Hello. Send jpg image!", 150, 55);
    _receive_canvas.pushCanvas(120, 430, UPDATE_MODE_DU4);

    _canvas_title->pushCanvas(0, 8, UPDATE_MODE_NONE);

    EPDGUI_AddObject(_key_exit);

    webOpen();

    return 3;
};
