/**
 * 
 */

#include <FS.h>

#include <Arduino.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

#define touchPin0 D1
#define touchPin1 D0

#include <time.h>
time_t current_time_t, last_time_t = 0;
struct tm *current_time;

#include <list>
#include <string>
using namespace std;

#include <Streaming.h>

#include <tinyxml2.h>
using namespace tinyxml2;

#include <ESP8266HTTPClient.h>

#include <SPI.h>

#include <ESP8266WiFi.h>

#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include <ESP8266mDNS.h>

#include <DNSServer.h>

#include <WiFiUdp.h>

// https://github.com/arduino-libraries/NTPClient
#include <NTPClient.h>
WiFiUDP ntpUDP;

char ntp_server[40] = "de.pool.ntp.org";

const char* server = "http://programmingexcuses.com";

NTPClient timeClient(ntpUDP);

#include <ArduinoOTA.h>

// https://github.com/tzapu/WiFiManager
#include <WiFiManager.h>
#define ConfigSSID      "ProgrammingExcuses"
#define ConfigSSIDPass  "ProgrammingExcuses"

//flag for saving data
bool shouldSaveConfig = false;

#define DISPLAY_RST   D4     // RST pin is connected to NodeMCU pin D4 (GPIO2)
#define DISPLAY_CS    D3     // CS  pin is connected to NodeMCU pin D4 (GPIO0)
#define DISPLAY_DC    D2     // DC  pin is connected to NodeMCU pin D4 (GPIO4)
// initialize U8g2 library with hardware SPI module
// SCK (CLK) ---> NodeMCU pin D5 (GPIO14)
// MOSI(DIN) ---> NodeMCU pin D7 (GPIO13)

#include <U8g2lib.h>
U8G2_ST7565_PE12864_004_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ DISPLAY_CS, /* dc=*/ DISPLAY_DC, /* reset=*/ DISPLAY_RST);

char time_str[9] = {0};
char date_show_str[11] = {0};

char excuse[128] = "";

list<string> l;

/**
 * 
 */
static bool IsDst(int hour, int day, int month, int dow) {
  if ((month < 3) || (month > 10)) {
    return false;
  }

  if ((month > 3) && (month < 10)) {
    return true;
  }

  int previousSunday = day - dow;

  if ((month == 3) && (previousSunday >= 25)) {
    return hour >= 2;
  }

  if ((month == 10) && (previousSunday <= 25)) {
    return hour < 2;
  }

  return false; // this line never gonna happen
}

/**
 * 
 */
void configModeCallback (WiFiManager *myWiFiManager) {
  unsigned char mac_addr[6] = {0};
  char mac_addr_string[4 + 12 + 5] = {0};

  WiFi.macAddress(mac_addr);

  //                                     MAC XX:XX:XX:XX:XX:XX 
  snprintf(mac_addr_string, 5 + 12 + 5, "MAC %02X:%02X:%02X:%02X:%02X:%02X",
    mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

  Serial << F("Entered config mode") << endl;
  Serial << F("AP-IP: ") << WiFi.softAPIP() << endl;

  Serial << F("AP-SSID: ") << myWiFiManager->getConfigPortalSSID() << endl;

  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_6x10_tr);	// choose a suitable font

  u8g2.drawStr(9, 8, "ProgrammingExcuses");	// write something to the internal memory
  u8g2.drawStr(0, 18, mac_addr_string);
  u8g2.drawStr(6, 28, "AP-SSID:");
  u8g2.drawStr(0, 38, myWiFiManager->getConfigPortalSSID().c_str());
  u8g2.drawStr(6, 48, "AP-Passphrase:");
  u8g2.drawStr(0, 58, ConfigSSIDPass);

  u8g2.sendBuffer();					// transfer internal memory to the display
}

/**
 * 
 */
void saveConfigCallback (void) {
  Serial << F("Should save config") << endl;

  shouldSaveConfig = true;
}

/**
 * https://stackoverflow.com/questions/6891652/formatting-a-string-into-multiple-lines-of-a-specific-length-in-c-c
 */
list<string> wraptext(string input, size_t width) {
  Serial << F("Input: [") << input.c_str() << F("] width: [") << width << F("]") << endl;
  
  size_t curpos = 0;
  size_t nextpos = 0;

  list<string> lines;
  string substr = input.substr(curpos, width + 1);

  while (substr.length() == width + 1 && (nextpos = substr.rfind(' ')) != input.npos) {
    lines.push_back(input.substr(curpos, nextpos));
    curpos += nextpos + 1;
    substr = input.substr(curpos, width + 1);
  }

  if (curpos != input.length()) {
    lines.push_back(input.substr(curpos, input.npos));
  }

  return lines;
}

/** 
 *
 */
