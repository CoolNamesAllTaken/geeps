#include "hardware/spi.h"
#include "Screen_EPD_EXT3.h"
#include "string.h"
#include <stdio.h> // for sprintf

#ifndef SPI_CLOCK_MAX
#define SPI_CLOCK_MAX 16000000
#endif

#define SPI_FREQ 4e6 // [Hz] Set to 4MHz by default.
#define SPI_BITS_PER_TRANSFER 8

// Convenience functions from Arduino.
void bitSet(uint8_t &value, uint8_t bit)
{
    value |= (0b1 << bit);
}
uint8_t bitRead(uint8_t value, uint8_t bit)
{
    return value & (0b1 << bit) ? 0b1 : 0b0;
}
void bitClear(uint8_t &value, uint8_t bit)
{
    value &= ~(0b1 << bit);
}
void digitalWrite(uint gpio, bool value)
{
    gpio_put(gpio, value);
}
bool digitalRead(uint gpio)
{
    return gpio_get(gpio);
}

/**
 * @brief Class constructor for Screen_EPD_EXT3 class.
 * @param[in] eScreen_EPD_EXT3 uint32_t that designates a screen type.
 * @param[in] board Struct containing a bunch of uint8_t's that describe each GPIO pin used by thge screen.
 * @param[in] epd_spi Pointer to the spi_inst_t that is used to communicate with the display.
 */
Screen_EPD_EXT3::Screen_EPD_EXT3(eScreen_EPD_EXT3_t eScreen_EPD_EXT3, pins_t board, spi_inst_t *epd_spi)
{
    _pin = board;
    _newImage = nullptr;
    _eScreen_EPD_EXT3 = eScreen_EPD_EXT3;
    epd_spi_ = epd_spi;
}

/**
 * @brief Sets model specific parameters (height / width etc), allocates the framebuffer if it's not yet allocated, then initializes
 * GPIO pins and SPI port to their relevant starting configurations.
 */
void Screen_EPD_EXT3::begin()
{
    _codeExtra = (_eScreen_EPD_EXT3 >> 16) & 0xff;
    _codeSize = (_eScreen_EPD_EXT3 >> 8) & 0xff;
    _codeType = _eScreen_EPD_EXT3 & 0xff;
    _screenColourBits = 2; // BWR

    switch (_codeSize)
    {
    case 0x15: // 1.54"

        _widthScreen = 152;  // x = wide size
        _heightScreen = 152; // y = small size
        _screenDiagonal = 154;
        _refreshTime = 16;
        break;

    case 0x21: // 2.13"

        _widthScreen = 212;  // x = wide size
        _heightScreen = 104; // y = small size
        _screenDiagonal = 213;
        _refreshTime = 15;
        break;

    case 0x26: // 2.66"

        _widthScreen = 296;  // x = wide size
        _heightScreen = 152; // y = small size
        _screenDiagonal = 266;
        _refreshTime = 15;
        break;

    case 0x27: // 2.71"

        _widthScreen = 264;  // x = wide size
        _heightScreen = 176; // y = small size
        _screenDiagonal = 271;
        _refreshTime = 19;
        break;

    case 0x28: // 2.87"

        _widthScreen = 296;  // x = wide size
        _heightScreen = 128; // y = small size
        _screenDiagonal = 287;
        _refreshTime = 14;
        break;

    case 0x37: // 3.70"

        _widthScreen = 416;  // x = wide size
        _heightScreen = 240; // y = small size
        _screenDiagonal = 370;
        _refreshTime = 15; // ?
        break;

    case 0x41: // 4.17"

        _widthScreen = 300;  // x = wide size
        _heightScreen = 400; // y = small size
        _screenDiagonal = 420;
        _refreshTime = 19;
        break;

    case 0x43: // 4.37"

        _widthScreen = 480;  // x = wide size
        _heightScreen = 176; // y = small size
        _screenDiagonal = 437;
        _refreshTime = 21;
        break;

    case 0x56: // 5.65"

        _widthScreen = 600;  // x = wide size
        _heightScreen = 448; // y = small size
        _screenDiagonal = 565;
        _refreshTime = 32;
        break;

    case 0x58: // 5.81"

        _widthScreen = 720;  // x = wide size
        _heightScreen = 256; // y = small size
        _screenDiagonal = 581;
        _refreshTime = 32;
        break;

    case 0x74: // 7.40"

        _widthScreen = 800;  // x = wide size
        _heightScreen = 480; // y = small size
        _screenDiagonal = 741;
        _refreshTime = 32;
        break;

    case 0x96: // 9.69"

        _widthScreen = 672;  // x = wide size
        _heightScreen = 960; // Actually, 960 = 480 x 2, y = small size
        _screenDiagonal = 970;
        _refreshTime = 45;
        break;

    case 0xB9: // 11.98"

        _widthScreen = 768;  // x = wide size
        _heightScreen = 960; // Actually, 960 = 480 x 2, y = small size
        _screenDiagonal = 1220;
        _refreshTime = 45;
        break;

    default:

        break;
    }

    _depthBuffer = _screenColourBits;  // 2 colours
    _widthBuffer = _widthScreen;       // x = wide size
    _heightBuffer = _heightScreen / 8; // small size 112 / 8;

    // Force conversion for two unit16_t multiplication into uint32_t.
    // Actually for 1 colour; BWR requires 2 pages.
    _sizePageColour = (uint32_t)_widthBuffer * (uint32_t)_heightBuffer;

    // _sizeFrame = _sizePageColour, except for 9.69 and 11.98
    // 9.69 and 11.98 combine two half-screens, hence two frames with adjusted size
    switch (_codeSize)
    {
    case 0x96: // 9.69"
    case 0xB9: // 11.98"

        _sizeFrame = _sizePageColour / 2;
        break;

    default:

        _sizeFrame = _sizePageColour;
        break;
    }

    if (_newImage == NULL)
    {
        static uint8_t *_newFrameBuffer;
        _newFrameBuffer = new uint8_t[_sizePageColour * _depthBuffer];
        _newImage = (uint8_t *)_newFrameBuffer;
    }

    // Check FRAM
    bool flag = true;
    uint8_t count = 8;

    _newImage[1] = 0x00;
    while (flag)
    {
        _newImage[1] = 0xaa;
        busy_wait_ms(100);
        if ((_newImage[1] == 0xaa) or (count == 0))
        {
            flag = false;
        }
        count--;
    }
    memset(_newImage, 0x00, _sizePageColour * _depthBuffer);

    // Initialise the /CS pins
    gpio_init(_pin.panelCS);
    gpio_set_dir(_pin.panelCS, GPIO_OUT);
    gpio_put(_pin.panelCS, HIGH); // CS# = 1

    // New generic solution
    gpio_init(_pin.panelDC);
    gpio_init(_pin.panelReset);
    gpio_init(_pin.panelBusy);
    gpio_set_dir(_pin.panelDC, GPIO_OUT);
    gpio_set_dir(_pin.panelReset, GPIO_OUT);
    gpio_set_dir(_pin.panelBusy, GPIO_IN); // All Pins 0

    // Initialise Flash /CS as HIGH
    if (_pin.flashCS != NOT_CONNECTED)
    {
        gpio_init(_pin.flashCS);
        gpio_set_dir(_pin.flashCS, GPIO_OUT);
        gpio_put(_pin.flashCS, HIGH);
    }

    // Initialise slave panel /CS as HIGH
    if (_pin.panelCSS != NOT_CONNECTED)
    {
        gpio_init(_pin.panelCSS);
        gpio_set_dir(_pin.panelCSS, GPIO_OUT);
        gpio_put(_pin.panelCSS, HIGH);
    }

    // Initialise slave Flash /CS as HIGH
    if (_pin.flashCSS != NOT_CONNECTED)
    {
        gpio_init(_pin.flashCSS);
        gpio_set_dir(_pin.flashCSS, GPIO_OUT);
        gpio_put(_pin.flashCSS, HIGH);
    }

    // Initialise SD-card /CS as HIGH
    if (_pin.cardCS != NOT_CONNECTED)
    {
        gpio_init(_pin.cardCS);
        gpio_set_dir(_pin.cardCS, GPIO_OUT);
        gpio_put(_pin.cardCS, HIGH);
    }

    // Initialise SPI
    spi_init(epd_spi_, SPI_FREQ);
    spi_set_format(epd_spi_, SPI_BITS_PER_TRANSFER, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST); // SPI Mode 0, MSB first, 1 Byte

    // Reset
    switch (_codeSize)
    {
    case 0x41: // 4.17"
    case 0x43: // 4.37"

        _reset(5, 5, 10, 5, 5);
        break;

    case 0x56: // 5.65"
    case 0x58: // 5.81"
    case 0x74: // 7.40"

        _reset(200, 20, 200, 50, 5);
        break;

    case 0x96: // 9.69"
    case 0xB9: // 11.98"

        _reset(200, 20, 200, 200, 5);
        break;

    default:

        _reset(5, 5, 10, 5, 5);
        break;
    }

    _screenWidth = _heightScreen;
    _screenHeigth = _widthScreen;

    // Standard
    hV_Screen_Buffer::begin();

    setOrientation(0);
    if (_f_fontMax() > 0)
    {
        _f_selectFont(0);
    }
    _f_fontSolid = false;

    _penSolid = false;
    _invert = false;

    clear();
}

