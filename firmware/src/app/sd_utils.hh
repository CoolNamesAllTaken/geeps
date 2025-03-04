#pragma once

#include "FatFsSd.h"
#include "bsp.hh"
// #include "f_util.h"
// #include "ff.h"
// #include "hw_config.h"
// #include "sd_card.h"
#include "stdio.h"

#define STA_NOINIT  0x01  // Drive not initialized
#define STA_NODISK  0x02  // No medium in the drive
#define STA_PROTECT 0x04  // Write protected

class SDUtil {
   public:
    static const uint16_t kErrNameMaxLen = 30;
    static const char kErrNames[][kErrNameMaxLen];

    static const uint16_t kNumSDCards = 1;
    static const uint16_t kNumSPIs = 1;

    SDUtil();
    void Init();
    bool Mount() {
        FRESULT res = card_p->mount();
        sd_card_mounted = res == FR_OK;
        return sd_card_mounted;
    }

    inline const char *FRESULT_str(FRESULT fr) { return kErrNames[fr]; }

    void sd_card_spi_dma_isr();
    void card_detect_callback(uint gpio, uint32_t events);

    FatFsNs::SdCard *card_p;

    bool sd_card_mounted = false;
};

extern SDUtil sd_util;
// std::vector<FatFsNs::SdCard> FatFsNs::FatFs::SdCards;

// extern sd_card_t *pSD;
// extern spi_t *pSPI;
