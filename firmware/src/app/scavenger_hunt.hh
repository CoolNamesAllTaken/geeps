#pragma once

#include "stdint.h"

class ScavengerHuntHint
{
public:
    static const uint16_t kHintTextMaxLen = 300;

    enum HintType : uint8_t
    {
        kHintTypeText = 0,
        kHintTypeDirection,
        kHintTypeDistance
    };

    struct Location
    {
        float lat_deg;
        float lon_deg;
    };

    HintType type;
    Location location;

    char hint_text[kHintTextMaxLen] = '\0';

private:
};