/**
 * @brief Resets the panel.
 * @param[in] ms1 Duration to wait before raising panelReset pin (ms).
 * @param[in] ms2 Duration of first panelReset pin HIGH (ms).
 * @param[in] ms3 Duration of first panelReset pin LOW (ms).
 * @param[in] ms4 Duration of second panelReset pin HIGH (ms) before raising panelCS pin.
 * @param[in] ms5 Duration to wait after raising panelCS pin before returning (ms).
 */
void Screen_EPD_EXT3::_reset(uint32_t ms1, uint32_t ms2, uint32_t ms3, uint32_t ms4, uint32_t ms5)
{
    // gpio_put(PNLON_PIN, HIGH); // PANEL_ON# = 1
    delay_ms(ms1);                   // delay_ms 5ms
    gpio_put(_pin.panelReset, HIGH); // RES# = 1
    delay_ms(ms2);                   // delay_ms 5ms
    gpio_put(_pin.panelReset, LOW);
    delay_ms(ms3);
    gpio_put(_pin.panelReset, HIGH);
    delay_ms(ms4);

    gpio_put(_pin.panelCS, HIGH); // CS# = 1

    // For 9.7 and 12.2 panels
    if ((_codeSize == 0x96) or (_codeSize == 0xB9))
    {
        if (_pin.panelCSS != NOT_CONNECTED)
        {
            gpio_put(_pin.panelCSS, HIGH); // CSS# = 1
        }
    }
    delay_ms(ms5);
}

/**
 * @brief Fills a string buffer with an info string that contains screen diagonal dimension and font mode.
 * @param[out] str_buf String buffer to fill with information string.
 */
void Screen_EPD_EXT3::WhoAmI(char *str_buf)
{
    char font_mode = '?';
#if (FONT_MODE == USE_FONT_HEADER)

    font_mode = 'H';

#elif (FONT_MODE == USE_FONT_FLASH)

    font_mode = 'F';

#elif (FONT_MODE == USE_FONT_TERMINAL)

    font_mode = 'T';

#endif // FONT_MODE

    sprintf(str_buf, "iTC %d.%d \" -%c", _screenDiagonal / 100, _screenDiagonal % 100, font_mode);
}

/**
 * @brief Standard function for flushing a framebuffer to a display with the proper global refresh. Takes a long-ass
 *          time but maybe does good things.
 */
