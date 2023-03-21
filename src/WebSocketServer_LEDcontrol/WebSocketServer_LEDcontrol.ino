/*
 * WebSocketServer_LEDcontrol.ino
 *
 *  Created on: 26.11.2015
 *
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <Hash.h>
#include <FS.h>

#include "WebSocketServer_LEDcontrol.h"
#include "spiffsFile.h"
#include "htmlHandler.h"
#include "sequencer.h"
#include "proghandler.h"
#include "program.h"
#include "webSocketHandler.h"

// select wich pin will trigger the configuraton portal when set to LOW
// Pin 0 is "FLASH"-button on nodeMCU
#define TRIGGER_PIN 0




extern const String faderOnlyString = "FaderOnly";

ESP8266WebServer server = ESP8266WebServer(80);
WebSocketsServer webSocket = WebSocketsServer(81);
Sequencer sequencer = Sequencer();
Proghandler proghandler = Proghandler("/prog/");
String programName = faderOnlyString;

Program program = {
  .m_numOfSteps = 3,
  .m_masterSetting = 255,
  .m_CrossfadeSetting = 128,
  .m_stepTime = 1000,
  .m_prog = {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 0, 0}
  }
};


void setup() {
  USE_SERIAL.begin(115200);
  
  wifi_station_set_hostname("ESP-Light");
  
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(TRIGGER_PIN, INPUT);

  USE_SERIAL.printf("Initialized pinMode\n");
  USE_SERIAL.flush();

  if (!SPIFFS.begin()) {
    USE_SERIAL.println("Failed to mount file system");
  }

  
  // WiFiManager start WiFi or AP if no connection possible
  WiFiManager wifiManager;
  wifiManager.setTimeout(180);
  
  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if(!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  
  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  // handle index
  server.on("/list", HTTP_GET, []() {
    htmlPrintDirectory(server);
  });
  server.on("/save", HTTP_POST, []() {
    htmlSaveProg(server);
  });
  /*
  server.on("/", []() {
    // send index.html
    server.send(200, "text/html", "<html><head><script>var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);connection.onopen = function () {  connection.send('Connect ' + new Date()); }; connection.onerror = function (error) {    console.log('WebSocket Error ', error);};connection.onmessage = function (e) {  console.log('Server: ', e.data);};function sendRGB() {  var r = parseInt(document.getElementById('r').value).toString(16);  var g = parseInt(document.getElementById('g').value).toString(16);  var b = parseInt(document.getElementById('b').value).toString(16);  if(r.length < 2) { r = '0' + r; }   if(g.length < 2) { g = '0' + g; }   if(b.length < 2) { b = '0' + b; }   var rgb = '#'+r+g+b;    console.log('RGB: ' + rgb); connection.send(rgb); }</script></head><body>LED Control:<br/><br/>R: <input id=\"r\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" onchange=\"sendRGB();\" /><br/>G: <input id=\"g\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" onchange=\"sendRGB();\" /><br/>B: <input id=\"b\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" onchange=\"sendRGB();\" /><br/></body></html>");
  });
  */
  server.onNotFound([]() {
    USE_SERIAL.printf("request: %s\n", server.uri().c_str());
    if(!handleFileRead(server.uri(), server))
      server.send(404, "text/plain", "FileNotFound");
  });
  
  server.begin();
  
  sequencer.setFaderMode();
}

void onDemandAP() {
  server.stop();                    // stop running server first

  WiFiManager wifiManager;          // Local intialization. Once its business is done, there is no need to keep it around
  // wifiManager.resetSettings();      // reset settings - for testing
  
  // set timeout until configuration portal gets turned off
  wifiManager.setTimeout(120);
  
  if (!wifiManager.startConfigPortal("OnDemandAP")) {
    delay(3000);
    ESP.reset();                  //reset and try again, or maybe put it to deep sleep
    delay(5000);
  }
}

void loop() {
  // on demand AP
  if ( digitalRead(TRIGGER_PIN) == LOW ) {
    // onDemandAP();
  }
  
  webSocket.loop();
  server.handleClient();
  
  sequencer.refreshOutputs();
}


