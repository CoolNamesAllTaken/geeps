///
/// @file hV_Font_Terminal.h
/// @brief hV_Font_Terminal
///
/// @details Project Pervasive Displays Library Suite
/// @n Based on highView technology
///
/// @author Rei Vilo
/// @date 02 Aug 2021
/// @version 507
///
/// @copyright (c) Rei Vilo, 2010-2022
/// @copyright All rights reserved
///
/// * Basic edition: for hobbyists and for basic usage
/// @n Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0)
///
/// * Advanced edition: for professionals or organisations, no commercial usage
/// @n All rights reserved
///
/// * Commercial edition: for professionals or organisations, commercial usage
/// @n All rights reserved
///
/// @see Font generated with MikroElektronika GLCD Font Creator 1.2.0.0 and edited manually
/// MikroeElektronika 2011 http://www.mikroe.com
///

// SDK
#if defined(ENERGIA) // LaunchPad specific
#include "Energia.h"
#else // Arduino general
// #include "Arduino.h"
#endif // end IDE

// Configuration
#include "hV_Configuration.h"

// The Arduino IDE doesn't allow to select the libraries, hence this condition.
#if (FONT_MODE == USE_FONT_TERMINAL)

#ifndef hV_FONT_TERMINAL_RELEASE
///
/// @brief Release
///
#define hV_FONT_TERMINAL_RELEASE 507

#include "hV_Utilities.h"
#include "hV_Font.h"

///
/// @brief Biggest font size
/// @details Based on the MCU, by default=0
///
#if (MAX_FONT_SIZE > 0)
#include "Terminal6x8e.h"
#if (MAX_FONT_SIZE > 1)
#include "Terminal8x12e.h"
#if (MAX_FONT_SIZE > 2)
#include "Terminal12x16e.h"
#if (MAX_FONT_SIZE > 3)
#include "Terminal16x24e.h"
#endif
#endif
#endif
#endif

///
/// @brief Font enumeration
/// @note Generated by hV_FontsFlash_Manage2
///
enum fontNumber_e
{
    Font_Terminal6x8,
    Font_Terminal8x12,
    Font_Terminal12x16,
    Font_Terminal16x24,
};

///
/// @brief Class for font as header file
///
class hV_Font_Terminal
{
  protected:
    ///
    /// @brief Initialisation
    ///
    void _f_begin();

    ///
    /// @brief Select font
    /// @param font default = 0 = small, 1 = large
    /// @warning Definition for this method is compulsory.
    /// @note Previously _setFontSize()
    ///
    void _f_selectFont(uint8_t font);

    ///
    /// @brief Use a font
    /// @param fontName name of the font
    /// @return number of the font, 0 otherwise
    /// @warning Definition for this method is compulsory.
    /// @note Previously setFontSize()
    /// @n @b More: @ref Fonts
    ///
    uint8_t _f_addFont(font_s fontName);

    ///
    /// @brief Set transparent or opaque text
    /// @param flag default = 1 = opaque = solid, false = transparent
    /// @warning Definition for this method is compulsory.
    ///
    void _f_setFontSolid(bool flag = true);

    ///
    /// @brief Set additional pixels between two characters, horizontal axis
    /// @param number of spaces default = 1 pixel
    /// @warning Definition for this method is compulsory.
    ///
    void _f_setFontSpaceX(uint8_t number = 1);

    ///
    /// @brief Set additional pixels between two characters, vertical axis
    /// @param number of spaces default = 1 pixel
    /// @warning Definition for this method is compulsory.
    ///
    void _f_setFontSpaceY(uint8_t number = 1);

    ///
    /// @brief Character size, x-axis
    /// @param character character to evaluate, default=0=font general size
    /// @return horizontal size of character for current font, in pixels
    /// @note With setSpaceX included
    /// @note In variable font, each character has a specific size. The general size is indicative.
    /// @note Previously fontSizeX()
    /// @n @b More: @ref Fonts
    ///
    uint16_t _f_characterSizeX(uint8_t character = 0x00);

    ///
    /// @brief Character size, y-axis
    /// @return vertical size for current font, in pixels
    /// @note Previously fontSizeY()
    /// @n @b More: @ref Fonts
    ///
    uint16_t _f_characterSizeY();

    ///
    /// @brief String size, x-axis
    /// @param text string to evaluate
    /// @return horizontal size of the string for current font, in pixels
    /// @n @b More: @ref Fonts
    ///
    uint16_t _f_stringSizeX(char * text);

    ///
    /// @brief Number of characters to fit a size, x-axis
    /// @param text string to evaluate
    /// @param pixels number of pixels to fit in
    /// @return number of characters to be displayed inside the pixels
    /// @n @b More: @ref Fonts
    ///
    uint8_t _f_stringLengthToFitX(char * text, uint16_t pixels);

    ///
    /// @brief Number of fonts
    /// @return number of fonts available
    /// @note First font is numbered 0, second 1, ...
    /// @n The latest font is numbered fontMax()-1
    ///
    uint8_t _f_fontMax();

    ///
    /// @brief Get selected font details
    /// @return font description
    ///
    uint8_t _f_getFontKind();

    ///
    /// @brief Get selected font details
    /// @return maximum width in pixels from *width array
    ///
    uint8_t _f_getFontMaxWidth();

  protected:
    ///
    /// @brief Get definition for line of character
    /// @param character character 32~255
    /// @param index column index
    /// @return definition for line of character
    ///
    uint8_t _f_getCharacter(uint8_t character, uint16_t index);

    ///
    /// @name Variables for font management
    /// @{
    ///
    font_s _f_font; ///< font
    uint8_t _f_fontNumber; ///< number of fonts available, 0.._fontNumber-1
    uint8_t _f_fontSize; ///< actual font selected
    uint8_t _f_fontSpaceX; ///< pixels between two characters, horizontal axis
    uint8_t _f_fontSpaceY; ///< pixels between two characters, vertical axis
    bool _f_fontSolid; ///<

};
/// @endcond

#endif // USE_FONT_TERMINAL

#endif // hV_FONT_TERMINAL_RELEASE
