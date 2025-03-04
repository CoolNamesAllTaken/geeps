#include "sd_utils.hh"

#include "hw_config.h"
#include "scavenger_hunt.hh"

extern ScavengerHunt scavenger_hunt;
std::vector<FatFsNs::SdCard> FatFsNs::FatFs::SdCards;

const char SDUtil::kErrNames[][kErrNameMaxLen] = {
    "FR_OK",                  /* (0) Succeeded */
    "FR_DISK_ERR",            /* (1) A hard error occurred in the low level disk I/O layer */
    "FR_INT_ERR",             /* (2) Assertion failed */
    "FR_NOT_READY",           /* (3) The physical drive cannot work */
    "FR_NO_FILE",             /* (4) Could not find the file */
    "FR_NO_PATH",             /* (5) Could not find the path */
    "FR_INVALID_NAME",        /* (6) The path name format is invalid */
    "FR_DENIED",              /* (7) Access denied due to prohibited access or directory full */
    "FR_EXIST",               /* (8) Access denied due to prohibited access */
    "FR_INVALID_OBJECT",      /* (9) The file/directory object is invalid */
    "FR_WRITE_PROTECTED",     /* (10) The physical drive is write protected */
    "FR_INVALID_DRIVE",       /* (11) The logical drive number is invalid */
    "FR_NOT_ENABLED",         /* (12) The volume has no work area */
    "FR_NO_FILESYSTEM",       /* (13) There is no valid FAT volume */
    "FR_MKFS_ABORTED",        /* (14) The f_mkfs() aborted due to any problem */
    "FR_TIMEOUT",             /* (15) Could not get a grant to access the volume within defined period */
    "FR_LOCKED",              /* (16) The operation is rejected according to the file sharing policy */
    "FR_NOT_ENOUGH_CORE",     /* (17) LFN working buffer could not be allocated */
    "FR_TOO_MANY_OPEN_FILES", /* (18) Number of open files > FF_FS_LOCK */
    "FR_INVALID_PARAMETER" /* (19) Given parameter is invalid */};

SDUtil::SDUtil() {
    static spi_t sd_card_spi = {.hw_inst = BSP::sd_card_spi_inst,
                                .miso_gpio = BSP::sd_card_miso_pin,
                                .mosi_gpio = BSP::sd_card_mosi_pin,
                                .sck_gpio = BSP::sd_card_clk_pin,
                                .baud_rate = BSP::sd_card_spi_clk_rate_hz,
                                .spi_mode = 0,
                                .use_static_dma_channels = false,
                                //  .dma_isr = sd_card_spi_dma_isr,
                                .initialized = false};
    static sd_spi_if_t spi_if = {
        .spi = &sd_card_spi,
        .ss_gpio = BSP::sd_card_cs_pin,
    };

    static sd_card_t sd_card = {.type = SD_IF_SPI,
                                .spi_if_p = &spi_if,  // Pointer to the SPI interface driving this card
                                // SD Card detect:
                                .use_card_detect = true,
                                .card_detect_gpio = BSP::sd_card_detect_pin,
                                .card_detected_true = 0,  // What the GPIO read returns when a card is present.
                                .card_detect_use_pull = false,
                                .card_detect_pull_hi = true};

    card_p = FatFsNs::FatFs::add_sd_card(&sd_card);
}

void SDUtil::Init() {
    // The H/W config must be set up before this is called:
    sd_init_driver();
}

// void sd_card_spi_dma_isr() { spi_irq_handler(sd_util.card_p.spi); }

// If the card is physically removed, unmount the filesystem:
// void card_detect_callback(uint gpio, uint32_t events) {
//     static bool busy;
//     if (busy) return;  // Avoid switch bounce
//     busy = true;
//     if (pSD->card_detect_gpio == gpio) {
//         if (pSD->mounted && events & GPIO_IRQ_EDGE_RISE) {
//             printf("(Card Detect Interrupt: unmounting %s)\n", pSD->pcName);
//             if (scavenger_hunt.UnmountSDCard()) {
//                 pSD->mounted = false;
//             }
//         } else if (!pSD->mounted && events & GPIO_IRQ_EDGE_FALL) {
//             printf("(Card Detect Interrupt: mounting %s)\n", pSD->pcName);
//             if (scavenger_hunt.MountSDCard()) {
//                 pSD->mounted = true;
//             }
//         }
//         sd_card_detect(pSD);
//     }
//     busy = false;
// }

// hw_config.h functions with C linkage.
#ifdef __cplusplus
extern "C" {
#endif

size_t sd_get_num() { return SDUtil::kNumSDCards; }

sd_card_t *sd_get_by_num(size_t num) {
    if (num == 0) {
        return sd_util.card_p->m_sd_card_p;
    } else {
        return NULL;
    }
}

// size_t spi_get_num() { return SDUtil::kNumSPIs; }
// spi_t *spi_get_by_num(size_t num) {
//     if (num == 0) {
//         return pSPI;
//     } else {
//         return NULL;
//     }
// }

#ifdef __cplusplus
}
#endif
