#ifndef SEQUENCER_H
#define SEQUENCER_H

// Sequencer to play programs

#include <Arduino.h>
#include "program.h"

class Sequencer {
public:
  
  explicit Sequencer();
  ~Sequencer();
  
  void setMaster(uint8_t value);
  void setCrossfade(uint8_t value);
  void setStepTime(uint16_t value);
  
  uint8_t getMaster();
  uint8_t getCrossfade();
  uint16_t getStepTime();
  uint8_t getLampValue(uint8_t lamp);

  void setLampValue(uint8_t lamp, uint8_t value);
  
  /**
   * Sets to program mode and uses the given program
   */
  void setProgram(Program &prog);
  
  /**
   * Sets to fader mode only
   */
  void setFaderMode();
  
  /**
   * Calculates and sets the value for each output. In program mode the program value is calculated first
   */
  void refreshOutputs();

private:
  
  /**
   * The values for the current moment in the program
   */
  void calculateProgramStep();
  
  // static members
  static const uint8_t s_numOfOutputs = 7;
  
  // members
  bool m_programMode;
  uint8_t m_output[s_numOfOutputs];
  uint32_t m_lastStepMs;
  uint8_t m_lastStep;
  Program m_program;
};


#endif // SEQUENCER_H
 
