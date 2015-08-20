#include "LoggerA.h"
#include "Utils.h"
#include "AVRConstants.h"
#include "AVRProgrammer.h"
#include "ConfFile.h"
#include "ProgramFile.h"
#include "TargetProgramDetector.h"
#include "HWInterface.h"
#include "Tests.h"

#include <SPI.h>
#include <SD.h>

#define ONEWIRE_CRC8_TABLE 0
#include <OneWire.h>

// For Random read
//------------------
//    A6
const byte pinRandomRead = A6;


#define RAM_DEPOSIT_SIZE 550
byte RAM_DEPOSIT[RAM_DEPOSIT_SIZE];

void setup() {
  #if RAM_DEPOSIT_SIZE > 0
    RAM_DEPOSIT[0]=0;
  #endif

  AVRProgrammer::setup();
  //                           TARGET_MCU_ID__W0, TARGET_MCU_ID__W1, TARGET_MCU_ID__W2, TARGET_MCU_ID__MANUAL0, TARGET_MCU_ID__MANUAL1
  TargetProgramDetector::setup(       A0        ,       A1         ,      A2          ,       A3              ,         A4            );
  
  Logger_setup();
  //initTargetMcuIdReaderPins();
  //serialInit();
  //lcdInit();
  logInfo("Hello World!");

  setup_test();
  setup_prod();
}

void setup_test() {//Used for testing
  logFreeRam();

  testUtilsGen();
  testUtilsAVR();
  
  #if 1
    testTargetProgramDetector();
  #endif

  #if 1 // Upoad test files to SD Card!!!
    // Init SD Card
    if (!initSDCard()) {
      logError("SD init failed!");
      return;
    }
    testConfFile();
  #endif

  logInfo("Happy testing day!");
  logFreeRam();
}

void setup_prod() {
  byte statusRes=0;
  
  delay(1000);
  byte b = waitForUserCommand();
  logInfoD("Command:",b);
  
  // Init SD Card
  if (!initSDCard()) {
    logError("SD init failed!");
    return;
  }

  // find out what program is selected
  char progIdBuf[PROG_ID_BUFFER_SIZE]; // 3 chars for type (ID_, R1_, R2_, MN_), 12 chars as a max length for ID - 6 bytes, 1 char '/0'
  boolean autoSelected;
  TargetProgramDetector::getProgId(progIdBuf, autoSelected, statusRes);
  if (statusRes > 0) {
    logError("getProgId");
    return;
  } else {
    logInfoS("ProgId:", progIdBuf);
  }
  delay(2000);
  

  // Prepare Target MCU for Programming
  AVRProgrammer::startupTargetMcuProgramming(statusRes);
  if (statusRes > 0) {
    logError("ProgEn failed!");
    AVRProgrammer::shutdownTargetMcu();
    return;
  } else {
    logInfo("Started prog!");
  }
  delay(2000);


  byte signBytes[3];
  #if 1
  // make sure it is ATmega328P
  AVRProgrammer::readSignatureBytes(signBytes,statusRes);
  if (statusRes > 0) {
    logError("signature error!");
    return;
  }
  delay(2000);
  #endif

  #if 1
  logInfo("Uploading...");
  ProgramFile::uploadMcuDataFromFile("TEST01", signBytes,
          AVR_MEM_PAGES_COUNT_256, AVR_MEM_PAGE_SIZE_64, 
          AVR_MEM_PAGES_COUNT_256, AVR_MEM_PAGE_SIZE_4, statusRes);
  if (statusRes > 0) {
    logError("Error uploading!");
    return;
  }
  logInfo("Success uploading!");
  delay(2000);
  #endif

  #if 1
  // test example of programming
  ProgramFile::_testProgramming(statusRes);
  if (statusRes > 0) { logErrorB("Failed!", statusRes); return; }
  logInfo("Success testing!");
  delay(2000);
  #endif
  
  #if 1
  logInfo("Reading...");
  ProgramFile::backupMcuDataToFile("RND" + String(analogRead(pinRandomRead),HEX), 
          AVR_MEM_PAGES_COUNT_256, AVR_MEM_PAGE_SIZE_64, 
          AVR_MEM_PAGES_COUNT_256, AVR_MEM_PAGE_SIZE_4, statusRes);
  if (statusRes > 0) {
    logError("Error reading!");
    return;
  }
  logInfo("Success reading!");
  delay(2000);
  #endif

  #if 1
  logInfo("Backup to file...");
  ProgramFile::backupMcuData("BACK", statusRes);
  if (statusRes > 0) {
    logError("Error backing-up!");
    return;
  }
  logError("Success backing-up!");
  #endif

  // now shutdown Target MCU
  AVRProgrammer::shutdownTargetMcu();

  logInfo("Done!");
}


boolean initSDCard() {
  // SD Card Module pins:
  // Arduino | Target
  //   Nano  |  MCU
  //------------------
  //    11   | MOSI
  //    12   | MISO
  //    13   | CLK
  //    4    | CS
  //    10   | is not used but always set to OUTPUT (read more in SD method implementation)

  logInfo("Init-g SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output
  // or the SD library functions will not work.
  pinMode(10, OUTPUT);

  if (!SD.begin(4)) {
    logError("SD init failed!");
    return false;
  }
  logInfo("SD init done");
  return true;
}

void loop()
{
}
