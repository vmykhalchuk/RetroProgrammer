#ifndef AVRConstants_h
#define AVRConstants_h

#include <Arduino.h>

// AVR MCU Models
const byte MCU_AVR_ATmega48A   = 1;
const byte MCU_AVR_ATmega48PA  = 2;
const byte MCU_AVR_ATmega88A   = 3;
const byte MCU_AVR_ATmega88PA  = 4;
const byte MCU_AVR_ATmega168A  = 5;
const byte MCU_AVR_ATmega168PA = 6;
const byte MCU_AVR_ATmega328   = 7;
const byte MCU_AVR_ATmega328P  = 8;

const byte MCU_AVR_ATtiny25   = 9;
const byte MCU_AVR_ATtiny45   = 10;
const byte MCU_AVR_ATtiny85   = 11;

const byte MCU_AVR_TYPES_COUNT = 11;

const byte MCU_AVR_TYPES [MCU_AVR_TYPES_COUNT][3] = {
          {0x1E,0x92,0x05},
          {0x1E,0x92,0x0A},
          {0x1E,0x93,0x0A},
          {0x1E,0x93,0x0F},
          {0x1E,0x94,0x06},
          {0x1E,0x94,0x0B},
          {0x1E,0x95,0x14},
          {0x1E,0x95,0x0F},

          {0x1E,0x91,0x08},
          {0x1E,0x92,0x06},
          {0x1E,0x93,0x0B}
};

const byte AVR_MEM_PAGE_SIZE_4   = 2;
//const byte AVR_MEM_PAGE_SIZE_8   = 3;
const byte AVR_MEM_PAGE_SIZE_16  = 4;
const byte AVR_MEM_PAGE_SIZE_32  = 5;
const byte AVR_MEM_PAGE_SIZE_64  = 6;

const byte AVR_MEM_PAGES_COUNT_32  = 5;
const byte AVR_MEM_PAGES_COUNT_64  = 6;
const byte AVR_MEM_PAGES_COUNT_128 = 7;
const byte AVR_MEM_PAGES_COUNT_256 = 8;

const byte MCU_AVR_CONFIGS [MCU_AVR_TYPES_COUNT] [4] = {
          //    MCU model        ,     PROG(FLASH)     ,      PROG(FLASH)       ,        EEPROM       ,         EEPROM
          //                     ,  PAGE SIZE X WORDS  ,       PAGES COUNT      ,  PAGE SIZE X BYTES  ,       PAGES COUNT
          /*MCU_AVR_ATmega48A   */{AVR_MEM_PAGE_SIZE_32, AVR_MEM_PAGES_COUNT_64 , AVR_MEM_PAGE_SIZE_4 , AVR_MEM_PAGES_COUNT_64 },
          /*MCU_AVR_ATmega48PA  */{AVR_MEM_PAGE_SIZE_32, AVR_MEM_PAGES_COUNT_64 , AVR_MEM_PAGE_SIZE_4 , AVR_MEM_PAGES_COUNT_64 },
          /*MCU_AVR_ATmega88A   */{AVR_MEM_PAGE_SIZE_32, AVR_MEM_PAGES_COUNT_128, AVR_MEM_PAGE_SIZE_4 , AVR_MEM_PAGES_COUNT_128},
          /*MCU_AVR_ATmega88PA  */{AVR_MEM_PAGE_SIZE_32, AVR_MEM_PAGES_COUNT_128, AVR_MEM_PAGE_SIZE_4 , AVR_MEM_PAGES_COUNT_128},
          /*MCU_AVR_ATmega168A  */{AVR_MEM_PAGE_SIZE_64, AVR_MEM_PAGES_COUNT_128, AVR_MEM_PAGE_SIZE_4 , AVR_MEM_PAGES_COUNT_128},
          /*MCU_AVR_ATmega168PA */{AVR_MEM_PAGE_SIZE_64, AVR_MEM_PAGES_COUNT_128, AVR_MEM_PAGE_SIZE_4 , AVR_MEM_PAGES_COUNT_128},
          /*MCU_AVR_ATmega328   */{AVR_MEM_PAGE_SIZE_64, AVR_MEM_PAGES_COUNT_256, AVR_MEM_PAGE_SIZE_4 , AVR_MEM_PAGES_COUNT_256},
          /*MCU_AVR_ATmega328P  */{AVR_MEM_PAGE_SIZE_64, AVR_MEM_PAGES_COUNT_256, AVR_MEM_PAGE_SIZE_4 , AVR_MEM_PAGES_COUNT_256},

          /*MCU_AVR_ATtiny25    */{AVR_MEM_PAGE_SIZE_16, AVR_MEM_PAGES_COUNT_64 , AVR_MEM_PAGE_SIZE_4 , AVR_MEM_PAGES_COUNT_32},
          /*MCU_AVR_ATtiny45    */{AVR_MEM_PAGE_SIZE_32, AVR_MEM_PAGES_COUNT_64 , AVR_MEM_PAGE_SIZE_4 , AVR_MEM_PAGES_COUNT_64},
          /*MCU_AVR_ATtiny85    */{AVR_MEM_PAGE_SIZE_32, AVR_MEM_PAGES_COUNT_128, AVR_MEM_PAGE_SIZE_4 , AVR_MEM_PAGES_COUNT_128}
};

#endif