bool refresh_excuse(void) {
  bool retval = false;

  WiFiClient wifiClient;
  HTTPClient httpClient;

  httpClient.begin(wifiClient, String(server));

  int http_response = httpClient.GET();

  if (http_response == 200) {
    String payload = httpClient.getString();

    // Serial << F("Payload: ") << payload << endl;

    XMLDocument doc;
    doc.Parse(payload.c_str());

    XMLElement *a_ele, *wrapper_ele, *html_body = doc.FirstChildElement("html")->FirstChildElement("body");

    wrapper_ele = html_body->FirstChildElement("div");

    // ensure wrapper_ele contains div with class="wrapper"
    while (strcmp(wrapper_ele->Attribute("class"), "wrapper")) {
      wrapper_ele = wrapper_ele->NextSiblingElement();
    }

    a_ele = wrapper_ele->FirstChildElement("center")->FirstChildElement("a");

    Serial << F("Excuse: [") << a_ele->GetText() << F("]") << endl;

    memset(excuse, 0, 128);
    strncpy(excuse, a_ele->GetText(), 128);

    retval = true;
  }

  httpClient.end();

  return retval;
}

/**
 * 
 */
void setup() {
  pinMode(touchPin0, INPUT);
  pinMode(touchPin1, INPUT);

  char hostname_string[19 + 7] = {0};
  sprintf(hostname_string, "programmingexcuses-%06x", ESP.getChipId());

  //clean FS, for testing
  SPIFFS.format();
  
  Serial.begin(115200);

  u8g2.begin();
  
  u8g2.setFlipMode(true);
  u8g2.enableUTF8Print();

  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_6x10_tr);	// choose a suitable font
  u8g2.drawStr(9, 8, "ProgrammingExcuses");	// write something to the internal memory
  u8g2.drawStr(29, 18, "- starting -");

  u8g2.sendBuffer();					// transfer internal memory to the display

  if (SPIFFS.begin()) {
    Serial << F("mounted file system") << endl;
    
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial << F("reading config file") << endl;

      File configFile = SPIFFS.open("/config.json", "r");

      if (configFile) {
        Serial << F("opened config file") << endl;
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);

        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());

        json.printTo(Serial);

        if (json.success()) {
          Serial << endl << F("parsed json") << endl;

          strcpy(ntp_server, json["ntp_server"]);

        } else {
          Serial << F("failed to load json config") << endl;
        }
        configFile.close();
      }
    }
  } else {
    Serial << F("failed to mount FS") << endl;
  }

  WiFi.hostname(hostname_string);

  WiFiManagerParameter custom_ntp_server("ntp_server", "ntp server", ntp_server, 40);
  WiFiManager wifiManager;

  //wifiManager.resetSettings();

  wifiManager.setTimeout(120);

  wifiManager.addParameter(&custom_ntp_server);

  wifiManager.setAPCallback(configModeCallback);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  if (!wifiManager.autoConnect(ConfigSSID, ConfigSSIDPass)) {
    Serial << F("failed to connect, we should reset as see if it connects") << endl;
    delay(3000);

    ESP.reset();
    delay(5000);
  }

  Serial << F("ST-SSID: ") << WiFi.SSID() << endl;
  Serial << F("ST-IP  : ") << WiFi.localIP().toString() << endl;

  strcpy(ntp_server, custom_ntp_server.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial << F("saving config") << endl;

    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["ntp_server"] = ntp_server;

    File configFile = SPIFFS.open("/config.json", "w");

    if (!configFile) {
      Serial << F("failed to open config file for writing") << endl;
    }

    json.printTo(Serial);
    json.printTo(configFile);

    configFile.close();
    //end save
  }

  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_6x10_tr);	// choose a suitable font
  u8g2.drawStr(9, 8, "ProgrammingExcuses");	// write something to the internal memory
  u8g2.drawStr(6, 18, "ST-SSID:");
  u8g2.drawStr(0, 28, WiFi.SSID().c_str());
  u8g2.drawStr(6, 38, "ST-IP:");
  u8g2.drawStr(0, 48, WiFi.localIP().toString().c_str());
  u8g2.drawStr(0, 58, ntp_server);

  u8g2.sendBuffer();					// transfer internal memory to the display

  delay(2500);

  while (WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial << ".";
  }

  if (MDNS.begin(hostname_string, WiFi.localIP(), 120)) {
    Serial << F("MDNS Responder started") << endl;

    MDNS.addService("workstation", "tcp", 9);
    MDNS.update();
  } else {
    Serial << F("Failed staring MDNS Responder") << endl;
  }

  timeClient.setPoolServerName(ntp_server);
  timeClient.begin();

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]

  ArduinoOTA.setHostname(hostname_string);
  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;

    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_6x10_tr);	// choose a suitable font
    u8g2.drawStr(9, 8, "ProgrammingExcuses");	// write something to the internal memory
    u8g2.drawStr(12, 18, "updating Firmware");

    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
      u8g2.drawStr(45, 28, type.c_str());
    } else { // U_FS
      type = "filesystem";
      u8g2.drawStr(33, 28, type.c_str());
    }

    u8g2.sendBuffer();					// transfer internal memory to the display

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial << F("Start updating ") << type << endl;
  });

  ArduinoOTA.onEnd([]() {
    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_6x10_tr);	// choose a suitable font
    u8g2.drawStr(9, 8, "ProgrammingExcuses");	// write something to the internal memory
    u8g2.drawStr(9, 18, "receiving Firmware");

    u8g2.drawStr(54, 38, "End");
    u8g2.drawStr(27, 48, "now flashing");
    
    u8g2.sendBuffer();					// transfer internal memory to the display

    Serial << endl << F("End") << endl;
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    char c_progress[5] = {0};

    snprintf(c_progress, 5, "%03u%%", (progress / (total / 100)));

    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_6x10_tr);	// choose a suitable font
    u8g2.drawStr(9, 8, "ProgrammingExcuses");	// write something to the internal memory
    u8g2.drawStr(9, 18, "receiving Firmware");

    u8g2.drawStr(36, 38, "Progress:");
    u8g2.drawStr(51, 48, c_progress);

    u8g2.sendBuffer();					// transfer internal memory to the display

    Serial << F("Progress: ") << c_progress << "\r";
  });

  ArduinoOTA.onError([](ota_error_t error) {
    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_6x10_tr);	// choose a suitable font
    u8g2.drawStr(9, 8, "ProgrammingExcuses");	// write something to the internal memory
    u8g2.drawStr(12, 18, "updating Firmware");

    u8g2.drawStr(45, 38, "ERROR:");

    Serial << F("Error[") << error << F("]:");

    if (error == OTA_AUTH_ERROR) {
      u8g2.drawStr(30, 48, "Auth Failed");
      Serial << F("Auth Failed") << endl;
    } else if (error == OTA_BEGIN_ERROR) {
      u8g2.drawStr(27, 48, "Begin Failed");
      Serial << F("Begin Failed") << endl;
    } else if (error == OTA_CONNECT_ERROR) {
      u8g2.drawStr(21, 48, "Connect Failed");
      Serial << F("Connect Failed") << endl;
    } else if (error == OTA_RECEIVE_ERROR) {
      u8g2.drawStr(21, 48, "Receive Failed");
      Serial << F("Receive Failed") << endl;
    } else if (error == OTA_END_ERROR) {
      u8g2.drawStr(33, 48, "End Failed");
      Serial << F("End Failed") << endl;
    }

    u8g2.sendBuffer();					// transfer internal memory to the display
  });

  ArduinoOTA.begin();

  // init excuse-lines
  l = wraptext(excuse, 21);
}

