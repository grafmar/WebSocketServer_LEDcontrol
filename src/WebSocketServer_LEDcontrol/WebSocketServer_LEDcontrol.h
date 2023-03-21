#ifndef WEBSOCKETSERVER_LEDCONTROL_H
#define WEBSOCKETSERVER_LEDCONTROL_H

#include <WebSocketsServer.h>
#include "sequencer.h"
#include "proghandler.h"
#include "program.h"

#define USE_SERIAL Serial

extern const String faderOnlyString;

extern WebSocketsServer webSocket;
extern Sequencer sequencer;
extern Proghandler proghandler;

extern Program program;
extern String programName;


#endif // WEBSOCKETSERVER_LEDCONTROL_H

