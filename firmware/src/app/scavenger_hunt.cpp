#include "scavenger_hunt.hh"

#include "gps_utils.hh"
// #include "geeps_gui.hh"
#include "sd_utils.hh"

// This is the name given to a hints file used to configure a scavenger hunt.
static constexpr char kHintsFilename[] = "hints.txt";
// This is the name given to a copy of the hints file that is being actively used by a scavenger hunt.
static constexpr char kHintsInUseFilename[] = "~hints.txt";
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

bool ScavengerHunt::TryHint(uint16_t hint_index) {
    if (hint_index > num_hints) {
        return false;
    }
    if (hints[hint_index].completed_timestamp_utc != -1) {
        LogMessage("Hint %d has already been completed.", hint_index);
        return false;
    }
    if (GetDistanceToHint(hint_index) < kHintCompleteRadiusM) {
        hints[hint_index].completed_timestamp_utc = last_update_timestamp_utc;
        LogMessage("Hint %d completed at %lu. Yay!", hint_index, hints[hint_index].completed_timestamp_utc);
        return true;
    }
    LogMessage("Not within %.0fm of hint %d.\n", kHintCompleteRadiusM, hint_index);
    return false;
}

float ScavengerHunt::GetDistanceToHint(Hint &hint) {
    return CalculateGeoidalDistance(last_update_lat_deg, last_update_lon_deg, hint.lat_deg, hint.lon_deg);
}

float ScavengerHunt::GetHeadingToHint(Hint &hint) {
    return CalculateHeadingToWaypoint(last_update_lat_deg, last_update_lon_deg, hint.lat_deg, hint.lon_deg);
}

void ScavengerHunt::Render() {
    Hint &hint = hints[rendered_hint_index];
    // Default settings.
    hint_box.width_chars = 25;
    compass.visible = false;
    strncpy(status_bar.center_button_label, "TRY", GUIStatusBar::kCenterButtonLabelLen);

    switch (hint.hint_type) {
        case Hint::kHintTypeText: {
            strncpy(hint_box.text, hint.hint_text, GUITextBox::kTextMaxLen);
            break;
        }
        case Hint::kHintTypeImage: {
            strncpy(hint_box.text, hint.hint_image_filename, GUITextBox::kTextMaxLen);
            break;
        }
        case Hint::kHintTypeDistance: {
            float distance_to_hint_m = GetDistanceToHint(hint);
            snprintf(hint_box.text, GUITextBox::kTextMaxLen, "Distance: %.2f m\n\n%s", distance_to_hint_m,
                     hint.hint_text);
            break;
        }
        case Hint::kHintTypeHeading: {
            float heading_to_hint_deg = GetHeadingToHint(hint);
            snprintf(hint_box.text, GUITextBox::kTextMaxLen, "Heading: %.2f deg\n\n%s", heading_to_hint_deg,
                     hint.hint_text);
            hint_box.width_chars = 10;
            compass.visible = true;
            compass.heading_deg = heading_to_hint_deg;
            break;
        }
        default: {
            strncpy(hint_box.text, "Invalid hint type.", GUITextBox::kTextMaxLen);
            break;
        }
    }
}