int touchValue0 = LOW;
int touchValue1 = LOW;

/**
 * 
 */
void loop() {
  timeClient.update();
  
  MDNS.update();

  ArduinoOTA.handle();

  touchValue0 = digitalRead(touchPin0);
  touchValue1 = digitalRead(touchPin1);

  current_time_t = (time_t) timeClient.getEpochTime();
  current_time = localtime(&current_time_t);

  if (IsDst(current_time->tm_hour, current_time->tm_mday, current_time->tm_mon, current_time->tm_wday)) {
    timeClient.setTimeOffset(60 * 60 * 2); // CEST
  } else {
    timeClient.setTimeOffset(60 * 60);     // CET
  }

  current_time_t = (time_t) timeClient.getEpochTime();
  current_time = localtime(&current_time_t);

  snprintf(time_str, 9, "%02d:%02d:%02d", current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
  snprintf(date_show_str, 11, "%04d/%02d/%02d", (current_time->tm_year + 1900), current_time->tm_mon + 1, current_time->tm_mday);

  Serial << F("Date: ") << date_show_str << endl;
  Serial << F("Time: ") << time_str << endl;

  u8g2.clearBuffer();					// clear the internal memory

  if ((current_time_t - last_time_t) >= 300) {
    last_time_t = current_time_t;

    if (refresh_excuse()) {
      l = wraptext(excuse, 21);
    }
  }

  u8g2.setFont(u8g2_font_6x10_tr);	// choose a suitable font

  if (touchValue0 == HIGH) {
    u8g2.drawStr(2, 9, "X");
  }

  u8g2.drawStr(9, 8, "ProgrammingExcuses");

  if (touchValue1 == HIGH) {
    u8g2.drawStr(127 - 8, 9, "X");
  }

  char line[22] = {0};

  // 4 lines => y = 21; ( 0)
  // 3 lines => y = 26; ( 5)
  // 2 lines => y = 31; (10)
  // 1 line  => y = 36; (15)
  int x, y = 21 + (20 - l.size() * 5);

  for (auto i = l.begin(); i != l.end(); ++i, y+=10) {
    strncpy(line, i->c_str(), 21);

    x = (128 - (strlen(line) * 6)) / 2;

    Serial << F("y: [") << y << F("] x: [") << x << F("] line: [") << line << F("]") << endl;

    u8g2.drawStr(x, y, line);
  }

  u8g2.drawStr(7, 63, (String(date_show_str) + " " + String(time_str)).c_str());

  u8g2.sendBuffer();

  delay(500);
}
