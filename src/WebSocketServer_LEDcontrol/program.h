#ifndef PROGRAM_H
#define PROGRAM_H

struct Program {
  static const uint8_t MAX_NUM_OF_STEPS = 100;
  static const uint8_t NUM_OF_CHANNELS = 7;
  uint8_t m_numOfSteps;
  uint8_t m_masterSetting;
  uint8_t m_CrossfadeSetting;
  uint16_t m_stepTime;
  uint8_t m_prog[NUM_OF_CHANNELS][MAX_NUM_OF_STEPS];
};

#endif // PROGRAM_H
