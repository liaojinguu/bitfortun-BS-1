#pragma once

#include "asic.h"
#include "bm1373.h"
#include "board.h"
#include "bitfortunqaxeplus.h"

class BitfortunQaxePlus2 : public BitfortunQaxePlus{

  int m_absMinAsicVoltageMillis;
  uint8_t m_fanMode[2];
  public:
    BitfortunQaxePlus2();
    float getTemperature(int index);
    bool hasEthernet() override { return true; }
    virtual void requestChipTemps();
};