void Screen_EPD_EXT3::flush()
{
    uint8_t *blackBuffer = _newImage;
    uint8_t *redBuffer = _newImage + _sizePageColour;

    // Three groups:
    // + small: up to 4.37
    // + medium: 5.65, 5.81 and 7.4
    // + large: 9.69 and 11,98
    // switch..case doesn't allow variable declarations
    //
    if ((_codeSize == 0x56) or (_codeSize == 0x58) or (_codeSize == 0x74))
    {
        _reset(200, 20, 200, 50, 5);

        // Send image data
        if (_codeSize == 0x56)
        {
            uint8_t data1_565[] = {0x00, 0x37, 0x00, 0x00, 0x57, 0x02}; // DUW
            _sendIndexData(0x13, data1_565, 6);                         // DUW
            uint8_t data2_565[] = {0x00, 0x37, 0x00, 0x97};             // DRFW
            _sendIndexData(0x90, data2_565, 4);                         // DRFW
            uint8_t data3_565[] = {0x37, 0x00, 0x14};                   // RAM_RW
            _sendIndexData(0x12, data3_565, 3);                         // RAM_RW
        }
        else if (_codeSize == 0x58)
        {
            uint8_t data1_565[] = {0x00, 0x1f, 0x50, 0x00, 0x1f, 0x03}; // DUW
            _sendIndexData(0x13, data1_565, 6);                         // DUW
            uint8_t data2_565[] = {0x00, 0x1f, 0x00, 0xc9};             // DRFW
            _sendIndexData(0x90, data2_565, 4);                         // DRFW
            uint8_t data3_565[] = {0x1f, 0x50, 0x14};                   // RAM_RW
            _sendIndexData(0x12, data3_565, 3);                         // RAM_RW
        }
        else if (_codeSize == 0x74)
        {
            uint8_t data1_565[] = {0x00, 0x3b, 0x00, 0x00, 0x1f, 0x03}; // DUW
            _sendIndexData(0x13, data1_565, 6);                         // DUW
            uint8_t data2_565[] = {0x00, 0x3b, 0x00, 0xc9};             // DRFW
            _sendIndexData(0x90, data2_565, 4);                         // DRFW
            uint8_t data3_565[] = {0x3b, 0x00, 0x14};                   // RAM_RW
            _sendIndexData(0x12, data3_565, 3);                         // RAM_RW
        }

        if (_codeType == 0x0B)
        {
            // y1 = 7 - (y1 % 8);
            uint8_t dtcl = 0x08;            // 0=IST, 8=IST
            _sendIndexData(0x01, &dtcl, 1); // DCTL 0x10 of MTP
        }

        _sendIndexData(0x10, blackBuffer, _sizeFrame); // First frame

        if (_codeSize == 0x56)
        {
            uint8_t data3_565[] = {0x37, 0x00, 0x14}; // RAM_RW
            _sendIndexData(0x12, data3_565, 3);       // RAM_RW
        }
        else if (_codeSize == 0x58)
        {
            uint8_t data3_565[] = {0x1f, 0x50, 0x14}; // RAM_RW
            _sendIndexData(0x12, data3_565, 3);       // RAM_RW
        }
        else if (_codeSize == 0x74)
        {
            uint8_t data3_565[] = {0x3b, 0x00, 0x14}; // RAM_RW
            _sendIndexData(0x12, data3_565, 3);       // RAM_RW
        }
        _sendIndexData(0x11, redBuffer, _sizeFrame); // Second frame

        // Initial COG
        uint8_t data4_565[] = {0x7d};
        _sendIndexData(0x05, data4_565, 1);
        delay_ms(200);
        uint8_t data5_565[] = {0x00};
        _sendIndexData(0x05, data5_565, 1);
        delay_ms(10);
        uint8_t data6_565[] = {0x3f};
        _sendIndexData(0xc2, data6_565, 1);
        delay_ms(1);
        uint8_t data7_565[] = {0x00};
        _sendIndexData(0xd8, data7_565, 1); // MS_SYNC mtp_0x1d
        uint8_t data8_565[] = {0x00};
        _sendIndexData(0xd6, data8_565, 1); // BVSS mtp_0x1e
        uint8_t data9_565[] = {0x10};
        _sendIndexData(0xa7, data9_565, 1);
        delay_ms(100);
        _sendIndexData(0xa7, data5_565, 1);
        delay_ms(100);
        // uint8_t data10_565[] = {0x00, 0x02 };
        if (_codeSize == 0x56)
        {
            uint8_t data10_565[] = {0x00, 0x02}; // OSC
            _sendIndexData(0x03, data10_565, 2); // OSC mtp_0x12
        }
        else if (_codeSize == 0x58)
        {
            uint8_t data10_565[] = {0x00, 0x01}; // OSC
            _sendIndexData(0x03, data10_565, 2); // OSC mtp_0x12
        }
        else if (_codeSize == 0x74)
        {
            uint8_t data10_565[] = {0x00, 0x01}; // OSC
            _sendIndexData(0x03, data10_565, 2); // OSC mtp_0x12
        }
        _sendIndexData(0x44, data5_565, 1);
        uint8_t data11_565[] = {0x80};
        _sendIndexData(0x45, data11_565, 1);
        _sendIndexData(0xa7, data9_565, 1);
        delay_ms(100);
        _sendIndexData(0xa7, data7_565, 1);
        delay_ms(100);
        uint8_t data12_565[] = {0x06};
        _sendIndexData(0x44, data12_565, 1);
        uint8_t data13_565[] = {0x82};
        _sendIndexData(0x45, data13_565, 1); // Temperature 0x82@25C
        _sendIndexData(0xa7, data9_565, 1);
        delay_ms(100);
        _sendIndexData(0xa7, data7_565, 1);
        delay_ms(100);
        uint8_t data14_565[] = {0x25};
        _sendIndexData(0x60, data14_565, 1); // TCON mtp_0x0b
        // uint8_t data15_565[] = {0x01 };
        if (_codeSize == 0x56)
        {
            uint8_t data15_565[] = {0x01};       // STV_DIR
            _sendIndexData(0x61, data15_565, 1); // STV_DIR mtp_0x1c
        }
        else if (_codeSize == 0x58)
        {
            uint8_t data15_565[] = {0x00};       // STV_DIR
            _sendIndexData(0x61, data15_565, 1); // STV_DIR mtp_0x1c
        }
        else if (_codeSize == 0x74)
        {
            uint8_t data15_565[] = {0x00};       // STV_DIR
            _sendIndexData(0x61, data15_565, 1); // STV_DIR mtp_0x1c
        }
        uint8_t data16_565[] = {0x00};
        _sendIndexData(0x01, data16_565, 1); // DCTL mtp_0x10
        uint8_t data17_565[] = {0x00};
        _sendIndexData(0x02, data17_565, 1); // VCOM mtp_0x11

        // DC-DC soft-start
        uint8_t index51_565[] = {0x50, 0x01, 0x0a, 0x01};
        _sendIndexData(0x51, &index51_565[0], 2);
        uint8_t index09_565[] = {0x1f, 0x9f, 0x7f, 0xff};

        for (int value = 1; value <= 4; value++)
        {
            _sendIndexData(0x09, &index09_565[0], 1);
            index51_565[1] = value;
            _sendIndexData(0x51, &index51_565[0], 2);
            _sendIndexData(0x09, &index09_565[1], 1);
            delay_ms(2);
        }
        for (int value = 1; value <= 10; value++)
        {
            _sendIndexData(0x09, &index09_565[0], 1);
            index51_565[3] = value;
            _sendIndexData(0x51, &index51_565[2], 2);
            _sendIndexData(0x09, &index09_565[1], 1);
            delay_ms(2);
        }
        for (int value = 3; value <= 10; value++)
        {
            _sendIndexData(0x09, &index09_565[2], 1);
            index51_565[3] = value;
            _sendIndexData(0x51, &index51_565[2], 2);
            _sendIndexData(0x09, &index09_565[3], 1);
            delay_ms(2);
        }
        for (int value = 9; value >= 2; value--)
        {
            _sendIndexData(0x09, &index09_565[2], 1);
            index51_565[2] = value;
            _sendIndexData(0x51, &index51_565[2], 2);
            _sendIndexData(0x09, &index09_565[3], 1);
            delay_ms(2);
        }
        _sendIndexData(0x09, &index09_565[3], 1);
        delay_ms(10);

        // Display Refresh Start
        while (gpio_get(_pin.panelBusy) != HIGH)
        {
            busy_wait_ms(100);
        }
        uint8_t data18_565[] = {0x3c};
        _sendIndexData(0x15, data18_565, 1); // Display Refresh
        delay_ms(5);

        // DC-DC off
        while (gpio_get(_pin.panelBusy) != HIGH)
        {
            busy_wait_ms(100);
        }
        uint8_t data19_565[] = {0x7f};
        _sendIndexData(0x09, data19_565, 1);
        uint8_t data20_565[] = {0x7d};
        _sendIndexData(0x05, data20_565, 1);
        _sendIndexData(0x09, data5_565, 1);
        delay_ms(200);

        while (gpio_get(_pin.panelBusy) != HIGH)
        {
            busy_wait_ms(100);
        }
        gpio_put(_pin.panelDC, LOW);
        gpio_put(_pin.panelCS, LOW);
        gpio_put(_pin.panelReset, LOW);
        // gpio_put(PNLON_PIN, LOW); // PANEL_OFF# = 0
    }
    else if ((_codeSize == 0x96) or (_codeSize == 0xB9))
    {
        _reset(200, 20, 200, 200, 5);

        // Send image data
        if (_codeSize == 0x96)
        {
            uint8_t data1_970[] = {0x00, 0x3b, 0x00, 0x00, 0x9f, 0x02}; // DUW
            _sendIndexData(0x13, data1_970, 6);                         // DUW for Both Master and Slave
            uint8_t data2_970[] = {0x00, 0x3b, 0x00, 0xa9};             // DRFW
            _sendIndexData(0x90, data2_970, 4);                         // DRFW for Both Master and Slave
        }
        else if (_codeSize == 0xB9)
        {
            uint8_t data1_970[] = {0x00, 0x3b, 0x00, 0x00, 0x1f, 0x03}; // DUW
            _sendIndexData(0x13, data1_970, 6);                         // DUW for Both Master and Slave
            uint8_t data2_970[] = {0x00, 0x3b, 0x00, 0xc9};             // DRFW
            _sendIndexData(0x90, data2_970, 4);                         // DRFW for Both Master and Slave
        }

        uint8_t data3_970[] = {0x3b, 0x00, 0x14};

        if (_codeType == 0x0B)
        {
            uint8_t dtcl = 0x08;            // 0=IST, 8=IST
            _sendIndexData(0x01, &dtcl, 1); // DCTL 0x10 of MTP
        }

        // Master
        _sendIndexDataMaster(0x12, data3_970, 3); // RAM_RW

        _sendIndexDataMaster(0x10, blackBuffer, _sizeFrame); // First frame

        _sendIndexDataMaster(0x12, data3_970, 3); // RAM_RW

        _sendIndexDataMaster(0x11, redBuffer, _sizeFrame); // Second frame

        // Slave
        _sendIndexDataSlave(0x12, data3_970, 3);                         // RAM_RW
        _sendIndexDataSlave(0x10, blackBuffer + _sizeFrame, _sizeFrame); // First frame

        _sendIndexDataSlave(0x12, data3_970, 3);                       // RAM_RW
        _sendIndexDataSlave(0x11, redBuffer + _sizeFrame, _sizeFrame); // Second frame

        // Initial COG
        uint8_t data4_970[] = {0x7d};
        _sendIndexData(0x05, data4_970, 1);
        delay_ms(200);
        uint8_t data5_970[] = {0x00};
        _sendIndexData(0x05, data5_970, 1);
        delay_ms(10);
        uint8_t data6_970[] = {0x3f};
        _sendIndexData(0xc2, data6_970, 1);
        delay_ms(1);
        uint8_t data7_970[] = {0x80};
        _sendIndexData(0xd8, data7_970, 1); // MS_SYNC
        uint8_t data8_970[] = {0x00};
        _sendIndexData(0xd6, data8_970, 1); // BVSS
        uint8_t data9_970[] = {0x10};
        _sendIndexData(0xa7, data9_970, 1);
        delay_ms(100);
        _sendIndexData(0xa7, data5_970, 1);
        delay_ms(100);
        // uint8_t data10_970[] = {0x00, 0x11}; // --- 9.69 specific
        if (_codeSize == 0x96)
        {
            uint8_t data10_970[] = {0x00, 0x11}; // OSC
            _sendIndexData(0x03, data10_970, 2); // OSC
        }
        else if (_codeSize == 0xB9)
        {
            uint8_t data10_970[] = {0x00, 0x12}; // OSC
            _sendIndexData(0x03, data10_970, 2); // OSC
        }

        _sendIndexDataMaster(0x44, data5_970, 1); // Master
        uint8_t data11_970[] = {0x80};
        _sendIndexDataMaster(0x45, data11_970, 1); // Master
        _sendIndexDataMaster(0xa7, data9_970, 1);  // Master
        delay_ms(100);
        _sendIndexDataMaster(0xa7, data5_970, 1); // Master
        delay_ms(100);
        uint8_t data12_970[] = {0x06};
        _sendIndexDataMaster(0x44, data12_970, 1); // Master
        uint8_t data13_970[] = {0x82};
        // uint8_t data13_970[] = {getTemperature(0x50, 0x82) };
        _sendIndexDataMaster(0x45, data13_970, 1); // Temperature 0x82@25C   0°C = 0x50, 25°C = 0x82
        _sendIndexDataMaster(0xa7, data9_970, 1);  // Master
        delay_ms(100);
        _sendIndexDataMaster(0xa7, data5_970, 1); // Master
        delay_ms(100);

        _sendIndexDataSlave(0x44, data5_970, 1);  // Slave
        _sendIndexDataSlave(0x45, data11_970, 1); // Slave
        _sendIndexDataSlave(0xa7, data9_970, 1);  // Slave
        delay_ms(100);
        _sendIndexDataSlave(0xa7, data5_970, 1); // Slave
        delay_ms(100);
        _sendIndexDataSlave(0x44, data12_970, 1); // Slave
        _sendIndexDataSlave(0x45, data13_970, 1); // Temperature 0x82@25C   0°C = 0x50, 25°C = 0x82
        _sendIndexDataSlave(0xa7, data9_970, 1);  // Slave
        delay_ms(100);
        _sendIndexDataSlave(0xa7, data5_970, 1); // Master
        delay_ms(100);

        uint8_t data14_970[] = {0x25};
        _sendIndexData(0x60, data14_970, 1); // TCON
        uint8_t data15_970[] = {0x01};
        _sendIndexDataMaster(0x61, data15_970, 1); // STV_DIR for Master
        uint8_t data16_970[] = {0x00};
        _sendIndexData(0x01, data16_970, 1); // DCTL
        uint8_t data17_970[] = {0x00};
        _sendIndexData(0x02, data17_970, 1); // VCOM

        // DC-DC soft-start
        uint8_t index51_970[] = {0x50, 0x01, 0x0a, 0x01};
        _sendIndexData(0x51, &index51_970[0], 2);
        uint8_t index09_970[] = {0x1f, 0x9f, 0x7f, 0xff};

        for (int value = 1; value <= 4; value++)
        {
            _sendIndexData(0x09, &index09_970[0], 1);
            index51_970[1] = value;
            _sendIndexData(0x51, &index51_970[0], 2);
            _sendIndexData(0x09, &index09_970[1], 1);
            delay_ms(2);
        }
        for (int value = 1; value <= 10; value++)
        {
            _sendIndexData(0x09, &index09_970[0], 1);
            index51_970[3] = value;
            _sendIndexData(0x51, &index51_970[2], 2);
            _sendIndexData(0x09, &index09_970[1], 1);
            delay_ms(2);
        }
        for (int value = 3; value <= 10; value++)
        {
            _sendIndexData(0x09, &index09_970[2], 1);
            index51_970[3] = value;
            _sendIndexData(0x51, &index51_970[2], 2);
            _sendIndexData(0x09, &index09_970[3], 1);
            delay_ms(2);
        }
        for (int value = 9; value >= 2; value--)
        {
            _sendIndexData(0x09, &index09_970[2], 1);
            index51_970[2] = value;
            _sendIndexData(0x51, &index51_970[2], 2);
            _sendIndexData(0x09, &index09_970[3], 1);
            delay_ms(2);
        }
        _sendIndexData(0x09, &index09_970[3], 1);
        delay_ms(10);

        // Display Refresh Start
        while (gpio_get(_pin.panelBusy) != HIGH)
        {
            busy_wait_ms(100);
        }
        uint8_t data18_970[] = {0x3c};
        _sendIndexData(0x15, data18_970, 1); // Display Refresh
        delay_ms(5);

        // DC/DC off
        while (gpio_get(_pin.panelBusy) != HIGH)
        {
            busy_wait_ms(100);
        }
        uint8_t data19_970[] = {0x7f};
        _sendIndexData(0x09, data19_970, 1);
        uint8_t data20_970[] = {0x7d};
        _sendIndexData(0x05, data20_970, 1);
        _sendIndexData(0x09, data5_970, 1);
        delay_ms(200);
        while (gpio_get(_pin.panelBusy) != HIGH)
        {
            busy_wait_ms(100);
        }
        gpio_put(_pin.panelDC, LOW);
        gpio_put(_pin.panelCS, LOW);

        if (_pin.panelCSS != NOT_CONNECTED)
        {
            gpio_put(_pin.panelCSS, LOW);
        }

        gpio_put(_pin.panelReset, LOW);
        // gpio_put(PNLON_PIN, LOW); // PANEL_OFF# = 0

        if (_pin.panelCSS != NOT_CONNECTED)
        {
            gpio_put(_pin.panelCSS, HIGH); // CSS# = 1
        }
    }
    else // small, including 420 and 437
    {
        _reset(5, 5, 10, 5, 5);

        uint8_t data9[] = {0x0e};
        _sendIndexData(0x00, data9, 1); // Soft-reset
        delay_ms(5);

        uint8_t data7[] = {0x19};
        // uint8_t data7[] = {getTemperature() };
        _sendIndexData(0xe5, data7, 1); // Input Temperature 0°C = 0x00, 22°C = 0x16, 25°C = 0x19
        uint8_t data6[] = {0x02};
        _sendIndexData(0xe0, data6, 1); // Active Temperature

        // Send image data
        _sendIndexData(0x10, blackBuffer, _sizeFrame); // First frame
        _sendIndexData(0x13, redBuffer, _sizeFrame);   // Second frame

        delay_ms(50);
        uint8_t data8[] = {0x00};
        _sendIndexData(0x04, data8, 1); // Power on
        delay_ms(5);
        while (gpio_get(_pin.panelBusy) != HIGH)
        {
            busy_wait_ms(100);
        };

        while (gpio_get(_pin.panelBusy) != HIGH)
            ;
        _sendIndexData(0x12, data8, 1); // Display Refresh
        delay_ms(5);
        while (gpio_get(_pin.panelBusy) != HIGH)
        {
            busy_wait_ms(100);
        };

        _sendIndexData(0x02, data8, 1); // Turn off DC/DC
        delay_ms(5);
        while (gpio_get(_pin.panelBusy) != HIGH)
        {
            busy_wait_ms(100);
        };
        gpio_put(_pin.panelDC, LOW);
        gpio_put(_pin.panelCS, LOW);

        gpio_put(_pin.panelReset, LOW);
        // gpio_put(PNLON_PIN, LOW);
    }
    gpio_put(_pin.panelCS, HIGH); // CS# = 1
}

