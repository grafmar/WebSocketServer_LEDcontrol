#ifndef PROGHANDLER_H
#define PROGHANDLER_H

// Sequencer to play programs

#include <Arduino.h>
#include <list>
#include "program.h"

class Proghandler {
public:
  
  explicit Proghandler(const String progRoot);
  ~Proghandler();
  
  void readFile(String progName, Program& program);
  void writeFile(String progName, Program& program);
  void deleteFile(String progName);
  std::list<String> listProgs();


private:
  String extractProgName(String path);
  bool isProg(String path);
  long saveStringToNumber(String input, long min, long max);
  String saveStringToFilename(String progName);
  bool charValid(char c);

  const uint8_t MAX_PROGNAME_LENGHT = 30;
  
  String m_progRoot;
};


#endif // PROGHANDLER_H

