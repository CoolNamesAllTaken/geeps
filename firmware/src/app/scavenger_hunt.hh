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
    uint16_t hint_image_width;
    uint16_t hint_image_height;

    int32_t completed_timestamp_utc = -1;  // UTC timestamp when hint was completed.

   private:
};

class ScavengerHunt {
   public:
    static const uint16_t kMaxHints = 100;
    static const uint16_t kHintsFileLineMaxLen = 300;
    static const uint16_t kTitleMaxLen = 100;

    ScavengerHunt() {};

    bool Init();
    bool Update(float lat_deg, float lon_deg, uint32_t timestamp_utc);

    bool MountSDCard();
    bool UnmountSDCard();

    bool LoadHints();
    bool SaveHints();

    Hint hints[kMaxHints];
    uint16_t num_hints;

    char status_text[Hint::kHintTextMaxLen] = {'\0'};

    // Labels read from hints.txt
    char title[kTitleMaxLen] = {'\0'};
    char splash_image_filename[Hint::kImageFilenameMaxLen] = {'\0'};

   private:
    void LogMessage(const char* fmt, ...);
};

extern ScavengerHunt scavenger_hunt;