/**
 * @brief Clears the framebuffer by setting all pixels to a specific color.
 * @param[in] colour uint16_t indicating which color to fill the framebuffer with. Options are black, white, gray,
 *                      red, dark red, light red.
 */
void Screen_EPD_EXT3::clear(uint16_t colour)
{
    if (colour == myColours.red)
    {
        // physical red 01
        memset(_newImage, 0x00, _sizePageColour);
        memset(_newImage + _sizePageColour, 0xff, _sizePageColour);
    }
    else if (colour == myColours.grey)
    {
        for (uint16_t i = 0; i < _widthBuffer; i++)
        {
            uint16_t pattern = (i % 2) ? 0b10101010 : 0b01010101;
            for (uint16_t j = 0; j < _heightBuffer; j++)
            {
                _newImage[i * _heightBuffer + j] = pattern;
            }
        }
        memset(_newImage + _sizePageColour, 0x00, _sizePageColour);
    }
    else if (colour == myColours.darkRed)
    {
        // red = 0-1, black = 1-0, white 0-0
        for (uint16_t i = 0; i < _widthBuffer; i++)
        {
            uint16_t pattern1 = (i % 2) ? 0b10101010 : 0b01010101; // black
            uint16_t pattern2 = (i % 2) ? 0b01010101 : 0b10101010; // red
            for (uint16_t j = 0; j < _heightBuffer; j++)
            {
                _newImage[i * _heightBuffer + j] = pattern1;
                _newImage[i * _heightBuffer + j + _sizePageColour] = pattern2;
            }
        }
    }
    else if (colour == myColours.lightRed)
    {
        // red = 0-1, black = 1-0, white 0-0
        for (uint16_t i = 0; i < _widthBuffer; i++)
        {
            uint16_t pattern1 = (i % 2) ? 0b00000000 : 0b00000000; // white
            uint16_t pattern2 = (i % 2) ? 0b01010101 : 0b10101010; // red
            for (uint16_t j = 0; j < _heightBuffer; j++)
            {
                _newImage[i * _heightBuffer + j] = pattern1;
                _newImage[i * _heightBuffer + j + _sizePageColour] = pattern2;
            }
        }
    }
    else if ((colour == myColours.white) xor _invert)
    {
        // physical black 00
        memset(_newImage, 0x00, _sizePageColour);
        memset(_newImage + _sizePageColour, 0x00, _sizePageColour);
    }
    else
    {
        // physical white 10
        memset(_newImage, 0xff, _sizePageColour);
        memset(_newImage + _sizePageColour, 0x00, _sizePageColour);
    }
}

