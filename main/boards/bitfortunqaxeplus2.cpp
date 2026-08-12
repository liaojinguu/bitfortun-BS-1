#include "board.h"
#include "bitfortunqaxeplus2.h"
#include <cmath>

static const char* TAG __attribute__((unused)) = "bitfortunqaxeplus2";

BitfortunQaxePlus2::BitfortunQaxePlus2() : BitfortunQaxePlus() { 
    m_deviceModel = "Bitfortun BS-1";
    m_miningAgent = m_deviceModel;
    m_asicModel = "BM1373";
    m_asicCount = 4;
    m_numPhases = 4;
    m_imax = static_cast<int>(std::round(0.85 / 48700.0 / 5.0e-3 * 35000.0));
    m_ifault = (float) 160.0f;


    m_maxPin = 180.0;
    m_minPin = 50.0;
    m_maxVin = 13.0;
    m_minVin = 11.0;
    m_minCurrentA = 0.0f;
    m_maxCurrentA = 15.0f;

    m_asicFrequencies = {250, 275, 300, 325, 350, 375, 400, 425, 475, 500, 550};
    m_asicVoltages = {980, 990, 1000, 1010, 1020, 1030, 1040, 1050, 1060, 1070, 1080};
    m_defaultAsicFrequency = m_asicFrequency = 425;
    m_defaultAsicVoltageMillis = m_asicVoltageMillis = 1010;
    m_absMaxAsicFrequency = 730;
    m_absMinAsicVoltageMillis = 900;
    m_absMaxAsicVoltageMillis = 1200;
    m_initVoltageMillis = 1050;

    m_pidSettings[0].targetTemp = 60;
    m_pidSettings[0].p = 600;  //   6.00
    m_pidSettings[0].i = 10;   //   0.10
    m_pidSettings[0].d = 1000; // 10.00

    m_pidSettings[1].targetTemp = 65; // target temp for vreg
    m_pidSettings[1].p = 600;         //   6.00
    m_pidSettings[1].i = 10;          //   0.10
    m_pidSettings[1].d = 1000;        // 10.00

    // ship with PID fan control by default (ch1 stays linked); with target 60°C
    // the fan holds max(ASIC, VReg) for good out-of-the-box efficiency
    m_fanMode[0] = 2; // PID

    m_asicMaxDifficulty = 4096;
    m_asicMinDifficulty = 1024;
    m_asicMinDifficultyDualPool = 512;

#ifdef BITFORTUNQAXEPLUS2
    m_theme = new ThemeBitfortunqaxeplus2();
#endif
    m_asics = new BM1373();
    m_hasHashCounter = true;
    m_vrFrequency = m_defaultVrFrequency = m_asics->getDefaultVrFrequency();
}

float BitfortunQaxePlus2::getTemperature(int index) {
    float temp = BitfortunQaxePlus::getTemperature(index);
    if (!temp) {
        return 0.0;
    }
    // we can't read the real chip temps but this should be about right
    return temp + 10.0f; // offset of 10°C
}

void BitfortunQaxePlus2::requestChipTemps() {
    // NOP
}