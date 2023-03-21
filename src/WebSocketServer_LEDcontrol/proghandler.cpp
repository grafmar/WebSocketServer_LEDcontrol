
#include "proghandler.h"
#include "WebSocketServer_LEDcontrol.h"
#include <FS.h>



Proghandler::Proghandler(const String progRoot) :
  m_progRoot(progRoot)
{
}

Proghandler::~Proghandler() {
}


void Proghandler::readFile(String progName, Program& program) {
  progName = m_progRoot + progName + ".prog";
  if (SPIFFS.exists(progName)) {
    program.m_numOfSteps = 0;
    File f = SPIFFS.open(progName, "r");
    if (f) {
      while(f.available()) {
        //Lets read line by line from the file
        String line = f.readStringUntil('\n');
        //Serial.println(line);
        
        if (line.startsWith("M:")) {
          program.m_masterSetting = saveStringToNumber(line.substring(2), 0, 255);
        }
        else if (line.startsWith("F:")) {
          program.m_CrossfadeSetting = saveStringToNumber(line.substring(2), 0, 255);
        }
        else if (line.startsWith("S:")) {
          program.m_stepTime = saveStringToNumber(line.substring(2), 1, 0xFFFF);
        }
        else {
          line = line.substring(line.indexOf(":") + 1);
          if (line.length() >= 13) {
            for (int ch = 0 ; ch < program.NUM_OF_CHANNELS-1 ; ch++) {
              program.m_prog[ch][program.m_numOfSteps] = saveStringToNumber(line.substring(0, line.indexOf(",")), 0, 255);;          
              line = line.substring(line.indexOf(",") + 1);
              //Serial.print(" " + String(program.m_prog[ch][program.m_numOfSteps]));
            }
            program.m_prog[program.NUM_OF_CHANNELS-1][program.m_numOfSteps] = saveStringToNumber(line, 0, 255);;          
            //Serial.print(" " + String(program.m_prog[program.NUM_OF_CHANNELS-1][program.m_numOfSteps]));
            //Serial.println("");
            program.m_numOfSteps++;
          }
        }
      }
      f.close();
    }
  }
}

void Proghandler::writeFile(String progName, Program& program) {
  progName = saveStringToFilename(progName);
  String filename = m_progRoot + progName + ".prog";
  
  File file = SPIFFS.open(filename, "w");
  if (!file) {
    USE_SERIAL.printf("Can not write file %s\n", filename.c_str());
    return;
  }
  USE_SERIAL.printf("Writing file %s:\n\n", filename.c_str());

  file.printf("M:%d\n", program.m_masterSetting);
  file.printf("F:%d\n", program.m_CrossfadeSetting);
  file.printf("S:%d\n", program.m_stepTime);
  
  for (uint8_t stepNr = 1; stepNr <= program.m_numOfSteps ; stepNr++) {
    file.printf("%d:", stepNr);
    for (int ch = 0 ; ch < program.NUM_OF_CHANNELS-1 ; ch++) {
      file.printf("%d,", program.m_prog[ch][stepNr-1]);
    }
    file.println(program.m_prog[program.NUM_OF_CHANNELS-1][stepNr-1]);
  }

  file.close();
}

std::list<String> Proghandler::listProgs() {
  std::list<String> listOfProgs;
  Dir dir = SPIFFS.openDir(m_progRoot);

  while (dir.next()) {
    if (isProg(dir.fileName())) {
      listOfProgs.push_back(extractProgName(dir.fileName()));
      // listOfProgs.push_front(extractProgName(dir.fileName()));
    }
  }
  
  return listOfProgs;
}

void Proghandler::deleteFile(String progName) {
  progName = m_progRoot + progName + ".prog";
  if (SPIFFS.exists(progName)) {
    SPIFFS.remove(progName);
  }
}

String Proghandler::extractProgName(String path) {
  return path.substring(m_progRoot.length(), path.length() - String(".prog").length());
}

bool Proghandler::isProg(String path) {
  return path.endsWith(".prog");
}

long Proghandler::saveStringToNumber(String input, long min, long max) {
  long number = input.toInt();
  if (number < min) {
    number = min;
  } else if (number > max) {
    number = max;
  }
  return number;
}

String Proghandler::saveStringToFilename(String progName) {
  if (progName.length() > MAX_PROGNAME_LENGHT) {
    progName = progName.substring(0, MAX_PROGNAME_LENGHT);    
  }
  
  for (int i = 0 ; i < progName.length() ; i++) {
    if (!charValid(progName.charAt(i))) {
      progName.setCharAt(i, '_');
    }
  }

  if (progName.length() < 1) {
    progName = "_";
  }
  
  return progName;
}

bool Proghandler::charValid(char c) {
  if ( ((c >= 'a') && (c <= 'z'))
    ||  ((c >= 'A') && (c <= 'Z'))
    ||  ((c >= '0') && (c <= '9'))
    ||  (c == '_') )
  {
    return true;
  } else {
    return false;
  }
}

  const uint8_t MAX_PROGNAME_LENGHT = 30;
