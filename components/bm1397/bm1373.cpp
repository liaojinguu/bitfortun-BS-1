#include "bm1373.h"

#include <endian.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "asic.h"
#include "crc.h"
#include "serial.h"
#include "mining_utils.h"

#ifndef BM1373_SMALL_CORE_COUNT_CC
#define BM1373_SMALL_CORE_COUNT_CC    2040
#endif

#ifndef BM1373_SMALL_CORE_COUNT_AA
#define BM1373_SMALL_CORE_COUNT_AA    2040
#endif

static const char *TAG = "bm1373Module";


static const uint8_t CHIP_ID_CC[6] = {0xaa, 0x55, 0x13, 0x72, 0x00, 0x00};
static const uint8_t CHIP_ID_AA[6]  = {0xaa, 0x55, 0x13, 0x72, 0x00, 0x01};

BM1373::BM1373() : Asic() {
    memcpy(detectedChipId, CHIP_ID_CC, 6);
}


void BM1373::detectChipType() {

    ESP_LOGI(TAG, "BM1373 芯片检测 - 当前使用默认逻辑");
    ESP_LOGW(TAG, "注意：当前芯片ID检测为简化版，请确认硬件类型");
}

const uint8_t* BM1373::getChipId() {
    return detectedChipId;
}

uint32_t BM1373::getDefaultVrFrequency() {
    return vrRegToFreq(0x1eb5);
}

uint16_t BM1373::getSmallCoreCount() {
    return smallCoreCount;
}

uint8_t BM1373::init(uint64_t frequency, uint16_t asic_count, uint32_t difficulty, uint32_t vrFrequency)
{

    detectChipType();

    if (asic_count == 1) {
        m_addressInterval = 4;  
    } else {
        m_addressInterval = 4;
    }

    ESP_LOGI(TAG, "BM1373 初始化 | 类型: %s | Address Interval: %d", 
             isAA ? "AA" : "CC", m_addressInterval);


    for(int i = 0; i < 4; i++) {
        send6(CMD_WRITE_ALL, 0x00, 0xA4, 0x90, 0x00, 0xFF, 0xFF);
    }

    int chip_counter = count_asics();
    if (asic_count == 4) {
        chip_counter = 4;
    }

    ESP_LOGI(TAG, "%i chip(s) detected, expected %i", chip_counter, asic_count);

    // enable and set version rolling mask
    send6(CMD_WRITE_ALL, 0x00, 0xA4, 0x90, 0x00, 0xFF, 0xFF);

    send6(CMD_WRITE_ALL, 0x00, 0xA8, 0x00, 0x07, 0x00, 0x00);
    send6(CMD_WRITE_ALL, 0x00, 0x18, 0xFF, 0x00, 0xC1, 0x00);

    sendChainInactive();

    m_addressInterval = (chip_counter > 0) ? (256 / next_power_of_two(chip_counter)) : 4;

    for (uint8_t i = 0; i < chip_counter; i++) {
        setChipAddress(i * m_addressInterval);
    }

    // Core Register Control
    send6(CMD_WRITE_ALL, 0x00, 0x3C, 0x80, 0x00, 0x8B, 0x00);
    send6(CMD_WRITE_ALL, 0x00, 0x3C, 0x80, 0x00, 0x80, 0x0C);

    setJobDifficultyMask(difficulty);

    send6(CMD_WRITE_ALL, 0x00, 0x58, 0x00, 0x01, 0x11, 0x11);
    send6(CMD_WRITE_ALL, 0x00, 0x68, 0x5A, 0xA5, 0x5A, 0xA5);

    for (uint8_t i = 0; i < chip_counter; i++) {
        uint8_t addr = i * m_addressInterval;
        send6(CMD_WRITE_SINGLE, addr, 0xA8, 0x00, 0x07, 0x01, 0xF0);
        send6(CMD_WRITE_SINGLE, addr, 0x18, 0xFF, 0x00, 0xC1, 0x00);
        send6(CMD_WRITE_SINGLE, addr, 0x3C, 0x80, 0x00, 0x8B, 0x00);
        send6(CMD_WRITE_SINGLE, addr, 0x3C, 0x80, 0x00, 0x80, 0x0c);
        send6(CMD_WRITE_SINGLE, addr, 0x3C, 0x80, 0x00, 0x82, 0xAA);
    }

    send6(CMD_WRITE_ALL, 0x00, 0xB9, 0x00, 0x00, 0x44, 0x80);
    send6(CMD_WRITE_ALL, 0x00, 0x54, 0x00, 0x00, 0x00, 0x02);
    send6(CMD_WRITE_ALL, 0x00, 0xB9, 0x00, 0x00, 0x44, 0x80);
    send6(CMD_WRITE_ALL, 0x00, 0x3C, 0x80, 0x00, 0x8D, 0xEE);

    doFrequencyTransition(frequency);
    setVrFrequency(vrFrequency);

    send6(CMD_WRITE_ALL, 0x00, 0xA4, 0x90, 0x00, 0xFF, 0xFF);

    return chip_counter;
}

// =======================================
uint8_t BM1373::jobToAsicId(uint8_t job_id) {
    return (job_id * 24) & 0x7f;
}

uint8_t BM1373::asicToJobId(uint8_t asic_id) {
    return (asic_id & 0xf0) >> 1;
}

uint8_t BM1373::nonceToAsicNr(uint32_t nonce) {
    return (uint8_t) ((nonce & 0x0000fc00) >> 11);
}

uint8_t BM1373::chipIndexFromAddr(uint8_t addr) {
    return addr >> 2;
}

uint8_t BM1373::addrFromChipIndex(uint8_t idx) {
    return idx << 2;
}