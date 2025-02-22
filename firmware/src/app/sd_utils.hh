#pragma once

#include "bsp.hh"
#include "sd_card.h"
#include "stdio.h"

#define STA_NOINIT  0x01  // Drive not initialized
#define STA_NODISK  0x02  // No medium in the drive
#define STA_PROTECT 0x04  // Write protected

static const uint16_t kErrNameMaxLen = 30;
static const char kErrNames[][kErrNameMaxLen] = {
    "FR_OK",                  /* (0) Succeeded */
    "FR_DISK_ERR",            /* (1) A hard error occurred in the low level disk I/O layer */
    "FR_INT_ERR",             /* (2) Assertion failed */
    "FR_NOT_READY",           /* (3) The physical drive cannot work */
    "FR_NO_FILE",             /* (4) Could not find the file */
    "FR_NO_PATH",             /* (5) Could not find the path */
    "FR_INVALID_NAME",        /* (6) The path name format is invalid */
    "FR_DENIED",              /* (7) Access denied due to prohibited access or directory full */
    "FR_EXIST",               /* (8) Access denied due to prohibited access */
    "FR_INVALID_OBJECT",      /* (9) The file/directory object is invalid */
    "FR_WRITE_PROTECTED",     /* (10) The physical drive is write protected */
    "FR_INVALID_DRIVE",       /* (11) The logical drive number is invalid */
    "FR_NOT_ENABLED",         /* (12) The volume has no work area */
    "FR_NO_FILESYSTEM",       /* (13) There is no valid FAT volume */
    "FR_MKFS_ABORTED",        /* (14) The f_mkfs() aborted due to any problem */
    "FR_TIMEOUT",             /* (15) Could not get a grant to access the volume within defined period */
    "FR_LOCKED",              /* (16) The operation is rejected according to the file sharing policy */
    "FR_NOT_ENOUGH_CORE",     /* (17) LFN working buffer could not be allocated */
    "FR_TOO_MANY_OPEN_FILES", /* (18) Number of open files > FF_FS_LOCK */
    "FR_INVALID_PARAMETER" /* (19) Given parameter is invalid */};

static const uint16_t kNumSDCards = 1;
static const uint16_t kNumSPIs = 1;

inline const char *FRESULT_str(FRESULT fr) { return kErrNames[fr]; }

void sd_card_spi_dma_isr();
void card_detect_callback(uint gpio, uint32_t events);

extern sd_card_t *pSD;
extern spi_t *pSPI;
