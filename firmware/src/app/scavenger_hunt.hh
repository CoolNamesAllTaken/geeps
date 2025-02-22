#pragma once

#include "bsp.hh"
#include "geeps_gui.hh"
#include "stdint.h"

class Hint {
   public:
    static const uint16_t kHintTextMaxLen = 300;
    static const uint16_t kImageFilenameMaxLen = 100;

    enum HintType : uint8_t { kHintTypeText = 0, kHintTypeImage, kHintTypeDistance, kHintTypeDirection };

    float lat_deg, lon_deg;
    HintType type;

    char hint_text[kHintTextMaxLen] = {'\0'};
    uint8_t hint_image_filename[kImageFilenameMaxLen] = {'\0'};
    uint16_t hint_image_width;
    uint16_t hint_image_height;

    int32_t time_completed_utc = -1;  // UTC timestamp when hint was completed.

   private:
};

class ScavengerHunt {
   public:
    static const uint16_t kMaxHints = 100;

    ScavengerHunt() {};

    bool Init();
    bool Update(float lat_deg, float lon_deg, uint32_t timestamp_utc);

    Hint hints[kMaxHints];
    uint16_t num_hints;

    char status_text[Hint::kHintTextMaxLen] = {'\0'};
};

extern ScavengerHunt scavenger_hunt;