#include <FS.h>

#include "htmlHandler.h"
#include "program.h"
#include "proghandler.h"

#include "WebSocketServer_LEDcontrol.h"


void htmlPrintDirectory(ESP8266WebServer &server) {
  String indent = "";
  String path = "/";
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n<title>lightmaster</title>\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"stylesheet\" type=\"text/css\" href=\"lightmaster.css\">\n</head>\n<body class=\"body\">\n";
  while(dir.next()){
    output += indent;
    output += "<a href=\"" + dir.fileName() + "\">";
    output += dir.fileName();
    output += "</a>";
    output += "<br>\n";
  }
  
  output += "\n</body>\n</html>\n";
  server.send(200, "text/html", output);
}

void htmlSaveProg(ESP8266WebServer &server) {
  Program newProgram;
  String newProgramName;

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    USE_SERIAL.printf("argument [%s] has data [%s]\n", server.argName( i ).c_str(), server.arg( i ).c_str());
  }
  
  if (server.arg("oldName") != "") {
    proghandler.deleteFile(server.arg("oldName"));
  }

  newProgramName = server.arg("name");
  
  newProgram.m_numOfSteps = server.arg("steps").toInt();
  newProgram.m_masterSetting = server.arg("total").toInt();
  newProgram.m_CrossfadeSetting = server.arg("fade").toInt();
  newProgram.m_stepTime = server.arg("speed").toInt();
  
  static const uint8_t MAX_NUM_OF_STEPS = 100;
  static const uint8_t NUM_OF_CHANNELS = 7;

  String progString = server.arg("prog%5B%5D");
  if (newProgram.m_numOfSteps > MAX_NUM_OF_STEPS) {
    newProgram.m_numOfSteps = MAX_NUM_OF_STEPS;
  }

  USE_SERIAL.printf("data of \"prog\" is: %s\n", progString.c_str());
  for (uint8_t stepNr = 0 ; stepNr < newProgram.m_numOfSteps ; stepNr++) {
    for (uint8_t ch = 0 ; ch < NUM_OF_CHANNELS ; ch++) {
      // USE_SERIAL.printf("%s: %d %s\n", progString.c_str(), progString.indexOf(","), progString.substring(0, progString.indexOf(",")).c_str());
      uint8_t value = progString.substring(0, progString.indexOf(",")).toInt();
      newProgram.m_prog[ch][stepNr] = value;
      progString = progString.substring(progString.indexOf(",") + 1);
      USE_SERIAL.printf(" %d", newProgram.m_prog[ch][stepNr]);
    }
    USE_SERIAL.printf("\n");
  }
  
  proghandler.writeFile(newProgramName, newProgram);

  // server.sendHeader("Location","http://httpbin.org/post");
  server.sendHeader("Location","/editMenu.html");
  server.send(301);
}

