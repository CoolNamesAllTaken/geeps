#include "scavenger_hunt.hh"

#include "gps_utils.hh"
// #include "geeps_gui.hh"
#include "sd_utils.hh"

// extern GUITextBox hint_box;

static constexpr char kHintsFilename[] = "hints.txt";
static constexpr char kBeginHintsSectionDelimiter[] = "begin_hints";
static constexpr char kEndHintsSectionDelimiter[] = "end_hints";

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
    printf("Files in root SD card directory:\n");
    while (true) {
        fr = f_readdir(&dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0) break;
        if (!(fno.fattrib & AM_DIR)) {
            printf("  %s - %lu bytes\n", fno.fname, fno.fsize);
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

bool ScavengerHunt::Update(float lat_deg, float lon_deg, uint32_t timestamp_utc) {
    for (uint16_t i = 0; i < num_hints; i++) {
        if (hints[i].completed_timestamp_utc != -1) {
            // Skip hints that have already been completed.
            continue;
        }
        // Check if the user is within 10 meters of the hint location.
        float distance_m = CalculateGeoidalDistance(lat_deg, lon_deg, hints[i].lat_deg, hints[i].lon_deg);
        if (distance_m < 10.0f) {
            hints[i].completed_timestamp_utc = timestamp_utc;
            LogMessage("Hint %d completed at %d.\n", i, timestamp_utc);
            rendered_hint_index = i + 1;  // Jump to rendering the next hint when it's found.
            continue;
        }
        active_hint_index = i;
        break;  // Don't bother checking hints after the currently active one.
    }
    return true;
}

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

/**
 * Find the next token in a string. Mimicks strtok but accepts delimiters that are not separated.
 * @param token_start Pointer to the start of the token.
 * @param delim Pointer to the delimiter.
 * @return Pointer to the next token, or NULL if no more delimiters are found.
 */
char *find_next_token(char *token_start, const char *delim) {
    static char *line = (char *)"\0";
    token_start = token_start == NULL ? line : token_start;
    if (strlen(token_start) == 0) {
        // No more characters. Return NULL.
        return NULL;
    }
    char *token_end = strchr(token_start, *delim);
    if (token_end == NULL) {
        // No more delimiters, return the rest of the string as a token.
        line = token_start + strlen(token_start);
    } else {
        // Found a delimiter. Null terminate the token.
        *token_end = '\0';
        line = token_end + 1;
    }

    return token_start;
}

bool ScavengerHunt::LoadHints() {
    FRESULT fr;

    ListFiles();

    // Open the hints.txt file.
    FIL hints_txt_file;
    fr = f_open(&hints_txt_file, kHintsFilename, FA_OPEN_EXISTING | FA_READ);
    if (FR_OK != fr && FR_EXIST != fr) {
        LogMessage("f_open(%s) error: %s (%d)\n", kHintsFilename, FRESULT_str(fr), fr);
        return false;
    }
    uint32_t hints_txt_size_bytes = f_size(&hints_txt_file);
    printf("File size: %lu bytes\n", hints_txt_size_bytes);

    // Step through hints.txt line by line.
    uint16_t bytes_read = 0;
    uint16_t lines_read = 0;
    bool in_hints_section = false;
    while (bytes_read < hints_txt_size_bytes) {
        // Read a line from the file.
        char line[kHintsFileLineMaxLen] = {'\0'};
        char *buf = f_gets(line, kHintsFileLineMaxLen, &hints_txt_file);
        lines_read++;
        bytes_read += strlen(line);  // Do this before strtok, since strtok modifies the string.
        if (buf == 0) {
            LogMessage("f_gets error.\n");
            return false;
        }
        printf("Read line: %s", line);
        // Parse the line.
        // Ignore commented out lines.
        if (line[0] == '#') {
            continue;
        }
        // Look for special fields.
        if (char *title_start = strstr(line, "title=\"")) {
            title_start += 7;  // Skip 'title="'
            char *title_end = strchr(title_start, '"');
            if (title_end) {
                *title_end = '\0';  // Null terminate the title
                printf("Found title: %s\n", title_start);
                strcpy(title, title_start);
            }
        } else if (char *splash_image_filename_start = strstr(line, "splash_image=\"")) {
            splash_image_filename_start += 14;  // Skip 'splash_image="'
            char *splash_image_filename_end = strchr(splash_image_filename_start, '"');
            if (splash_image_filename_end) {
                *splash_image_filename_end = '\0';  // Null terminate the filename
                printf("Found splash image filename: %s\n", splash_image_filename_start);
                strcpy(splash_image_filename, splash_image_filename_start);
            }
        } else if (strncasecmp(line, kBeginHintsSectionDelimiter, strlen(kBeginHintsSectionDelimiter)) == 0) {
            in_hints_section = true;
            continue;
        } else if (strncasecmp(line, kEndHintsSectionDelimiter, strlen(kEndHintsSectionDelimiter)) == 0) {
            in_hints_section = false;
            continue;
        }

        if (!in_hints_section) {
            // Skip any lines that don't match a special case, or aren't in the hints section or a hints section
            // delimiter.
            continue;
        }

        char *token;
        // Parse latitude.
        token = find_next_token(line, ",");
        if (!token || *token == '\0') {
            LogMessage("hints.txt line %d is missing latitude.\n", lines_read);
            return false;
        }
        hints[num_hints].lat_deg = atof(token);

        // Parse longitude.
        token = find_next_token(NULL, ",");
        if (!token || *token == '\0') {
            LogMessage("hints.txt line %d is missing longitude.\n", lines_read);
            return false;
        }
        hints[num_hints].lon_deg = atof(token);

        // Parse hint_type.
        token = find_next_token(NULL, ",");
        if (!token || *token == '\0') {
            LogMessage("hints.txt line %d is missing hint type.\n", lines_read);
            return false;
        }
        // Strip leading and trailing whitespace
        while (*token == ' ' || *token == '\t') token++;
        size_t len = strlen(token);
        while (len > 0 && (token[len - 1] == ' ' || token[len - 1] == '\t')) {
            token[--len] = '\0';
        }
        // Convert token text to enum.
        if (strcasecmp(token, "text") == 0) {
            hints[num_hints].hint_type = Hint::kHintTypeText;
        } else if (strcasecmp(token, "image") == 0) {
            hints[num_hints].hint_type = Hint::kHintTypeImage;
        } else if (strcasecmp(token, "distance") == 0) {
            hints[num_hints].hint_type = Hint::kHintTypeDistance;
        } else if (strcasecmp(token, "heading") == 0) {
            hints[num_hints].hint_type = Hint::kHintTypeHeading;
        } else {
            LogMessage("hints.txt line %d has unknown hint type: %s.\n", lines_read, token);
            return false;
        }
        // Parse hint_text.
        token = find_next_token(NULL, "\"");  // Skip leading characters before first quote.
        token = find_next_token(NULL, "\"");  // Get characters between quotes.
        if (token != NULL) {
            strncpy(hints[num_hints].hint_text, token, Hint::kHintTextMaxLen - 1);
            hints[num_hints].hint_text[Hint::kHintTextMaxLen - 1] = '\0';
        } else {
            LogMessage("hints.txt line %d is missing hint text.\n", lines_read);
            return false;
        }
        token = find_next_token(NULL, ",");  // Skip to the next CSV field.
        // Parse hint_image_filename.
        if (hints[num_hints].hint_type == Hint::kHintTypeImage) {
            token = find_next_token(NULL, "\"");  // Skip leading characters before first quote.
            token = find_next_token(NULL, "\"");  // Get characters between quotes.
            if (token != NULL) {
                strncpy(hints[num_hints].hint_image_filename, token, Hint::kImageFilenameMaxLen - 1);
                hints[num_hints].hint_text[Hint::kImageFilenameMaxLen - 1] = '\0';
            } else {
                LogMessage("hints.txt line %d is missing an image filename.\n", lines_read);
                return false;
            }
            token = find_next_token(NULL, ",");  // Skip to the next CSV field.
                                                 // TODO: Load image dimensions.
        } else {
            token = find_next_token(NULL, ",");  // Ignore filename if hint is not an image.
        }
        // Parse completed_timestamp_utc.
        token = find_next_token(NULL, "#");  // Line ends with a comment, or blank.

        hints[num_hints].completed_timestamp_utc = atoi(token);

        char hint_str[500];
        hints[num_hints].ToString(hint_str, sizeof(hint_str));
        printf("\tParsed hint: %s\n", hint_str);
        num_hints++;
    }

    if (bytes_read != hints_txt_size_bytes) {
        LogMessage("f_read error: read %lu bytes, expected %lu bytes\n", bytes_read, hints_txt_size_bytes);
        return false;
    }

    fr = f_close(&hints_txt_file);
    if (FR_OK != fr) {
        LogMessage("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
        return false;
    }

    LogMessage("Loaded %d hints from SD card.\n", num_hints);
    return true;
}

void ScavengerHunt::LogMessage(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(status_text, Hint::kHintTextMaxLen, fmt, args);
    printf(status_text);
    va_end(args);
}