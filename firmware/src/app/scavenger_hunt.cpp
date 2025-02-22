#include "scavenger_hunt.hh"

#include "sd_utils.hh"

bool ScavengerHunt::Init() {
    if (pSD->use_card_detect) {
        // Set up an interrupt on Card Detect to detect removal of the card
        // when it happens:
        gpio_set_irq_enabled_with_callback(pSD->card_detect_gpio, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true,
                                           &card_detect_callback);
    }

    int ret = sd_init_card(pSD);
    if (ret != 0) {
        printf("ScavengerHunt::Init: Failed to initialize SD card: [");
        if (ret & STA_NOINIT) {
            printf("STA_NOINIT ");
        }
        if (ret & STA_NODISK) {
            printf("STA_NODISK ");
        }
        if (ret & STA_PROTECT) {
            printf("STA_PROTECT ");
        }
        printf("]\r\n");
        return false;
    } else {
        printf("ScavengerHunt::Init: Initialized SD card.\r\n");
    }

    return true;
}

bool ScavengerHunt::Update(float lat_deg, float lon_deg, uint32_t timestamp_utc) { return true; }