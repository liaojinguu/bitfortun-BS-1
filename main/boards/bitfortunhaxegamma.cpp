#include "board.h"
#include "bitfortunhaxegamma.h"
#include "bitfortunqaxeplus2.h"

static const char* TAG="bitfortunhaxegamma";

BitfortunHaxeGamma::BitfortunHaxeGamma() : BitfortunQaxePlus2() {
    m_deviceModel = "BitfortunHaxe-γ";
    m_miningAgent = m_deviceModel;
    m_asicModel = "BM1373";
    m_asicCount = 6;
    m_numPhases = 4;
    m_imax = 120;
    m_ifault = 105.0;

    // use m_asicVoltage for init
    m_initVoltageMillis = 0;

    m_maxPin = 250.0;
    m_minPin = 75.0;
    m_maxVin = 13.0;
    m_minVin = 11.0;
    m_minCurrentA = 0.0f;
    m_maxCurrentA = 15.0f;

    m_asicMaxDifficulty = 4096;
    m_asicMinDifficulty = 1024;
    m_asicMinDifficultyDualPool = 256;

#ifdef BITFORTUNHAXEGAMMA
    m_theme = new ThemeBitfortunhaxegamma();
#endif

    m_swarmColorName = "#00e7e2";  // cyan

}
