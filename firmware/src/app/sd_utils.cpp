#include "sd_utils.hh"

#include "scavenger_hunt.hh"

extern ScavengerHunt scavenger_hunt;

spi_t sd_card_spi = {.hw_inst = BSP::sd_card_spi_inst,
                     .miso_gpio = BSP::sd_card_miso_pin,
                     .mosi_gpio = BSP::sd_card_mosi_pin,
                     .sck_gpio = BSP::sd_card_clk_pin,
                     .baud_rate = BSP::sd_card_spi_clk_rate_hz,
                     .dma_isr = sd_card_spi_dma_isr,
                     .initialized = false};
spi_t *pSPI = &sd_card_spi;

sd_card_t sd_card = {
    .pcName = "0:",
    .spi = &sd_card_spi,
    .ss_gpio = BSP::sd_card_cs_pin,
    .card_detect_gpio = BSP::sd_card_detect_pin,
    .card_detected_true = 1,
    .m_Status = STA_NOINIT,
    .sectors = 0,
    .card_type = 0,
};
sd_card_t *pSD = &sd_card;

void sd_card_spi_dma_isr() { spi_irq_handler(sd_card.spi); }

// If the card is physically removed, unmount the filesystem:
void card_detect_callback(uint gpio, uint32_t events) {
    static bool busy;
    if (busy) return;  // Avoid switch bounce
    busy = true;
    if (pSD->card_detect_gpio == gpio) {
        if (pSD->mounted && events & GPIO_IRQ_EDGE_RISE) {
            printf("(Card Detect Interrupt: unmounting %s)\n", pSD->pcName);
            if (scavenger_hunt.UnmountSDCard()) {
                pSD->mounted = false;
            }
        } else if (!pSD->mounted && events & GPIO_IRQ_EDGE_FALL) {
            printf("(Card Detect Interrupt: mounting %s)\n", pSD->pcName);
            if (scavenger_hunt.MountSDCard()) {
                pSD->mounted = true;
            }
        }
        sd_card_detect(pSD);
    }
    busy = false;
}

// hw_config.h functions with C linkage.
#ifdef __cplusplus
extern "C" {
#endif

size_t sd_get_num() { return kNumSDCards; }

sd_card_t *sd_get_by_num(size_t num) {
    if (num == 0) {
        return pSD;
    } else {
        return NULL;
    }
}

size_t spi_get_num() { return kNumSPIs; }
spi_t *spi_get_by_num(size_t num) {
    if (num == 0) {
        return pSPI;
    } else {
        return NULL;
    }
}

#ifdef __cplusplus
}
#endif
