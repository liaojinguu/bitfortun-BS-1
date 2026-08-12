#pragma once

#include "asic.h"
#include "bm1373.h"
#include "board.h"
#include "bitfortunaxe.h"

class BitfortunaxeGamma : public BitfortunAxe {
  protected:
    int m_initVoltageMillis;

  public:
    BitfortunaxeGamma();

    virtual bool initBoard();
    virtual bool initAsics();

    virtual void shutdown();

    virtual bool setVoltage(float core_voltage);

    virtual float getTemperature(int index);
    virtual float getVRTemp();
    virtual bool isPIDAvailable() { return true; }

    virtual float getVin();
    virtual float getIin();
    virtual float getPin();
    virtual float getVout();
    virtual float getIout();
    virtual float getPout();
};