/**
 * @brief Sets a flag that inverts the colors on the screen. Useful for toggling pixels for important reasons.
 * @param[in] flag Invert if true, don't invert if false.
 */
void Screen_EPD_EXT3::invert(bool flag)
{
    _invert = flag;
}

void Screen_EPD_EXT3::_setPoint(uint16_t x1, uint16_t y1, uint16_t colour)
{
    // Orient and check coordinates are within screen
    // _orientCoordinates() returns false=success, true=error
    if (_orientCoordinates(x1, y1))
    {
        return;
    }

    uint32_t z1 = _getZ(x1, y1);

    // Convert combined colours into basic colours
    bool flagOdd = ((x1 + y1) % 2 == 0);

    if (colour == myColours.darkRed)
    {
        if (flagOdd)
        {
            colour = myColours.red; // red
        }
        else
        {
            colour = _invert ? myColours.white : myColours.black; // white
        }
    }
    else if (colour == myColours.lightRed)
    {
        if (flagOdd)
        {
            colour = myColours.red; // red
        }
        else
        {
            colour = _invert ? myColours.black : myColours.white; // black
        }
    }
    else if (colour == myColours.grey)
    {
        if (flagOdd)
        {
            colour = myColours.black; // black
        }
        else
        {
            colour = myColours.white; // white
        }
    }

    // Basic colours
    if (colour == myColours.red)
    {
        // physical red 01
        bitClear(_newImage[z1], 7 - (y1 % 8));
        bitSet(_newImage[_sizePageColour + z1], 7 - (y1 % 8));
    }
    else if ((colour == myColours.white) xor _invert)
    {
        // physical black 00
        bitClear(_newImage[z1], 7 - (y1 % 8));
        bitClear(_newImage[_sizePageColour + z1], 7 - (y1 % 8));
    }
    else if ((colour == myColours.black) xor _invert)
    {
        // physical white 10
        bitSet(_newImage[z1], 7 - (y1 % 8));
        bitClear(_newImage[_sizePageColour + z1], 7 - (y1 % 8));
    }
}

