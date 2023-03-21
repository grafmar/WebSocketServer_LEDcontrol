#include "sequencer.h"
#include "WebSocketServer_LEDcontrol.h"



Sequencer::Sequencer() :
  m_programMode(false),
  m_lastStepMs(0),
  m_lastStep(0)
{
  for (uint8_t i = 0 ; i < s_numOfOutputs ; i++) {
    m_output[i] = 255;
  }
  m_program.m_masterSetting = 128;
  m_program.m_CrossfadeSetting = 50;
  m_program.m_stepTime = 1000;
}

Sequencer::~Sequencer() {
}

void Sequencer::setMaster(uint8_t value) {
  m_program.m_masterSetting = value;
}

void Sequencer::setCrossfade(uint8_t value) {
  m_program.m_CrossfadeSetting = value;
}

void Sequencer::setStepTime(uint16_t value) {
  if (value < 10) {
    value = 10;
  }
  m_program.m_stepTime = value;
}

uint8_t Sequencer::getMaster() {
  return m_program.m_masterSetting;
}
uint8_t Sequencer::getCrossfade() {
  return m_program.m_CrossfadeSetting;
}

uint16_t Sequencer::getStepTime() {
  return m_program.m_stepTime;
}

uint8_t Sequencer::getLampValue(uint8_t lamp) {
  if (lamp < s_numOfOutputs) {
    return m_output[lamp];
  }
}

void Sequencer::setLampValue(uint8_t lamp, uint8_t value) {
  if (lamp < s_numOfOutputs) {
    m_output[lamp] = value;
  }
}

void Sequencer::setProgram(Program &prog) {
    m_programMode = true;
    m_program.m_numOfSteps = prog.m_numOfSteps;
    m_program.m_masterSetting = prog.m_masterSetting;
    m_program.m_CrossfadeSetting = prog.m_CrossfadeSetting;
    m_program.m_stepTime = prog.m_stepTime;

    for (uint8_t i = 0 ; i < m_program.m_numOfSteps ; i++) {
      for (uint8_t ch = 0 ; ch < s_numOfOutputs ; ch++) {
        m_program.m_prog[ch][i] = prog.m_prog[ch][i];
      }
    }
}

void Sequencer::setFaderMode() {
  m_programMode = false;
}
  

void Sequencer::refreshOutputs() {
  if (m_programMode) {
    calculateProgramStep();
  }
  
  analogWrite(D1, map(m_output[0] * m_program.m_masterSetting, 0, 255*255, 0, PWMRANGE));
  analogWrite(D2, map(m_output[1] * m_program.m_masterSetting, 0, 255*255, 0, PWMRANGE));
  analogWrite(D3, map(m_output[2] * m_program.m_masterSetting, 0, 255*255, 0, PWMRANGE));
  analogWrite(D4, map(m_output[3] * m_program.m_masterSetting, 0, 255*255, 0, PWMRANGE));
  analogWrite(D5, map(m_output[4] * m_program.m_masterSetting, 0, 255*255, 0, PWMRANGE));
  analogWrite(D6, map(m_output[5] * m_program.m_masterSetting, 0, 255*255, 0, PWMRANGE));
  analogWrite(D7, map(m_output[6] * m_program.m_masterSetting, 0, 255*255, 0, PWMRANGE));
}

void Sequencer::calculateProgramStep() {
  uint32_t time;
  uint32_t crossFadeValueScaledTime;
  uint32_t crossFadeFactorA;
  uint32_t crossFadeFactorB;
  uint32_t bigValue;
  uint8_t nextStep;
  
  time = millis() - m_lastStepMs;
  m_lastStep += time / m_program.m_stepTime;
  m_lastStep %= m_program.m_numOfSteps;
  nextStep = (m_lastStep + 1) % m_program.m_numOfSteps;
  m_lastStepMs += time - (time % m_program.m_stepTime);
  time %= m_program.m_stepTime;
  
  crossFadeValueScaledTime = (255 * time / m_program.m_stepTime);
  if (crossFadeValueScaledTime > m_program.m_CrossfadeSetting) {
    crossFadeFactorB = 255 * (255 * time - m_program.m_CrossfadeSetting * m_program.m_stepTime) / (255 - m_program.m_CrossfadeSetting) / m_program.m_stepTime;
  }
  else {
    crossFadeFactorB = 0;
  }
  crossFadeFactorA = 255 - crossFadeFactorB;
  for (uint8_t i = 0 ; i < s_numOfOutputs ; i++) {
    bigValue = crossFadeFactorA * m_program.m_prog[i][m_lastStep] + crossFadeFactorB * m_program.m_prog[i][nextStep];
    m_output[i] = bigValue / 255;
  }
}
