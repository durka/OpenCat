/*  Please add http://arduino.esp8266.com/stable/package_esp8266com_index.json 
 *  to the "Additional boards manager URLs" in Preference 
 *  Then add ESP8266 in "Boards Manager"
 *  Install https://github.com/tzapu/WiFiManager by add .Zip Library
 *  
 *          Petoi WiFi module Config 
 *  Module name: ESP-WROOM-02D / 4MB Flash
 *  Board: Generic ESP8266 Module
 *  Built-in LED: IO2 (Pin 2)
 *  Upload speed 921600 (may downgrade to half automatically)
 *  CPU Frequency: 80MHz (Power saving, lower heat) / 160MHz (High performance)
 *  Crystal: 26MHz
 *  Flash size: 4MB (FS:2MB / OTA:1019KB recommend) 
 *  Flash mode: DQOUT (Compatible)
 *  Flash frequency: 40MHz
 *  Others: Default
 *  
 *  Connecting the WiFi dongle to the serial programmer, search and connect "Bittle-AP" in WiFi
 *  The setup page should pop up directly, or you may visit 192.168.4.1 
 *  Set up the connection to your router 
 *  Check the IP address assigned to the WiFi module, then you can visit the server by your web browser.
 */

 /*
  * TODO:
  * - [x] implement websocket-based serial terminal over wifi
  *     - [x] shift to async web server
  *     - [x] add websocket code
  *     - [x] implement terminal page
  * - [ ] show voltage on main page
  * - [x] notify when clients connect/disconnect to wifi
  * 
  * - [ ] flash arduino over wifi
  *     - [ ] upload firmware file (store on esp filesystem???)
  *     - [ ] implement flashing over serial
  */

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "commons.h"
#include "mainpage.h"
#include "actionpage.h"
#include "calibrationpage.h"

#define BUILTIN_LED 2
//#define DIRECT_SERIAL // serial port is directly connected to monitor (not Bittle)

#ifdef DIRECT_SERIAL
#define PT(...) Serial.print(__VA_ARGS__)
#define PTL(...) Serial.println(__VA_ARGS__)
#define PTF(...) Serial.printf(__VA_ARGS__)
#else
#define PT(...)
#define PTL(...)
#define PTF(...)
#endif

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

String renderHtml(String body, String title) {
  String page;
  page += FPSTR(head);
  page.replace(FPSTR("%TITLE%"), title);
  page += body;
  page += FPSTR(foot);
  return page;
}

void handleMainPage(AsyncWebServerRequest *request) {
  auto head_len = strlen(head);
  auto body_len = strlen(mainpage);
  auto foot_len = strlen(foot);
  auto response = request->beginChunkedResponse("text/html",
  [head_len, body_len, foot_len](uint8_t* data, size_t len, size_t filled_len) -> size_t {
    if (filled_len < head_len) {
      auto copy_len = min(len, head_len - filled_len);
      strncpy((char*)data, &head[filled_len], copy_len);
      return copy_len;
    } else if (filled_len < (head_len + body_len)) {
      auto copy_len = min(len, body_len - (filled_len - head_len));
      strncpy((char*)data, &mainpage[filled_len - head_len], copy_len);
      return copy_len;
    } else if (filled_len < (head_len + body_len + foot_len)) {
      auto copy_len = min(len, foot_len - (filled_len - head_len - body_len));
      strncpy((char*)data, &foot[filled_len - head_len - body_len], copy_len);
      return copy_len;
    } else {
      return 0;
    }
  });
  request->send(response);
}

void handleActionPage(AsyncWebServerRequest *request) {
  request->send(200, "text/html", renderHtml(FPSTR(actionpage), "Actions"));
}

void handleCalibrationPage(AsyncWebServerRequest *request) {
  request->send(200, "text/html", renderHtml(FPSTR(calibrationpage), "Calibration"));
  Serial.print("c");
}

void handleCalibration(AsyncWebServerRequest *request) {
  String joint = request->arg("c");
  String offset = request->arg("o");
    
  if (joint == "s") {
    Serial.print("s");
  } else {
    Serial.print("c" + joint + " " + offset);
  }
  request->send(200, "text/html", renderHtml(FPSTR(calibrationpage), "Calibration"));
}

