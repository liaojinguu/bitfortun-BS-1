#pragma once

#include "asic.h"
#include "driver/gpio.h"
#include "mining.h"
#include "rom/gpio.h"

class BM1373 : public Asic
{
private:
    uint8_t detectedChipId[6] = {0xaa, 0x55, 0x13, 0x72, 0x00, 0x00}; // 默认CC
    bool isAA = false;                 // true = AA, false = CC
    uint16_t smallCoreCount = 7000;

public:
    BM1373();

    virtual const char* getName() override
    {
        return isAA ? "BM1373AA" : "BM1373CC";
    }

protected:
    virtual const uint8_t* getChipId() override;
    virtual uint32_t getDefaultVrFrequency() override;

    virtual uint8_t jobToAsicId(uint8_t job_id) override;
    virtual uint8_t asicToJobId(uint8_t asic_id) override;

    virtual uint8_t nonceToAsicNr(uint32_t nonce) override;
    virtual uint8_t chipIndexFromAddr(uint8_t addr) override;
    virtual uint8_t addrFromChipIndex(uint8_t idx) override;

    // 芯片类型检测（简化版）
    void detectChipType();

public:
    virtual uint8_t init(
        uint64_t frequency,
        uint16_t asic_count,
        uint32_t difficulty,
        uint32_t vrFrequency) override;

    virtual uint16_t getSmallCoreCount() override;
};