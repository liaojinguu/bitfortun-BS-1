#pragma once

#include "asic.h"
#include "driver/gpio.h"
#include "mining.h"
#include "rom/gpio.h"

/*class BM1373 : public Asic {
protected:
    virtual const uint8_t* getChipId();
    virtual uint32_t getDefaultVrFrequency();

    virtual uint8_t jobToAsicId(uint8_t job_id);
    virtual uint8_t asicToJobId(uint8_t asic_id);

    virtual uint8_t nonceToAsicNr(uint32_t nonce);
    virtual uint8_t chipIndexFromAddr(uint8_t addr);
    virtual uint8_t addrFromChipIndex(uint8_t idx);

public:
    BM1373();
    virtual const char* getName() { return "BM1373"; };
    virtual uint8_t init(uint64_t frequency, uint16_t asic_count, uint32_t difficulty, uint32_t vrFrequency);
    virtual uint16_t getSmallCoreCount();
};*/

class BM1373 : public Asic
{
public:
    BM1373();

    virtual const char* getName() override
    {
        return "BM1373";
    }

protected:
    virtual const uint8_t* getChipId() override;
    virtual uint32_t getDefaultVrFrequency() override;

    virtual uint8_t jobToAsicId(uint8_t job_id) override;
    virtual uint8_t asicToJobId(uint8_t asic_id) override;

    virtual uint8_t nonceToAsicNr(uint32_t nonce) override;
    virtual uint8_t chipIndexFromAddr(uint8_t addr) override;
    virtual uint8_t addrFromChipIndex(uint8_t idx) override;

public:
    virtual uint8_t init(
        uint64_t frequency,
        uint16_t asic_count,
        uint32_t difficulty,
        uint32_t vrFrequency) override;

    virtual uint16_t getSmallCoreCount() override;
};