void Screen_EPD_EXT3::_setOrientation(uint8_t orientation)
{
    _orientation = orientation % 4;
}

bool Screen_EPD_EXT3::_orientCoordinates(uint16_t &x, uint16_t &y)
{
    bool flag = true; // false=success, true=error
    switch (_orientation)
    {
    case 3: // checked, previously 1

        if ((x < _widthScreen) and (y < _heightScreen))
        {
            x = _widthScreen - 1 - x;
            flag = false;
        }
        break;

    case 2: // checked

        if ((x < _heightScreen) and (y < _widthScreen))
        {
            x = _heightScreen - 1 - x;
            y = _widthScreen - 1 - y;
            swap(x, y);
            flag = false;
        }
        break;

    case 1: // checked, previously 3

        if ((x < _widthScreen) and (y < _heightScreen))
        {
            y = _heightScreen - 1 - y;
            flag = false;
        }
        break;

    default: // checked

        if ((x < _heightScreen) and (y < _widthScreen))
        {
            swap(x, y);
            flag = false;
        }
        break;
    }

    return flag;
}

/**
 * @brief Returns byte index of a given set of X/Y coordinates in the framebuffer.
 * @param[in] x1 X-coordinate for given byte.
 * @param[in] y1 Y-coordinate for given byte.
 * @retval Byte index of given location.
 */
