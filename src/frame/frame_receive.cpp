#include "frame_receive.h"
#include "WiFi.h"
#include "WebServer.h"
#include "ESPmDNS.h"
#include "ArduinoJson.h"

#define FORMAT_FILESYSTEM true

const char *ssid = "Narou-Paper";
const char *password = "mokemoke";
const char *host = "narou";
const char *config_file_path = "/config.json";

WebServer _web_server(80);
File fsUploadFile;
std::vector<String> filepaths;
xTaskHandle xHandle;

M5EPD_Canvas _receive_canvas(&M5.EPD);

String formatBytes(size_t bytes)
{
  if (bytes < 1024)
  {
    return String(bytes) + "B";
  }
  else if (bytes < (1024 * 1024))
  {
    return String(bytes / 1024.0) + "KB";
  }
  else if (bytes < (1024 * 1024 * 1024))
  {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
  else
  {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

String getContentType(String filename)
{
  if (_web_server.hasArg("download"))
  {
    return "application/octet-stream";
  }
  else if (filename.endsWith(".htm"))
  {
    return "text/html";
  }
  else if (filename.endsWith(".html"))
  {
    return "text/html";
  }
  else if (filename.endsWith(".css"))
  {
    return "text/css";
  }
  else if (filename.endsWith(".js"))
  {
    return "text/javascript";
  }
  else if (filename.endsWith(".png"))
  {
    return "image/png";
  }
  else if (filename.endsWith(".gif"))
  {
    return "image/gif";
  }
  else if (filename.endsWith(".jpg"))
  {
    return "image/jpeg";
  }
  else if (filename.endsWith(".ico"))
  {
    return "image/x-icon";
  }
  else if (filename.endsWith(".xml"))
  {
    return "text/xml";
  }
  else if (filename.endsWith(".pdf"))
  {
    return "application/x-pdf";
  }
  else if (filename.endsWith(".zip"))
  {
    return "application/x-zip";
  }
  else if (filename.endsWith(".gz"))
  {
    return "application/x-gzip";
  }
  return "text/plain";
}

bool exists(String path)
{
  bool yes = false;
  File file = SPIFFS.open(path, "r");
  if (!file.isDirectory())
  {
    yes = true;
  }
  file.close();
  return yes;
}

void getDJD(DynamicJsonDocument &doc)
{
  File file;
  file = SD.open(config_file_path, "r");
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));
  file.close();
}

void writeDJD(DynamicJsonDocument &doc)
{
  File file = SD.open(config_file_path, "w");
  serializeJsonPretty(doc, Serial); // ただの表示
  size_t error = serializeJson(doc, file);
  if (error == 0)
    Serial.println("error1");
    SPIFFS.begin();
  file.close();
}

void editConfigRemoveEpisode(String title, String episode)
{
  DynamicJsonDocument doc(2048);
  getDJD(doc);

  JsonObject novel = doc[title];
  novel.remove(episode);

  writeDJD(doc);
}

void editConfigWriteSubtitle(String title, String episode, String subtitle)
{
  DynamicJsonDocument doc(2048);
  getDJD(doc);

  doc[title][episode]["subtitle"] = subtitle;

  writeDJD(doc);
}

void editConfigAddFile(String title, String episode, String filename)
{
  DynamicJsonDocument doc = DynamicJsonDocument(2048);
  getDJD(doc);

  JsonArray files = doc[title][episode]["files"];
  if (files.isNull())
    files = doc[title][episode].createNestedArray("files");
  files.add("/" + filename);

  writeDJD(doc);
}

void editConfigWriteFiles(String title, String episode)
{
  DynamicJsonDocument doc(2048);
  getDJD(doc);

  JsonArray files = doc[title][episode].createNestedArray("files");

  for (String path : filepaths)
  {
    files.add(path);
  }

  writeDJD(doc);
}

void webOpenHandler(void *pvParameter)
{
    while (1)
    {
        _web_server.handleClient();
    }
}

void webOpen()
{
    WiFi.softAP(ssid, password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    MDNS.begin(host);
    Serial.print("Open http://");
    Serial.print(host);
    Serial.println(".local/edit to see the file browser");

    _web_server.begin();
    xTaskCreatePinnedToCore(webOpenHandler, "webOpen", 4096, NULL, 1, &xHandle, 0);
};

void webClose()
{
    _web_server.close();
    vTaskDelete(xHandle);
    WiFi.softAPdisconnect(true);
};

bool handleFileRead(String path)
{
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/"))
  {
    path += "index.html";
  }
  String contentType = getContentType(path);
  if (exists(path))
  {
    File file = SPIFFS.open(path, "r");
    _web_server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload()
{
  String novelTitle = _web_server.arg("title");
  String episode = _web_server.arg("episode");
  String subtitle = _web_server.arg("subtitle");

  HTTPUpload &upload = _web_server.upload();
  // // ---------------------------------------------
  // DBG_OUTPUT_PORT.print("upload.filename: ");
  // DBG_OUTPUT_PORT.println(upload.filename);
  // DBG_OUTPUT_PORT.print("upload.name: ");
  // DBG_OUTPUT_PORT.println(upload.name);
  // DBG_OUTPUT_PORT.print("upload.type: ");
  // DBG_OUTPUT_PORT.println(upload.type);
  // DBG_OUTPUT_PORT.print("upload.totalSize: ");
  // DBG_OUTPUT_PORT.println(upload.totalSize);
  // DBG_OUTPUT_PORT.print("upload.currentSize: ");
  // DBG_OUTPUT_PORT.println(upload.currentSize);
  // for (int i = 0; i < upload.currentSize; i++)
  // {
  //   DBG_OUTPUT_PORT.printf("%c ", upload.buf[i]);
  // }
  // // ---------------------------------------------

  if (upload.status == UPLOAD_FILE_START)
  {
    if (novelTitle == "" || episode == "" || subtitle == "")
      _web_server.send(400, "Bad Request");

    String filename = upload.filename;
    if (!filename.startsWith("/"))
      filename = "/" + filename;

    filepaths.push_back(filename);

    Serial.print("File Name: ");
    Serial.println(filename);

    fsUploadFile = SD.open(filename, "w");
    filename = String();
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if (fsUploadFile)
    {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (fsUploadFile)
    {
      fsUploadFile.close();
    }

    Serial.print("handleFileUpload Size: ");
    Serial.println(upload.totalSize);
  }
}

Frame_Receive::Frame_Receive(void)
{
    _frame_name = "Frame_Receive";

    exitbtn("Home");

    _canvas_title->drawString("Receive", 270, 34);

    _key_exit->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void *)(&_is_run));
    _key_exit->Bind(EPDGUI_Button::EVENT_RELEASED, &Frame_Base::exit_cb);

    _web_server.serveStatic("/index.html", SPIFFS, "/index.html");
    _web_server.serveStatic("/favicon.ico", SPIFFS, "/favicon.ico");
    _web_server.serveStatic("/app.js", SPIFFS, "/app.js");
    _web_server.serveStatic("/chunk-vendors.css", SPIFFS, "/chunk-vendors.css");
    _web_server.serveStatic("/chunk-vendors.js", SPIFFS, "/chunk-vendors.js");
    _web_server.serveStatic("/config.json", SD, "/config.json"); //デバッグ用に

    _web_server.on(
        "/", HTTP_POST, []() {
        String novelTitle = _web_server.arg("title");
        String episode = _web_server.arg("episode");
        String subtitle = _web_server.arg("subtitle");

        // DBG_OUTPUT_PORT.println(novelTitle);
        // DBG_OUTPUT_PORT.println(episode);
        // DBG_OUTPUT_PORT.println(subtitle);

        editConfigWriteSubtitle(novelTitle, episode, subtitle);
        editConfigWriteFiles(novelTitle, episode);
        filepaths.clear();

        _web_server.send(200, "text/plain", "Thank You!");
        },
        handleFileUpload);

    _web_server.onNotFound([]() {
    if (!handleFileRead(_web_server.uri()))
    {
        _web_server.send(404, "text/plain", "FileNotFound");
    }
    });
};

Frame_Receive::~Frame_Receive(void){

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

    _receive_canvas.createCanvas(300, 100);
    _receive_canvas.fillCanvas(15);
    _receive_canvas.setTextSize(26);
    _receive_canvas.setTextColor(0);
    _receive_canvas.setTextDatum(CC_DATUM);
    _receive_canvas.drawString("Hello. Send jpg image!", 150, 55);
    _receive_canvas.pushCanvas(120, 430, UPDATE_MODE_DU4);

    _canvas_title->pushCanvas(0, 8, UPDATE_MODE_NONE);

    EPDGUI_AddObject(_key_exit);

    {
        File root = SPIFFS.open("/");
        File file = root.openNextFile();
        while (file)
        {
            String fileName = file.name();
            size_t fileSize = file.size();
            Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
            file = root.openNextFile();
        }
        Serial.printf("\n");
    }

    webOpen();

    return 3;
};
