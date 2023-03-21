#include <Arduino.h>
#include <list>

#include "webSocketHandler.h"
#include "WebSocketServer_LEDcontrol.h"

typedef std::list<uint8_t> socketList_t;
socketList_t socketList;

void sendFaderTo(uint8_t num);
void sendFaderToOtherThan(uint8_t num);
void sendProgTo(uint8_t num);
void sendProgToOtherThan(uint8_t num);
void sendRequestedProgram(uint8_t num, Program& program);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  String payloadString = String((char*)payload);

  switch(type) {
    case WStype_DISCONNECTED:
      USE_SERIAL.printf("[%u] Disconnected!\n", num);
      socketList.remove(num);
      break;
      
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        socketList.push_back(num);
  
        sendFaderTo(num);
        
        // send message to client
        webSocket.sendTXT(num, "p:" + faderOnlyString);
        std::list<String> progList = proghandler.listProgs();
        while(!progList.empty()) {
          webSocket.sendTXT(num, String("p:") + progList.front());
          progList.pop_front();
        }
        // active Program
        webSocket.sendTXT(num, String("P:") + programName);
      }
      break;
      
    case WStype_TEXT:
      {
        // USE_SERIAL.printf("[%u] get Text: length %d [%s] -> %d [%s]\n", num, lenght, payload, payloadString.length(), payloadString.c_str());
        USE_SERIAL.printf("prefix: [%s]\n", payloadString.substring(0,2).c_str());
        String cmd = payloadString.substring(0,2);
        String data = payloadString.substring(2);
        USE_SERIAL.printf("payload: [%s], payload length: [%d], msg length: [%d]\n", payloadString.c_str(), payloadString.length(), lenght);
        USE_SERIAL.printf("prefix: [%s], data: [%s]\n", cmd.c_str(), data.c_str());
        
        if (cmd == "p:") {
          programName = data;
          if (data == faderOnlyString) {
            USE_SERIAL.printf("Fader mode\n");
            sequencer.setFaderMode();
          } else {
            USE_SERIAL.printf("Set Program: %s\n", programName.c_str());
            proghandler.readFile(programName, program);
            sequencer.setProgram(program);
            sendFaderTo(num);
            sendProgTo(num);
            sendFaderToOtherThan(num);
            sendProgToOtherThan(num);
          }
        } else if (cmd == "D:") {
          USE_SERIAL.printf("Delete Program: %s\n", data.c_str());
          proghandler.deleteFile(data);
        } else if (cmd == "U:") {
          sendFaderToOtherThan(num);
          sendProgToOtherThan(num);
        } else if (cmd == "R:") {
          USE_SERIAL.printf("Requsted Program: %s\n", data.c_str());
          proghandler.readFile(data, program);
          sendRequestedProgram(num, program);
        }
      }
      break;
      
    case WStype_BIN:
      // USE_SERIAL.printf("BIN: l=%d %d\n", lenght, payload[0]);
      if (lenght >= 11) {
        sequencer.setMaster(payload[0]);
        sequencer.setCrossfade(payload[1]);
        uint16_t stepTime = 256 * static_cast<uint16_t>(payload[2]) + payload[3];
        sequencer.setStepTime(static_cast<uint16_t>(stepTime));
        sequencer.setLampValue(0, payload[4]);
        sequencer.setLampValue(1, payload[5]);
        sequencer.setLampValue(2, payload[6]);
        sequencer.setLampValue(3, payload[7]);
        sequencer.setLampValue(4, payload[8]);
        sequencer.setLampValue(5, payload[9]);
        sequencer.setLampValue(6, payload[10]);
      }
      break;
      
    default:
      break;
  }
}

void sendFaderTo(uint8_t num) {
  uint8_t payload[13];
  payload[0] = 'f';
  payload[1] = ':';
  payload[2] = sequencer.getMaster();
  payload[3] = sequencer.getCrossfade();
  payload[4] = (sequencer.getStepTime() / 256) & 0x00FF;
  payload[5] = sequencer.getStepTime() & 0x00FF;
  payload[6] = sequencer.getLampValue(0);
  payload[7] = sequencer.getLampValue(1);
  payload[8] = sequencer.getLampValue(2);
  payload[9] = sequencer.getLampValue(3);
  payload[10] = sequencer.getLampValue(4);
  payload[11] = sequencer.getLampValue(5);
  payload[12] = sequencer.getLampValue(6);
  webSocket.sendBIN(num, payload, 13);
}

void sendFaderToOtherThan(uint8_t num) {
  for (socketList_t::iterator it = socketList.begin() ; it != socketList.end() ; ++it) {
    if (*it != num ){
      sendFaderTo(*it);
    }
  }
}


void sendProgTo(uint8_t num) {
  webSocket.sendTXT(num, String("P:") + programName);
}

void sendProgToOtherThan(uint8_t num) {
  for (socketList_t::iterator it = socketList.begin() ; it != socketList.end() ; ++it) {
    if (*it != num ){
      sendProgTo(*it);
    }
  }
}

#define MAX_LENGTH (2 + 5 + (Program::MAX_NUM_OF_STEPS * Program::NUM_OF_CHANNELS))

void sendRequestedProgram(uint8_t num, Program& program) {
	uint8_t payload[MAX_LENGTH];
	uint16_t length = 2 + 5 + (program.m_numOfSteps * Program::NUM_OF_CHANNELS);

	payload[0] = 'r';
	payload[1] = ':';
  payload[2] = program.m_numOfSteps;
  payload[3] = program.m_masterSetting;
	payload[4] = program.m_CrossfadeSetting;
	payload[5] = (program.m_stepTime / 256) & 0x00FF;
	payload[6] = program.m_stepTime & 0x00FF;

  for (uint8_t step = 0 ; step < program.m_numOfSteps ; step++) {
    for (uint8_t ch = 0 ; ch < Program::NUM_OF_CHANNELS ; ch++) {
      payload[7 + (step * Program::NUM_OF_CHANNELS) + ch] = program.m_prog[ch][step];
    }
	}

	webSocket.sendBIN(num, payload, length);
}