uint32_t Screen_EPD_EXT3::_getZ(uint16_t x1, uint16_t y1)
{
    uint32_t z1 = 0;
    // According to 11,98 inch Spectra Application Note
    // at http:// www.pervasivedisplays.com/LiteratureRetrieve.aspx?ID=245146
    if ((_codeSize == 0x96) or (_codeSize == 0xB9))
    {
        if (y1 >= (_heightScreen >> 1))
        {
            y1 -= (_heightScreen >> 1);   // rebase y1
            z1 += (_sizePageColour >> 1); // buffer second half
        }
        z1 += (uint32_t)x1 * (_heightBuffer >> 1) + (y1 >> 3);
    }
    else
    {
        z1 = (uint32_t)x1 * _heightBuffer + (y1 >> 3);
    }
    return z1;
}

uint16_t Screen_EPD_EXT3::_getPoint(uint16_t x1, uint16_t y1)
{
    // Orient and check coordinates are within screen
    // _orientCoordinates() returns false = success, true = error
    if (_orientCoordinates(x1, y1))
    {
        return 0;
    }

    uint16_t result = 0;
    uint8_t value = 0;

    uint32_t z1 = _getZ(x1, y1);

    value = bitRead(_newImage[z1], 7 - (y1 % 8));
    value <<= 4;
    value |= bitRead(_newImage[_sizePageColour + z1], 7 - (y1 % 8));

    // red = 0-1, black = 1-0, white 0-0
    switch (value)
    {
    case 0x10:

        result = myColours.black;
        break;

    case 0x01:

        result = myColours.red;
        break;

    default:

        result = myColours.white;
        break;
    }

    return result;
}

void Screen_EPD_EXT3::point(uint16_t x1, uint16_t y1, uint16_t colour)
{
    _setPoint(x1, y1, colour);
}

uint16_t Screen_EPD_EXT3::readPixel(uint16_t x1, uint16_t y1)
{
    return _getPoint(x1, y1);
}

// Utilities
void Screen_EPD_EXT3::_sendIndexData(uint8_t index, const uint8_t *data, uint32_t size)
{
    gpio_put(_pin.panelDC, LOW); // DC Low
    gpio_put(_pin.panelCS, LOW); // CS Low
    if ((_codeSize == 0x96) or (_codeSize == 0xB9))
    {
        if (_pin.panelCSS != NOT_CONNECTED)
        {
            gpio_put(_pin.panelCSS, LOW);
        }
        busy_wait_us(450); // 450 + 50 = 500
    }
    busy_wait_us(50);
    spi_write_blocking(epd_spi_, &index, 1);
    // SPI.transfer(index);
    busy_wait_us(50);
    if ((_codeSize == 0x96) or (_codeSize == 0xB9))
    {
        if (_pin.panelCSS != NOT_CONNECTED)
        {
            busy_wait_us(450); // 450 + 50 = 500
            gpio_put(_pin.panelCSS, HIGH);
        }
    }
    gpio_put(_pin.panelCS, HIGH); // CS High
    gpio_put(_pin.panelDC, HIGH); // DC High
    gpio_put(_pin.panelCS, LOW);  // CS Low
    if ((_codeSize == 0x96) or (_codeSize == 0xB9))
    {
        if (_pin.panelCSS != NOT_CONNECTED)
        {
            gpio_put(_pin.panelCSS, LOW); // CSS Low
            busy_wait_us(450);            // 450 + 50 = 500
        }
    }
    busy_wait_us(50);
    for (uint32_t i = 0; i < size; i++)
    {
        spi_write_blocking(epd_spi_, &data[i], 1);
        // SPI.transfer(data[i]);
    }
    busy_wait_us(50);
    if ((_codeSize == 0x96) or (_codeSize == 0xB9))
    {
        if (_pin.panelCSS != NOT_CONNECTED)
        {
            busy_wait_us(450); // 450 + 50 = 500
            gpio_put(_pin.panelCSS, HIGH);
        }
    }
    gpio_put(_pin.panelCS, HIGH); // CS High
}

