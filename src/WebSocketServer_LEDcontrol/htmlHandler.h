#ifndef HTMLHANDLER_H
#define HTMLHANDLER_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

void htmlPrintDirectory(ESP8266WebServer &server);
void htmlSaveProg(ESP8266WebServer &server);

#endif // HTMLHANDLER_H

