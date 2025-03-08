#pragma once

#include "bsp.hh"
#include "geeps_gui.hh"
#include "stdint.h"

class Hint {
   public:
    static const uint16_t kHintTextMaxLen = 300;
    static const uint16_t kImageFilenameMaxLen = 100;

    enum HintType : int8_t {
        kHintTypeInvalid = -1,
        kHintTypeText = 0,
        kHintTypeImage,
        kHintTypeDistance,
        kHintTypeHeading
    };
    static const char* HintTypeStr(HintType type) {
        switch (type) {
            case kHintTypeText:
                return "text";
            case kHintTypeImage:
                return "image";
            case kHintTypeDistance:
                return "distance";
            case kHintTypeHeading:
                return "heading";
            default:
                return "invalid";
        }
    }

    inline void ToString(char* buf, uint16_t buf_len) {
        snprintf(buf, buf_len, "%f, %f, %s, \"%s\", \"%s\", %d", lat_deg, lon_deg, HintTypeStr(hint_type), hint_text,
                 hint_image_filename, completed_timestamp_utc);
    }

    float lat_deg, lon_deg;
    HintType hint_type;

    char hint_text[kHintTextMaxLen] = {'\0'};
    char hint_image_filename[kImageFilenameMaxLen] = {'\0'};

    int32_t completed_timestamp_utc = -1;  // UTC timestamp when hint was completed.

   private:
};

class ScavengerHunt {
   public:
    static const uint16_t kMaxHints = 50;
    static const uint16_t kHintsFileLineMaxLen = 300;
    static const uint16_t kTitleMaxLen = 100;
    static constexpr float kHintCompleteRadiusM = 10.0f;

    static const uint32_t kInactivityTimeoutIntervalMs = 60'000 * 5;  // 5 minute inactivity timeout.

    ScavengerHunt(GUIStatusBar& status_bar_in, GUITextBox& hint_box_in, GUIBitMap& hint_image_in,
                  GUICompass& compass_in, GUINotification& notification_in)
        : status_bar(status_bar_in),
          hint_box(hint_box_in),
          hint_image(hint_image_in),
          compass(compass_in),
          notification(notification_in) {};

    bool Init();
    bool Update(float lat_deg, float lon_deg, uint32_t timestamp_utc);

    inline void UpdateInactivityTimer() {
        inactivity_timeout_ms = to_ms_since_boot(get_absolute_time()) + kInactivityTimeoutIntervalMs;
    }
    /**
     * Power off the system via the POHO control pin. Only works while on battery power.
     */
    inline void PowerOff() {
        LogMessage("Powering off...");
        gpio_init(BSP::poho_ctrl_pin);
        gpio_set_dir(BSP::poho_ctrl_pin, GPIO_OUT);
        gpio_put(BSP::poho_ctrl_pin, 1);
    }

    bool LoadHints();
    bool SaveHints();

    bool TryHint(uint16_t hint_index);
    inline float GetDistanceToHint(uint16_t hint_index) {
        if (hint_index >= num_hints) {
            return -1.0f;
        }
        return GetDistanceToHint(hints[hint_index]);
    }
    float GetDistanceToHint(Hint& hint);
    float GetHeadingToHint(uint16_t hint_index) {
        if (hint_index >= num_hints) {
            return -1.0f;
        }
        return GetHeadingToHint(hints[hint_index]);
    }
    float GetHeadingToHint(Hint& hint);

    void Render();

    /**
     * Reset the scavenger hunt to the first hint.
     * @return True if successful.
     */
    bool Reset();

    inline void IncrementRenderedHint() {
        if (rendered_hint_index < num_hints - 1) {
            rendered_hint_index++;
        }
        if (hints[rendered_hint_index].completed_timestamp_utc != -1) {
            LogMessage("Hint %d has already been completed.", rendered_hint_index);
        }
    }
    inline void DecrementRenderedHint() {
        if (rendered_hint_index > 0) {
            rendered_hint_index--;
        }
        if (hints[rendered_hint_index].completed_timestamp_utc != -1) {
            LogMessage("Hint %d has already been completed.", rendered_hint_index);
        }
    }

    Hint* GetActiveHintPtr();
    Hint* GetRenderedHintPtr();
    void LogMessage(const char* fmt, ...);

    Hint hints[kMaxHints];
    uint16_t num_hints = 0;

    char status_text[Hint::kHintTextMaxLen] = {'\0'};
    char notification_text[GUINotification::kNotificationMaxNumChars] = {'\0'};

    // Labels read from hints.txt
    char title[kTitleMaxLen] = {'\0'};
    char splash_image_filename[Hint::kImageFilenameMaxLen] = {'\0'};

    uint16_t active_hint_index = 0;    // The hint we are looking for.
    uint16_t rendered_hint_index = 0;  // The hint we are displaying.

    float last_update_lat_deg = 0.0f;
    float last_update_lon_deg = 0.0f;
    uint32_t last_update_timestamp_utc = 0;

    bool skip_initialization = false;

    GUIStatusBar& status_bar;
    GUITextBox& hint_box;
    GUIBitMap& hint_image;
    GUICompass& compass;
    GUINotification& notification;

    uint32_t inactivity_timeout_ms;

   private:
};