// Software SPI Master protocol setup
void Screen_EPD_EXT3::_sendIndexDataMaster(uint8_t index, const uint8_t *data, uint32_t size)
{
    if (_pin.panelCSS != NOT_CONNECTED)
    {
        gpio_put(_pin.panelCSS, HIGH); // CS slave HIGH
    }
    gpio_put(_pin.panelDC, LOW); // DC Low = Command
    gpio_put(_pin.panelCS, LOW); // CS Low = Select
    busy_wait_us(500);
    spi_write_blocking(epd_spi_, &index, 1);
    // SPI.transfer(index);
    busy_wait_us(500);
    gpio_put(_pin.panelCS, HIGH); // CS High = Unselect
    gpio_put(_pin.panelDC, HIGH); // DC High = Data
    gpio_put(_pin.panelCS, LOW);  // CS Low = Select
    busy_wait_us(500);

    for (uint32_t i = 0; i < size; i++)
    {
        // SPI.transfer(data[i]);
        spi_write_blocking(epd_spi_, &data[i], 1);
    }
    busy_wait_us(500);
    gpio_put(_pin.panelCS, HIGH); // CS High= Unselect
}

// Software SPI Slave protocol setup
void Screen_EPD_EXT3::_sendIndexDataSlave(uint8_t index, const uint8_t *data, uint32_t size)
{
    gpio_put(_pin.panelCS, HIGH); // CS Master High
    gpio_put(_pin.panelDC, LOW);  // DC Low= Command
    if (_pin.panelCSS != NOT_CONNECTED)
    {
        gpio_put(_pin.panelCSS, LOW); // CS slave LOW
    }

    busy_wait_us(500);
    // SPI.transfer(index);
    spi_write_blocking(epd_spi_, &index, 1);
    busy_wait_us(500);

    if (_pin.panelCSS != NOT_CONNECTED)
    {
        gpio_put(_pin.panelCSS, HIGH); // CS slave HIGH
    }

    gpio_put(_pin.panelDC, HIGH); // DC High = Data

    if (_pin.panelCSS != NOT_CONNECTED)
    {
        gpio_put(_pin.panelCSS, LOW); // CS slave LOW
    }

    busy_wait_us(500);

    for (uint32_t i = 0; i < size; i++)
    {
        spi_write_blocking(epd_spi_, &data[i], 1);
        // SPI.transfer(data[i]);
    }
    busy_wait_us(500);
    if (_pin.panelCSS != NOT_CONNECTED)
    {
        gpio_put(_pin.panelCSS, HIGH); // CS slave HIGH
    }
}

uint8_t Screen_EPD_EXT3::getRefreshTime()
{
    return _refreshTime;
}

// Functions imported from Screen_EPD_EXT3_Basic-Fast

void Screen_EPD_EXT3::_waitBusy()
{
    // LOW = busy, HIGH = ready
    while (digitalRead(_pin.panelBusy) != HIGH)
    {
        delay_ms(32); // non-blocking
    }
}

void Screen_EPD_EXT3::flush_fast()
{
    uint8_t *nextBuffer = _newImage;
    uint8_t *previousBuffer = _newImage + _sizePageColour;

    _reset(5, 5, 10, 5, 5);

    uint8_t data9[] = {0x0e};
    _sendIndexData(0x00, data9, 1); // Soft-reset
    delay_ms(5);

    uint8_t data7[] = {0x19 | 0x40};
    // uint8_t data7[] = {getTemperature() };
    _sendIndexData(0xe5, data7, 1); // Input Temperature 0°C = 0x00, 22°C = 0x16, 25°C = 0x19
    uint8_t data6[] = {0x02};
    _sendIndexData(0xe0, data6, 1); // Active Temperature

    uint8_t data0[] = {0xcf | 0x10, 0x8d | 0x02};
    _sendIndexData(0x00, data0, 2); // PSR

    uint8_t data4[] = {0x07};
    _sendIndexData(0x50, data4, 1); // Vcom and data interval setting

    // Send image data
    _sendIndexData(0x10, previousBuffer, _sizeFrame); // Previous frame
    _sendIndexData(0x13, nextBuffer, _sizeFrame);     // Next frame
    memcpy(previousBuffer, nextBuffer, _sizeFrame);   // Copy displayed next to previous

    delay_ms(50);
    _waitBusy();
    uint8_t data8[] = {0x00};
    _sendIndexData(0x04, data8, 1); // Power on
    delay_ms(5);
    _waitBusy();

    // This is the step that takes freakin forever.
    _sendIndexData(0x12, data8, 1); // Display Refresh
    delay_ms(5);
    _waitBusy();

    _sendIndexData(0x02, data8, 1); // Turn off DC/DC
    delay_ms(5);
    _waitBusy();
    digitalWrite(_pin.panelDC, LOW);
    digitalWrite(_pin.panelCS, LOW);

    digitalWrite(_pin.panelReset, LOW);
    // digitalWrite(PNLON_PIN, LOW);

    digitalWrite(_pin.panelCS, HIGH); // CS# = 1
}

void Screen_EPD_EXT3::regenerate()
{
    clear(myColours.black);
    flush_fast();

    delay_ms(100);

    clear(myColours.white);
    flush_fast();
}
