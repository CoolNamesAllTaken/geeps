#include "scavenger_hunt.hh"

#include "sd_utils.hh"

bool ScavengerHunt::Init() {
    if (pSD->use_card_detect) {
        // Set up an interrupt on Card Detect to detect removal of the card
        // when it happens:
        // gpio_set_irq_enabled_with_callback(pSD->card_detect_gpio, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true,
        //                                    &card_detect_callback);
    }

    int ret = sd_init_card(pSD);
    if (ret != 0) {
        char fail_reason[100] = {'\0'};
        sprintf(fail_reason, "Failed to initialize SD card: ");
        if (ret & STA_NOINIT) {
            strcat(fail_reason, "STA_NOINIT ");
        }
        if (ret & STA_NODISK) {
            strcat(fail_reason, "STA_NODISK ");
        }
        if (ret & STA_PROTECT) {
            strcat(fail_reason, "STA_PROTECT ");
        }

        printf("ScavengerHunt::Init: %s\r\n", fail_reason);
        strcpy(status_text, fail_reason);
        return false;
    } else {
        printf("ScavengerHunt::Init: Initialized SD card.\r\n");
    }

    snprintf(status_text, Hint::kHintTextMaxLen, "SD card initialized.");

    return true;
}

bool ScavengerHunt::Update(float lat_deg, float lon_deg, uint32_t timestamp_utc) { return true; }