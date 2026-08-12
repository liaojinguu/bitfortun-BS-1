#pragma once

#include "asic.h"
#include "bm1373.h"
#include "board.h"
#include "bitfortunqaxeplus2.h"
#include "bitfortunoctaxegamma.h"
#include "./drivers/tmp451_mux.h"

class BitfortunQX : public BitfortunQaxePlus2 {
  protected:
    Tmp451Mux* m_tmp451 = nullptr;

    // flag to remember if we found the tmux
    bool m_hasTMux = false;

  public:
    BitfortunQX();
    virtual bool initBoard();
    virtual void requestChipTemps();
};