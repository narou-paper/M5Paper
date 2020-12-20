#include "frame_receive.h"
#include "WiFi.h"
#include "WebServer.h"
#include "ESPmDNS.h"
#include "ArduinoJson.h"

#define FORMAT_FILESYSTEM true

const char *ssid = "Narou-Paper";
const char *password = "mokemoke";
const char *host = "narou";

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

void editConfigWriteSubtitle(String title, String episode, String subtitle)
{
  DynamicJsonDocument doc(10*1024);
  getDJD(doc);

  doc[title][episode]["subtitle"] = subtitle;

  writeDJD(doc);
  doc.clear();
}

void editConfigAddFile(String title, String episode, String filename)
{
  DynamicJsonDocument doc(10*1024);
  getDJD(doc);

  JsonArray files = doc[title][episode]["files"];
  if (files.isNull())
    files = doc[title][episode].createNestedArray("files");
  files.add("/" + filename);

  writeDJD(doc);
  doc.clear();
}

void editConfigWriteFiles(String title, String episode)
{
  DynamicJsonDocument doc(5*1024);
  getDJD(doc);

  JsonArray files = doc[title][episode].createNestedArray("files");

  for (String path : filepaths)
  {
    files.add(path);
  }

  writeDJD(doc);
  doc.clear();
}

void webOpenHandler(void *pvParameter)
{
    while (1)
    {
        _web_server.handleClient();
    }
}

bool handleFileRead(String path)
{
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/"))
  {
    path += "index.html";
  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (exists(pathWithGz) || exists(path))
  {
    if (exists(pathWithGz)) {
      path += ".gz";
    }
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
  // Serial.print("upload.filename: ");
  // Serial.println(upload.filename);
  // Serial.print("upload.name: ");
  // Serial.println(upload.name);
  // Serial.print("upload.type: ");
  // Serial.println(upload.type);
  // Serial.print("upload.totalSize: ");
  // Serial.println(upload.totalSize);
  // Serial.print("upload.currentSize: ");
  // Serial.println(upload.currentSize);
  // for (int i = 0; i < upload.currentSize; i++)
  // {
  //   Serial.printf("%c ", upload.buf[i]);
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
    Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
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

void Frame_Receive::webOpen()
{
    WiFi.softAP(ssid, password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    // MDNS.begin(host);
    // Serial.print("Open http://");
    // Serial.print(host);
    // Serial.println(".local/");

    String message = "1. Open http://" + myIP.toString();
    _receive_canvas.drawString(message, 30, 45);
    message = "2. Upload image files.";
    _receive_canvas.drawString(message, 30, 100);
    _receive_canvas.pushCanvas(80, 440, UPDATE_MODE_DU4);

    _web_server.begin();
    xTaskCreatePinnedToCore(webOpenHandler, "webOpen", 4096, NULL, 1, &xHandle, 0);
};

void Frame_Receive::webClose()
{
    _web_server.close();
    vTaskDelete(xHandle);
    WiFi.softAPdisconnect(true);
};

Frame_Receive::Frame_Receive(void)
{
    _frame_name = "Frame_Receive";

    exitbtn("Home");

    _canvas_title->drawString("Receive", 270, 34);

    _key_exit->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void *)(&_is_run));
    _key_exit->Bind(EPDGUI_Button::EVENT_RELEASED, &Frame_Base::exit_cb);

    _web_server.on("/index.html", HTTP_GET, []() {
      if (!handleFileRead("/index.html")) {
        _web_server.send(404, "text/plain", "FileNotFound");
      }
    });
    _web_server.on("/favicon.ico", HTTP_GET, []() {
      if (!handleFileRead("/favicon.ico")) {
        _web_server.send(404, "text/plain", "FileNotFound");
      }
    });
    _web_server.on("/app.js", HTTP_GET, []() {
      if (!handleFileRead("/app.js")) {
        _web_server.send(404, "text/plain", "FileNotFound");
      }
    });
    _web_server.on("/chunk-vendors.css", HTTP_GET, []() {
      if (!handleFileRead("/chunk-vendors.css")) {
        _web_server.send(404, "text/plain", "FileNotFound");
      }
    });
    _web_server.on("/chunk-vendors.js", HTTP_GET, []() {
      if (!handleFileRead("/chunk-vendors.js")) {
        _web_server.send(404, "text/plain", "FileNotFound");
      }
    });
    // _web_server.serveStatic("/index.html", SPIFFS, "/index.html");
    // _web_server.serveStatic("/favicon.ico", SPIFFS, "/favicon.ico.gz");
    // _web_server.serveStatic("/app.js", SPIFFS, "/app.js.gz");
    // _web_server.serveStatic("/chunk-vendors.css", SPIFFS, "/chunk-vendors.css.gz");
    // _web_server.serveStatic("/chunk-vendors.js", SPIFFS, "/chunk-vendors.js.gz");
    _web_server.serveStatic("/config.json", SD, "/config.json"); //デバッグ用に

    _web_server.on(
        "/", HTTP_POST, []() {
        String novelTitle = _web_server.arg("title");
        String episode = _web_server.arg("episode");
        String subtitle = _web_server.arg("subtitle");

        // Serial.println(novelTitle);
        // Serial.println(episode);
        // Serial.println(subtitle);

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

    _receive_canvas.createCanvas(380, 140);
    _receive_canvas.fillCanvas(15);
    _receive_canvas.setTextSize(26);
    _receive_canvas.setTextColor(0);
    _receive_canvas.setTextDatum(CL_DATUM);

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