bool ScavengerHunt::Update(float lat_deg, float lon_deg, uint32_t timestamp_utc) {
    last_update_lat_deg = lat_deg;
    last_update_lon_deg = lon_deg;
    last_update_timestamp_utc = timestamp_utc;

    for (uint16_t i = 0; i < num_hints; i++) {
        if (hints[i].completed_timestamp_utc != -1) {
            // Skip hints that have already been completed.
            continue;
        }
        active_hint_index = i;
    }

    // Update the current hint dot on the display.
    status_bar.rendered_hint_frac = (float)(rendered_hint_index + 1) / num_hints;
    status_bar.progress_frac = (float)(active_hint_index + 1) / num_hints;
    return true;
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
    // ListFiles();

    FRESULT fr;
    FIL hints_txt_file;

    bool hints_in_use = f_stat(kHintsInUseFilename, NULL) == FR_OK;
    if (hints_in_use) {
        LogMessage("Scavenger hunt in progress: using hints in %s.", kHintsInUseFilename);
    } else {
        LogMessage("No active scavenger hunt: using hints in %s.", kHintsFilename);
    }

    // Open the hints.txt file.
    fr = f_open(&hints_txt_file, hints_in_use ? kHintsInUseFilename : kHintsFilename, FA_OPEN_EXISTING | FA_READ);
    if (FR_OK != fr && FR_EXIST != fr) {
        LogMessage("f_open(%s) error: %s (%d)", hints_in_use ? kHintsInUseFilename : kHintsFilename, FRESULT_str(fr),
                   fr);
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
            LogMessage("f_gets error.");
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
                if (f_stat(splash_image_filename_start, NULL) == FR_OK) {
                    printf("Found splash image filename: %s\n", splash_image_filename_start);
                    strcpy(splash_image_filename, splash_image_filename_start);
                } else {
                    LogMessage("Splash image file not found: %s.", splash_image_filename_start);
                    return false;
                }
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
            LogMessage("hints.txt line %d is missing latitude.", lines_read);
            return false;
        }
        hints[num_hints].lat_deg = atof(token);

        // Parse longitude.
        token = find_next_token(NULL, ",");
        if (!token || *token == '\0') {
            LogMessage("hints.txt line %d is missing longitude.", lines_read);
            return false;
        }
        hints[num_hints].lon_deg = atof(token);

        // Parse hint_type.
        token = find_next_token(NULL, ",");
        if (!token || *token == '\0') {
            LogMessage("hints.txt line %d is missing hint type.", lines_read);
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
            LogMessage("hints.txt line %d has unknown hint type: %s.", lines_read, token);
            return false;
        }
        // Parse hint_text.
        token = find_next_token(NULL, "\"");  // Skip leading characters before first quote.
        token = find_next_token(NULL, "\"");  // Get characters between quotes.
        if (token != NULL) {
            strncpy(hints[num_hints].hint_text, token, Hint::kHintTextMaxLen - 1);
            hints[num_hints].hint_text[Hint::kHintTextMaxLen - 1] = '\0';
        } else {
            LogMessage("hints.txt line %d is missing hint text.", lines_read);
            return false;
        }
        token = find_next_token(NULL, ",");  // Skip to the next CSV field.
        // Parse hint_image_filename.
        if (hints[num_hints].hint_type == Hint::kHintTypeImage) {
            token = find_next_token(NULL, "\"");  // Skip leading characters before first quote.
            token = find_next_token(NULL, "\"");  // Get characters between quotes.
            if (token != NULL) {
                strncpy(hints[num_hints].hint_image_filename, token, Hint::kImageFilenameMaxLen - 1);
                hints[num_hints].hint_image_filename[Hint::kImageFilenameMaxLen - 1] = '\0';
                if (f_stat(hints[num_hints].hint_image_filename, NULL) != FR_OK) {
                    LogMessage("hints.txt line %d image file not found: %s.", lines_read,
                               hints[num_hints].hint_image_filename);
                    return false;
                }
            } else {
                LogMessage("hints.txt line %d is missing an image filename.", lines_read);
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
        LogMessage("f_read error: read %lu bytes, expected %lu bytes", bytes_read, hints_txt_size_bytes);
        return false;
    }

    fr = f_close(&hints_txt_file);
    if (FR_OK != fr) {
        LogMessage("f_close error: %s (%d)", FRESULT_str(fr), fr);
        return false;
    }

    LogMessage("Loaded %d hints from SD card.", num_hints);

    if (!hints_in_use) {
        SaveHints();  // Save the hints to the ~hints.txt file.
    }

    return true;
}

bool ScavengerHunt::SaveHints() {
    // Create a temporary hints file
    FRESULT fr;
    FIL hints_txt_file;
    fr = f_open(&hints_txt_file, kHintsInUseFilename, FA_CREATE_ALWAYS | FA_WRITE);
    if (FR_OK != fr) {
        LogMessage("Failed to create %s: %s (%d)", kHintsInUseFilename, FRESULT_str(fr), fr);
        return false;
    }

    // Write title if it exists
    if (strlen(title) > 0) {
        f_printf(&hints_txt_file, "title=\"%s\"\n", title);
    }

    // Write splash image if it exists
    if (strlen(splash_image_filename) > 0) {
        f_printf(&hints_txt_file, "splash_image=\"%s\"\n", splash_image_filename);
    }

    // Write hints section
    f_printf(&hints_txt_file, "%s\n", kBeginHintsSectionDelimiter);

    for (uint16_t i = 0; i < num_hints; i++) {
        const char *hint_type_str;
        switch (hints[i].hint_type) {
            case Hint::kHintTypeText:
                hint_type_str = "text";
                break;
            case Hint::kHintTypeImage:
                hint_type_str = "image";
                break;
            case Hint::kHintTypeDistance:
                hint_type_str = "distance";
                break;
            case Hint::kHintTypeHeading:
                hint_type_str = "heading";
                break;
            default:
                continue;  // Skip invalid hint types
        }
        printf("%.6f,%.6f,%s,\"%s\"", hints[i].lat_deg, hints[i].lon_deg, hint_type_str, hints[i].hint_text);
        f_printf(&hints_txt_file, "%.6f,%.6f,%s,\"%s\"", hints[i].lat_deg, hints[i].lon_deg, hint_type_str,
                 hints[i].hint_text);

        if (hints[i].hint_type == Hint::kHintTypeImage) {
            f_printf(&hints_txt_file, ",\"%s\"", hints[i].hint_image_filename);
        } else {
            f_printf(&hints_txt_file, ",");
        }

        f_printf(&hints_txt_file, ",%d\n", hints[i].completed_timestamp_utc);
    }

    f_printf(&hints_txt_file, "%s\n", kEndHintsSectionDelimiter);

    // Close the file
    fr = f_close(&hints_txt_file);
    if (FR_OK != fr) {
        LogMessage("Failed to close %s: %s (%d)", kHintsInUseFilename, FRESULT_str(fr), fr);
        return false;
    }

    return true;
}

bool ScavengerHunt::Reset() {
    // Find a ~hints.txt file and delete it if it exists.
    FRESULT fr;
    FIL hints_txt_file;
    fr = f_stat(kHintsInUseFilename, NULL);
    if (fr == FR_OK) {
        // File exists, try to delete it
        fr = f_unlink(kHintsInUseFilename);
        if (fr != FR_OK) {
            LogMessage("Failed to delete %s: %s (%d)", kHintsInUseFilename, FRESULT_str(fr), fr);
            return false;
        }
        LogMessage("Deleted %s.", kHintsInUseFilename);
    }

    return true;
}

void ScavengerHunt::LogMessage(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(notification_text, Hint::kHintTextMaxLen, fmt, args);
    notification.DisplayNotification(notification_text, 2000);
    printf("%s\n", notification_text);
    va_end(args);
}