void handleAction(AsyncWebServerRequest *request) {
  String argname = request->arg("name");

  if(argname == "gyro"){              // gyro switch
    Serial.print("g");
  }
  else if(argname == "calibration"){  // calibration mode
    Serial.print("c");
  }
  else if(argname == "balance"){      // balance mode
    Serial.print("kbalance");   
  }
  else if(argname == "walk"){         // demo walk
    Serial.print("kwkF");
  }
  else if(argname == "trot"){         // trot
    Serial.print("ktrF");
  }
  else if(argname == "crawl"){        // crawl
    Serial.print("kcrF");
  }
  else if(argname == "sit"){          // sit
    Serial.print("ksit");
  }
  else if(argname == "check"){        // check 
    Serial.print("kck");
  }
  else if(argname == "buttup"){       // butt UP
    Serial.print("kbuttUp");
  }
  else if(argname == "hello"){        // Say Hi~ 
    Serial.print("khi");
  }
  else if(argname == "stretch"){      // stretch body
    Serial.print("kstr");
  }
  else if(argname == "run"){          // run
    Serial.print("krnF");
  }
  else if(argname == "pee"){          // pee
    Serial.print("kpee");
  }
  else if(argname == "pushup"){       // pushup
    Serial.print("kpu");
  }
  else if(argname == "stepping"){     // stepping
    Serial.print("kvt");
  }
  else if(argname == "lookup"){       // lookup
    Serial.print("lu");
  }
  else if(argname == "forward"){
    Serial.print("kwkF");
  }
  else if(argname == "forwardleft"){
    Serial.print("kwkL");
  }
  else if(argname == "forwardright"){
    Serial.print("kwkR");
  }
  else if(argname == "backleft"){
    Serial.print("kbkL");
  }
  else if(argname == "backright"){
    Serial.print("kbkR");
  }
  else if(argname == "back"){
    Serial.print("kbk");
  }
  else if(argname == "stop"){
    Serial.print("d");
  }
  else {
    Serial.print(argname);      // pass through http argument to Bittle
  }

  // Return to actionpage after CMD
  handleActionPage(request);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    Serial.write(data, len);
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        PTF("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        Serial.print("kbalance");
        break;
      case WS_EVT_DISCONNECT:
        PTF("WebSocket client #%u disconnected\n", client->id());
        Serial.print("krest");
        break;
      case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void serialEvent() {
  String recv;
  while (Serial.available()) {
    char ch = Serial.read();
    recv += ch;
  }
  ws.textAll(recv);
}

void setup(void) {
  delay(1000);

  // Serial and GPIO LED
  Serial.begin(115200);
  PTL("Starting wifi...");
  pinMode(BUILTIN_LED, OUTPUT);

  WiFi.persistent(false);
  WiFi.mode(WIFI_AP);
  IPAddress ip(192, 168, 4, 1);
  WiFi.softAPConfig(ip, ip, IPAddress(255, 255, 255, 0));
  WiFi.softAP("BoosterBittle", "nyboardv1.1");
  digitalWrite(BUILTIN_LED, HIGH);      // While connected, LED lights

  // Print the IP get from DHCP Server of your Router
  PT("IP address: "); PTL(WiFi.softAPIP());

  // HTTP server started with handlers
  server.on("/", handleMainPage);
  server.on("/actionpage", handleActionPage);
  server.on("/action", handleAction);
  server.on("/calibrationpage", handleCalibrationPage);
  server.on("/calibration", handleCalibration);

  initWebSocket();
  server.begin();
  PTL("HTTP server started");
}

void loop(void) {
  ws.cleanupClients();
  
  // WiFi.onSoftAPModeStationConnected does *not* work
  static uint8_t prev_station_num = 0;
  uint8_t station_num = WiFi.softAPgetStationNum();
  if (station_num != prev_station_num) {
    if(station_num > prev_station_num) {
      PTL("Station connected!");
    } else {
      PTL("Station disconnected!");
    }
    prev_station_num = station_num;
  }
}
