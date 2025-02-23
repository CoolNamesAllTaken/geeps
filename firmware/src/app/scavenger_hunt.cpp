#include "scavenger_hunt.hh"

// #include "geeps_gui.hh"
#include "sd_utils.hh"

// extern GUITextBox hint_box;

static constexpr char kHintsFilename[] = "hints.txt";

bool ListFiles() {
    FILINFO fno;
    DIR dir;
    // Open root directory
    FRESULT fr = f_opendir(&dir, "");
    if (FR_OK != fr) {
        printf("f_opendir error: %s (%d)\n", FRESULT_str(fr), fr);
        return false;
    }

    // List all files
    while (true) {
        fr = f_readdir(&dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0) break;
        if (!(fno.fattrib & AM_DIR)) {
            printf("%s - %lu bytes\n", fno.fname, fno.fsize);
        }
    }

    f_closedir(&dir);
    return true;
}

bool ScavengerHunt::Init() {
    if (sd_init_driver() != 0) {
        return false;
    }
    return true;
}

bool ScavengerHunt::Update(float lat_deg, float lon_deg, uint32_t timestamp_utc) { return true; }

bool ScavengerHunt::MountSDCard() {
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

        LogMessage("%s\r\n", fail_reason);
        // Maybe add a mutex exit here.
        UnmountSDCard();
        return false;
    } else {
        LogMessage("Initialized SD card.\r\n");
    }
    LogMessage("SD card initialized.\r\n");

    // Mount the SD card.
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (FR_OK != fr) {
        LogMessage("f_mount error: %s (%d)\r\n", FRESULT_str(fr), fr);
        return false;
    }

    ListFiles();

    FIL fil;
    fr = f_open(&fil, kHintsFilename, FA_OPEN_EXISTING | FA_WRITE);
    if (FR_OK != fr && FR_EXIST != fr) {
        LogMessage("f_open(%s) error: %s (%d)\n", kHintsFilename, FRESULT_str(fr), fr);
        return false;
    }

    fr = f_close(&fil);
    if (FR_OK != fr) {
        printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    }
    return true;
}

bool ScavengerHunt::UnmountSDCard() {
    FRESULT fr = f_unmount(pSD->pcName);
    if (fr == FR_OK) {
        LogMessage("Unmounted SD card.\r\n");
        return true;
    } else {
        LogMessage("Failed to unmount SD card: %s (%d)\r\n", FRESULT_str(fr), fr);
        return false;
    }
}

void ScavengerHunt::LogMessage(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(status_text, Hint::kHintTextMaxLen, fmt, args);
    printf(status_text);
    va_end(args);
}