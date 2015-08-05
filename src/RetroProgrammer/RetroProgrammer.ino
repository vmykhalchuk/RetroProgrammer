#include "LoggerA.h"
#include "Utils.h"
#include "AVRConstants.h"
#include "AVRProgrammingRoutines.h"
#include "ConfFile.h"
#include "TargetProgramDetector.h"
#include "Tests.h"

#include <SPI.h>
#include <SD.h>

#define ONEWIRE_CRC8_TABLE 0
#include <OneWire.h>

const byte BUTTON_UPLOAD__LED_READY__LED_SUCCESS = A5;
const byte BUTTON_VERIFY__LED_AUTO__LED_FAIL = A6;
const byte BUTTON_BACKUP = 2;

// For Random read
//------------------
//    A6
const byte pinRandomRead = A6;

// For parsing the file
const byte LINE_TYPE_COMMENT = 0x01; // Comment
const byte LINE_TYPE_TYP     = 0x10; // Type ("AVR","PIC",...)
const byte LINE_TYPE_MDL     = 0x11; // Model (1,2,3,4,....)
const byte LINE_TYPE_SGN     = 0x12; // Signature (for AVR - 3 bytes)
const byte LINE_TYPE_LKB     = 0x20; // Lock Bits
const byte LINE_TYPE_FSB     = 0x21; // Fuse Bits
const byte LINE_TYPE_FHB     = 0x22; // Fuse High Bits
const byte LINE_TYPE_EFB     = 0x23; // Extended Fuse Bits
const byte LINE_TYPE_CLB     = 0x24; // Calibration Byte
const byte LINE_TYPE_PRM     = 0x30; // Flash/Program Memory Page
const byte LINE_TYPE_ERM     = 0x31; // EEPROM Page

const byte LINE_READ_BUFFER_SIZE = 128;

//                                    TARGET_MCU_ID__W0, TARGET_MCU_ID__W1, TARGET_MCU_ID__W2, TARGET_MCU_ID__MANUAL0, TARGET_MCU_ID__MANUAL1
TargetProgramDetector programDetector(       A0        ,       A1         ,      A2          ,       A3              ,         A4            );

byte programBuffer[1 << (AVR_MEM_PAGE_SIZE_64 + 1)];//128 bytes

#define RAM_DEPOSIT_SIZE 550
byte RAM_DEPOSIT[RAM_DEPOSIT_SIZE];

void setup() {
  #if RAM_DEPOSIT_SIZE > 0
    RAM_DEPOSIT[0]=0;
  #endif
  
  AVRProgrammingRoutines_setup();
  Logger_setup();
  TargetProgramDetector_setup();
  //initTargetMcuIdReaderPins();
  //serialInit();
  //lcdInit();
  logInfo("Hello World!");

  setup_test();
  //setup_real();
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
    if (!initConfFileSD()) {
      logError("SD for Conf file init failed!");
      return;
    }
    testConfFile();
  #endif

  logInfo("Happy testing day!");
  logFreeRam();
}

void setup_prod() {
  byte statusRes=0;
  
  delay(3000);
  byte b = waitForUserCommand();
  logInfoD("Command:",b);
  
  // Init SD Card
  return;
  //if (!initSDCard()) {
  //  logError("SD init failed!");
  //  return;
  //}

  // Prepare Target MCU for Programming
  startupTargetMcuProgramming(statusRes);
  if (statusRes > 0) {
    logError("ProgEn failed!");
    shutdownTargetMcu();
    return;
  } else {
    logInfo("Started prog!");
  }
  delay(2000);


  byte signBytes[3];
  #if 1
  // make sure it is ATmega328P
  readSignatureBytes(signBytes,statusRes);
  if (statusRes > 0) {
    logError("signature error!");
    return;
  }
  delay(2000);
  #endif

  #if 1
  logInfo("Uploading...");
  uploadMcuDataFromFile("TEST01", signBytes,
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
  testProgramming(statusRes);
  if (statusRes > 0) { logErrorB("Failed!", statusRes); return; }
  logInfo("Success testing!");
  delay(2000);
  #endif
  
  #if 1
  logInfo("Reading...");
  backupMcuDataToFile("RND" + String(analogRead(pinRandomRead),HEX), 
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
  backupMcuData("BACK", statusRes);
  if (statusRes > 0) {
    logError("Error backing-up!");
    return;
  }
  logError("Success backing-up!");
  #endif

  // now shutdown Target MCU
  shutdownTargetMcu();

  logInfo("Done!");
}

void testProgramming(byte& statusRes) {
  byte progMemPageSize = 6;
  byte progMemPagesCount = 8;
  byte targetPage = 2;
  
  #if 0
    byte buf[128];
    for (int i = 0; i < 128; i++) buf[i] = 0xFF;
    buf[0] = 0x01;
    buf[1] = 0x12;
    buf[2] = 0x23;
    buf[3] = 0x34;
    buf[4] = 0x45;
    buf[5] = 0x56;
    buf[125] = 0x20;
    buf[126] = 0x30;
    uploadProgramMemoryPage(buf,targetPage,progMemPagesCount,progMemPageSize,statusRes);
    if (statusRes > 0) {
      logErrorB("Page upoad failed!",targetPage);
      return;
    }
  #endif

  delay(100);
  loadProgramMemoryPageByte(false, 128, 0x12, statusRes); if (statusRes > 0) { logDebug("bad L0"); return; }
  loadProgramMemoryPageByte(true, 128, 0x01, statusRes); if (statusRes > 0) { logDebug("bad H0"); return; }
  loadProgramMemoryPageByte(false, 129, 0x34, statusRes); if (statusRes > 0) { logDebug("bad L1"); return; }
  loadProgramMemoryPageByte(true, 129, 0x23, statusRes); if (statusRes > 0) { logDebug("bad H1"); return; }
  loadProgramMemoryPageByte(false, 130, 0x56, statusRes); if (statusRes > 0) { logDebug("bad L2"); return; }
  loadProgramMemoryPageByte(true, 130, 0x45, statusRes); if (statusRes > 0) { logDebug("bad H2"); return; }

  loadProgramMemoryPageByte(false, 131, 0x68, statusRes); if (statusRes > 0) { logDebug("bad L6"); return; }
  loadProgramMemoryPageByte(true, 131, 0x67, statusRes); if (statusRes > 0) { logDebug("bad H6"); return; }
  writeProgramMemoryPage(0, 128, statusRes); if (statusRes > 0) { logDebug("bad Page"); return; }
  delay(500);

  readProgramMemoryPage(programBuffer, targetPage, progMemPageSize, statusRes);
  if (statusRes > 0) {
    logErrorB("Page read failed!",targetPage);
    return;
  }
  byte maxByte = 1 << (progMemPageSize + 1);
  for (byte i = 0; i < maxByte; i++) {
    logDebugB("",programBuffer[i]);
  }
  logDebug("Done!");
  
  // now lets try manual!
  byte bh = readProgramMemoryByte(true, 0, 128, statusRes); if (statusRes > 0) { logErrorB("read bh 128 F!",targetPage); return; }
  byte bl = readProgramMemoryByte(false, 0, 128, statusRes); if (statusRes > 0) { logErrorB("read bh 128 F!",targetPage); return; }
  logDebugB("bh128:",bh);
  logDebugB("bl128:",bl);
  bh = readProgramMemoryByte(true, 0, 129, statusRes); if (statusRes > 0) { logErrorB("read bh 129 F!",targetPage); return; }
  bl = readProgramMemoryByte(false, 0, 129, statusRes); if (statusRes > 0) { logErrorB("read bh 129 F!",targetPage); return; }
  logDebugB("bh129:",bh);
  logDebugB("bl129:",bl);

}

void openFile(File& f, String fileName, int mode, byte& statusRes) {
  initStatus();
  if (fileName.length() > 8) {
    logError("2long F name");// File Name too long
    returnStatus(0x20);
  }
  //char buf[13];
  fileName = String(fileName + ".HRP");//.toCharArray(buf,13);
  boolean fileExists = SD.exists(fileName);
  if (mode == FILE_WRITE && fileExists) {
    logError("File exists!");// File already exists
    returnStatus(0x20);
  }
  if (mode == FILE_READ && !fileExists) {
    logError("No file!");// No such file!
    returnStatus(0x20);
  }
  f = SD.open(fileName, mode);
  if (!f) {
    logError("OpenFailed!"); //Cannot open file for write:
    returnStatus(0x20);
  }
}

void openFile2(File& f, String fileName, int mode, byte& statusRes) {
  initStatus();
  //char buf[13];
  //fileName.toCharArray(buf,13);
  boolean fileExists = SD.exists(fileName);//buf);
  if (mode == FILE_WRITE && fileExists) {
    logError("File exists!");// File already exists
    returnStatus(0x20);
  }
  if (mode == FILE_READ && !fileExists) {
    logError("No file!");// No such file!
    returnStatus(0x20);
  }
  f = SD.open(fileName, mode);//buf, mode);
  if (!f) {
    logError("OpenFailed!"); //Cannot open file for write:
    returnStatus(0x20);
  } else {
    logInfoS("Opened file:", fileName);
  }
}

void findNextFileName(String filePref, String& resFile, byte& statusRes) {
  initStatus();
  // Create file
  if (filePref.length() > 4) {
    returnStatus(0x20);
  }
  //char buf[13];
  String fileName;
  boolean isF = false;
  for (int n = 0; n < 10000; n++) {
    fileName = String(filePref + String(n / 1000) + String(n / 100 % 10) + String(n / 10 % 10) + String(n % 10) + ".BKP");
    //fileName.toCharArray(buf,13);
    boolean fe = SD.exists(fileName);
    if (!fe) {
      isF = true;
      break;
    }
  }
  if (!isF) {
    logError("Clean backups!");
    returnStatus(0x20);
  }
}

void backupMcuData(String filePref, byte& statusRes) {
  String fileName;
  findNextFileName(filePref, fileName, statusRes);
  checkStatus();

  byte progMemPageSize, progMemPagesCount, eepromMemPageSize, eepromMemPagesCount;
  byte signBytes[3];
  readSignatureBytes(signBytes, statusRes); checkStatus();
  byte modelId = getAVRModelAndConf(signBytes, progMemPageSize, progMemPagesCount, eepromMemPageSize, eepromMemPagesCount, statusRes); checkStatus();
  backupMcuDataToFile(fileName, progMemPageSize, progMemPagesCount, eepromMemPageSize, eepromMemPagesCount, statusRes);
}

void backupMcuDataToFile(String fileName, byte progMemPageSize, byte progMemPagesCount, 
                        byte eepromMemPageSize, byte eepromMemPagesCount, byte& statusRes) {
  File f;
  byte t;
  openFile2(f, fileName, FILE_WRITE, statusRes);
  checkStatus();
  logDebug("File opened");
  
  // start file
  f.println(F("# AUTOGENERATED FILE"));
  
  f.println(F("TYP:AVR"));

  // get device signature
  byte signBytes[3];
  readSignatureBytes(signBytes, statusRes);
  if (statusRes != 0) {
    logErrorB("Sig read failed!",statusRes);
    goto f_close;
  }
  // get device model ID
  t = getAVRModelIdBySignature(signBytes, statusRes);
  if (statusRes != 0) {
    logErrorB("Model read failed!",t);
    goto f_close;
  }
  fPrintBln(f,F("MDL:"),t); // print Model
  f.print(F("SGN:"));       // print Signature
  fPrintB(f,signBytes[0]);
  fPrintB(f,signBytes[1]);
  fPrintB(f,signBytes[2]);
  f.println();
  
  // save LOCK bits
  t = readLockBits(statusRes);
  if (statusRes != 0) {
    logErrorB("LOCKBITS failed!",t);
    goto f_close;
  }
  fPrintBln(f, F("LKB:"),t);
  
  // save FUSE bits
  t = readFuseBits(statusRes);
  if (statusRes != 0) {
    logErrorB("FUSEBITS failed!",t);
    goto f_close;
  }
  fPrintBln(f, F("FSB:"),t);
  
  // save FUSE HIGH bits
  t = readFuseHighBits(statusRes);
  if (statusRes != 0) {
    logErrorB("FUSEHIGHBITS failed!",t);
    goto f_close;
  }
  fPrintBln(f, F("FHB:"),t);

  // save EXTENDED FUSE bits
  t = readExtendedFuseBits(statusRes);
  if (statusRes != 0) {
    logErrorB("EXTFUSEBITS failed!",t);
    goto f_close;
  }
  fPrintBln(f, F("EFB:"),t);

  // save Calibration Byte
  t = readCalibrationByte(statusRes);
  if (statusRes != 0) {
    logErrorB("CLBRTN failed!",t);
    goto f_close;
  }
  fPrintBln(f, F("CLB:"),t);

  // write random
  f.println(F("#"));
  f.println(F("# RANDOM GOES BELOW"));
  f.println(F("#"));
  for (int r = 0; r < 10; r++) {
    f.print(F("#"));
    for (byte i = 0; i < 64; i++) {
      fPrintB(f,analogRead(pinRandomRead));
    }
    f.println();
  }
  
  // write programm memory
  f.println(F("#"));
  f.println(F("# PROGRAMM MEMORY"));
  f.print(F("# pages count: "));
  f.println(1 << progMemPagesCount);
  f.print(F("# page size: "));
  f.println(1 << progMemPageSize);
  for (int p = 0; p < (1 << progMemPagesCount); p++) {
    logInfoD("PMPage: ",p);
    readProgramMemoryPage(programBuffer, p, progMemPageSize, statusRes);
    if (statusRes != 0) {
      logErrorB("Page read failed!",t);
      goto f_close;
    }
    
    f.print(F("PRM:"));
    fPrint3Dig(f, p);
    f.print(F(":"));
    
    byte maxByte = 1 << (progMemPageSize + 1);
    for (byte i = 0; i < maxByte; i++) {
      fPrintB(f, programBuffer[i]);
    }
    
    f.println();
  }
  f.println(F("#"));

  // write EEPROM memory
  f.println(F("#"));
  f.println(F("# EEPROM MEMORY"));
  f.print(F("# pages count: "));
  f.println(1 << eepromMemPagesCount);
  f.print(F("# page size: "));
  f.println(1 << eepromMemPageSize);
  byte eepromBuffer[1 << eepromMemPageSize];
  for (int p = 0; p < (1 << eepromMemPagesCount); p++) {
    logInfoD("EEPROMPage: ",p);
    readEepromMemoryPage(eepromBuffer, p, eepromMemPageSize, statusRes);
    if (statusRes != 0) {
      logErrorB("Page read failed!",t);
      goto f_close;
    }
    
    f.print(F("ERM:"));
    fPrint3Dig(f, p);
    f.print(F(":"));
    
    byte maxByte = 1 << eepromMemPageSize;
    for (byte i = 0; i < maxByte; i++) {
      fPrintB(f, eepromBuffer[i]);
    }
    
    f.println();
  }
    
  f_close: f.close();
}

// Print 3 digits
void fPrint3Dig(File& f, byte b) {
  f.print(b / 100);
  f.print((b / 10) % 10);
  f.print(b % 10);
}

//  Print String + Byte + EOL
void fPrintBln(File& f, String str, byte b) {
  f.print(str);
  fPrintB(f, b);
  f.println();
}

// Print Byte
void fPrintB(File& f, byte b) {
  f.print(String((b & 0xF0)>>4,HEX));
  f.print(String(b & 0x0F,HEX));
}


/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
///         READING FROM SD CARD 
///         AND UPLOADING TO MCU
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

void uploadMcuDataFromFile(String fileName, byte* targetMcuSign, byte progMemPagesCount, byte progMemPageSize,
                        byte eepromMemPagesCount, byte eepromMemPageSize, byte& statusRes) {
  uploadMcuDataFromFile_internal(false, fileName, targetMcuSign, 
                                  progMemPagesCount, progMemPageSize,
                                  eepromMemPagesCount, eepromMemPageSize, statusRes);
  if (statusRes > 0) {
    logError("File corrupted!");
    return;
  }
  
  uploadMcuDataFromFile_internal(true, fileName, targetMcuSign, 
                                  progMemPagesCount, progMemPageSize,
                                  eepromMemPagesCount, eepromMemPageSize, statusRes);
  if (statusRes > 0) {
    logError("Programming failed!");
    return;
  }
}

void uploadMcuDataFromFile(String fileName, byte targetMcuModel, byte& statusRes) {
  uploadMcuDataFromFile_internal(false, fileName, targetMcuModel, statusRes);
  if (statusRes > 0) {
    logError("File corrupted!");
    return;
  }

  uploadMcuDataFromFile_internal(true, fileName, targetMcuModel, statusRes);
  if (statusRes > 0) {
    logError("Programming failed!");
    return;
  }
}

void uploadMcuDataFromFile_internal(boolean progMode, String fileName, byte targetMcuModel, byte& statusRes) {
  byte m = targetMcuModel - 1;
  byte signBytes[3];
  signBytes[0] = MCU_AVR_TYPES[m][0];
  signBytes[1] = MCU_AVR_TYPES[m][1];
  signBytes[2] = MCU_AVR_TYPES[m][2];
  byte progMemPageSize = MCU_AVR_CONFIGS[m][0], progMemPagesCount = MCU_AVR_CONFIGS[m][1];
  byte eepromMemPageSize = MCU_AVR_CONFIGS[m][2], eepromMemPagesCount = MCU_AVR_CONFIGS[m][3];
  uploadMcuDataFromFile_internal(progMode, fileName, signBytes, 
            progMemPagesCount, progMemPageSize, eepromMemPagesCount, eepromMemPageSize, statusRes);
}

// targetMcuSign - byte[3] with sign of MCU e.g. {0x1e,0x95,0x0f}
void uploadMcuDataFromFile_internal(boolean progMode, String fileName, byte* targetMcuSign,
                        byte progMemPagesCount, byte progMemPageSize,
                        byte eepromMemPagesCount, byte eepromMemPageSize, byte& statusRes) {
  File f;
  openFile(f, fileName, FILE_READ, statusRes);
  checkStatus();

  
  byte lkb,fsb,fhb,efb;
  boolean isSign=false,isLkb=false,isFsb=false,isFhb=false,isEfb=false;
  
  boolean isSignValid = false;
  boolean startedProgramming = false;
  
  byte buf[LINE_READ_BUFFER_SIZE];
  byte lineType;
  int resSize, pageNo;
  while(f.available()) {
    readLine(f,lineType,buf,resSize,pageNo,statusRes);
    if (statusRes != 0) goto f_close;
    logDebugB("Line type: ", lineType);
    logDebugD("Res size: ", resSize);
    logDebugD("PageNo: ", pageNo);
    logDebugB("buf[0]: ", buf[0]);
    logDebugB("buf[1]: ", buf[1]);
    
    // verify buffer result
    if (lineType == LINE_TYPE_PRM) {
      if (resSize != (1 << (progMemPageSize + 1))) {
        logDebugD("resSize bad:",resSize);
        statusRes = 0x30;
        goto f_close;
      }
      if (pageNo >= (1 << progMemPagesCount)) {
        logDebugD("pageNo bad:", pageNo);
        statusRes = 0x30;
        goto f_close;
      }
    } else if (lineType == LINE_TYPE_ERM) {
      if (resSize != (1 << eepromMemPageSize)) {
        logDebugD("EEPROM resSize bad:", resSize);
        statusRes = 0x30;
        goto f_close;
      }
      if (pageNo >= (1 << eepromMemPagesCount)) {
        logDebugD("EEPROM pageNo bad:", pageNo);
        statusRes = 0x30;
        goto f_close;
      }
    }
    
    // TODO Implement lineType: ERS - if is set to true - will Erase whole chip before programming!
    if (lineType == LINE_TYPE_PRM) {
      startedProgramming = true;
      if (!isSignValid) goto f_error;
      if (progMode && (pageNo == 240 || pageNo == 230)) {
        logInfoD("PRM Page!",pageNo);
        uploadProgramMemoryPage(buf, pageNo, progMemPagesCount, progMemPageSize, statusRes);
        if (statusRes != 0) return;
      }
    } else if (lineType == LINE_TYPE_ERM) {
      startedProgramming = true;
      if (!isSignValid) goto f_error;
      if (progMode) {
        logInfoD("ERM Page!",pageNo);
      }
    } else if (lineType == LINE_TYPE_SGN) {
      if (isSign || startedProgramming) goto f_error;
      isSign = true;
      isSignValid = true;
      for (byte i = 0; i < 3; i++) isSignValid = (targetMcuSign[i] == buf[i]) && isSignValid;
    } else if (lineType == LINE_TYPE_LKB) {
      if (isLkb || startedProgramming) goto f_error;
      isLkb = true;
      lkb = buf[0];
    } else if (lineType == LINE_TYPE_FSB) {
      if (isFsb || startedProgramming) goto f_error;
      isFsb = true;
      fsb = buf[0];
    } else if (lineType == LINE_TYPE_FHB) {
      if (isFhb || startedProgramming) goto f_error;
      isFhb = true;
      fhb = buf[0];
    } else if (lineType == LINE_TYPE_EFB) {
      if (isEfb || startedProgramming) goto f_error;
      isEfb = true;
      efb = buf[0];
    }
  }
  
  // now lets programm fuses and lock bits
  if (!isSignValid) goto f_error; // before programming fuses we must check signature
  if (isFsb && progMode) {
    logInfoB("FSB!",fsb);
  }
  if (isFhb && progMode) {
    logInfoB("FHB!",fhb);
  }
  if (isEfb && progMode) {
    logInfoB("EFB!",efb);
  }
  if (isLkb && progMode) { // lock bits should be programmed last!
    logInfoB("LKB!",lkb);
  }
  
  waitForTargetMCU(statusRes); // wait for last operation to finish
  
  f_close: f.close(); return;
  f_error: statusRes = 0x30; goto f_close;
}

void uploadProgramMemoryPage(byte* buf, int pageNo, 
      byte progMemPagesCount, byte progMemPageSize, byte& statusRes) {
  if (pageNo < 0) { statusRes = 0x20; return; }
  int addr = pageNo << progMemPageSize;
  byte addrMsb = addr >> 8;
  byte addrLsb = addr & 0xFF;
  
  boolean allFF = true;
  for (byte i = 0; i < (1 << (progMemPageSize + 1)); i++) {
    if (buf[i] != 0xFF) { allFF = false; break; }
  }
  if (allFF) return; // nothing to programm, all FF
  
  // load into buffer
  waitForTargetMCU(statusRes); if (statusRes > 0) return;
  for (byte i = 0; i < (1 << progMemPageSize); i++) {
  //for (byte i = (1 << progMemPageSize) - 1; i >= 0; i--) {
    //logDebug("i="+String(i)+":"+String(buf[i * 2],HEX)+":"+String(buf[i * 2 + 1],HEX));
    loadProgramMemoryPageByte(true, i, buf[i * 2], statusRes);
    if (statusRes > 0) return;
    loadProgramMemoryPageByte(false, i, buf[i * 2 + 1], statusRes);
    if (statusRes > 0) return;
  }
  // programm
  writeProgramMemoryPage(addrMsb, addrLsb, statusRes);
}

void waitForTargetMCU(byte& statusRes) {
  do {
    if (!isTargetMcuBusy(statusRes)) return;
    delay(1);
  } while(statusRes == 0);
}

// lineType:
//    0x00 - empty
//    0x01 - comment
//    0x10 - MCU Type - buffer will contain chars with type (resSize will contain size of result)
//    0x12 - Signature (for AVR, buffer will contain first 3 bytes) as signature
//    
// #    - 0x01  Comment
// TYP  - 0x10  Type ("AVR","PIC",...)
// MDL  - 0x11  Model (1,2,3,4,....)
// SGN  - 0x12  Signature (for AVR - 3 bytes)
// LKB  - 0x20  Lock Bits
// FSB  - 0x21  Fuse Bits
// FHB  - 0x22  Fuse Hight Bits
// EFB  - 0x23  Extended Fuse Bits
// CLB  - 0x24  Calibration Byte
// PRM  - 0x30  Flash/Program Memory Page
// ERM  - 0x31  EEPROM Page
const byte line_types[10][4] = {{'T','Y','P',LINE_TYPE_TYP}, {'M','D','L',LINE_TYPE_MDL}, {'S','G','N',LINE_TYPE_SGN},
                                {'L','K','B',LINE_TYPE_LKB}, {'F','S','B',LINE_TYPE_FSB}, {'F','H','B',LINE_TYPE_FHB},
                                {'E','F','B',LINE_TYPE_EFB}, {'C','L','B',LINE_TYPE_CLB},
                                {'P','R','M',LINE_TYPE_PRM}, {'E','R','M',LINE_TYPE_ERM}};

void readLine(File& f, byte& lineType, byte* buffer, int& resSize, int& pageNo, byte& statusRes) {
  byte c[4];
  logDebug("readLine");
  if (!readChar(f,c[0])) { statusRes = 0x30; return; }
  logDebugB("read c[0]:", c[0]);
  if (c[0] == '#') {
    lineType = LINE_TYPE_COMMENT;
    int chars = readToTheEOL(f,statusRes);
    logDebugS("readToEOL:", String(chars) + "," + String(statusRes,HEX));
    return;
  }
  if (!readChar(f,c[1])) { statusRes = 0x30; return; }
  logDebugB("read c[1]:", c[1]);
  if (!readChar(f,c[2])) { statusRes = 0x30; return; }
  logDebugB("read c[2]:", c[2]);
  if (!readChar(f,c[3])) { statusRes = 0x30; return; }
  logDebugB("read c[3]:", c[3]);
  if (c[3] != ':') {
    statusRes = 0x32;// Not ':'
    return;
  }
  logDebug("read 3 char");
  
  lineType = 0xFF;
  for (byte i = 0; i < 10; i++) {
    boolean matched = true;
    for (byte j = 0; j < 3; j++) {
      if (c[j] != line_types[i][j]) {
        matched = false;
        break;
      }
    }
    if (matched) {
      lineType = line_types[i][3];
      break;
    }
  }
  logDebugB("lineType:", lineType);
  if (lineType == 0xFF) {
    logWarn("Bad line");
    statusRes = 0x32;// No match of valid line type found!
    return;
  }
  
  if (lineType == LINE_TYPE_TYP) {
    readToTheEOL(f,statusRes);
    return;
    // TODO Implement verification of string: it should be "AVR" or "PIC" for PICs
  } else if (lineType == LINE_TYPE_MDL) {
    readToTheEOL(f,statusRes);
    return;
    // TODO Implement verification of string: it should be "AVR" or "PIC" for PICs
  } else if (lineType == LINE_TYPE_SGN) {
    resSize = 3;
    buffer[0] = readHexByte(f,statusRes); if (statusRes > 0) return;
    //logDebugB("byte0:", buffer[0]);

    buffer[1] = readHexByte(f,statusRes); if (statusRes > 0) return;
    //logDebugB("byte1:", buffer[1]);

    buffer[2] = readHexByte(f,statusRes); if (statusRes > 0) return;
    //logDebugB("byte2:", buffer[2]);

  } else if (lineType == LINE_TYPE_LKB || lineType == LINE_TYPE_FSB || 
             lineType == LINE_TYPE_FHB || lineType == LINE_TYPE_EFB || lineType == LINE_TYPE_CLB) {
    resSize = 1;
    buffer[0] = readHexByte(f,statusRes); if (statusRes > 0) return;

  } else if (lineType == LINE_TYPE_PRM || lineType == LINE_TYPE_ERM) {

    pageNo = read3DigByte(f,statusRes); if (statusRes > 0) return;
    //logDebugD("pageNo:", pageNo);

    byte cs;
    if (!readChar(f,cs)) { statusRes = 0x30; return; }
    //logDebugB("read cs:", cs);
    if (cs != ':') { statusRes = 0x32; return; } // No ':'
  
    resSize = 0;
    do {
      boolean isEOL = false;
      byte b = readHexByteOrEOL(f,isEOL,statusRes);
      if (statusRes > 0) return;
      if (isEOL) return; // its end of line, we should not continue from this place, otherwise line will get corrupted!
      
      if (resSize == LINE_READ_BUFFER_SIZE) {
        logDebug("TOO LONG!!!!");
        statusRes = 0x31; return; // too long line - buffer cannot hold that amount!
      }
      buffer[resSize++] = b;
    } while(true);
  }

  int z = readToTheEOL(f,statusRes);
  statusRes = z > 0 ? 0x30 : statusRes;
}

byte read3DigByte(File& f, byte& statusRes) {
  byte b = readHexByte(f,statusRes);
  if (statusRes > 0) return 0;
  byte c1,c2,c3;
  c1 = b >> 4;
  c2 = b & 0x0F;
  
  if (!readChar(f,c3)) { statusRes = 0x30; return 0; }
  c3 = convertHexCharToByte(c3,statusRes); if (statusRes > 0) return 0;
  
  if (c1 > 9 || c2 > 9 || c3 > 9) { statusRes = 0x33; return 0; }
  int r = c1 * 100 + c2 * 10 + c3;
  if (r > 255) { statusRes = 0x33; return 0; }
  return r;
}

byte readHexByte(File& f, byte& statusRes) {
  boolean isEOL;
  byte b = readHexByteOrEOL(f,isEOL,statusRes);
  if (isEOL) {
    statusRes = 0x30;
  }
  return b;
}

// remember to check isEOL before statusRes!!! When EOL, statusRes is also an error!
byte readHexByteOrEOL(File& f, boolean& isEOL, byte& statusRes) {
  statusRes = 0;
  byte c1,c2;
  if (!readChar(f,c1)) { statusRes = 0x30; return 0; }
    //logDebugB("readHexByte_1:", c1);
  if (!readChar(f,c2)) { statusRes = 0x30; return 0; }
    //logDebugB("readHexByte_2:", c2);
  if (c1 <= 0x0D || c2 <= 0x0D) {
    if (c1 == 0x0D && c2 == 0x0A) {
      logDebug("readHexByte_EOL");
      isEOL = true;
    } else {
      logDebug("readHexByte_EOL_corrupted");
      statusRes = 0x30;
    }
    return 0;
  }
  c1 = convertHexCharToByte(c1,statusRes);
    //logDebugB("_SR:", statusRes);
  if (statusRes > 0) return 0;
    //logDebug("readHexByte_convert1_success");
  c2 = convertHexCharToByte(c2,statusRes);
  if (statusRes > 0) return 0;
  return (c1 << 4) | c2;
}

boolean readChar(File& f, byte& c) {
  if (f.available()) {
    c = f.read();
    return true;
  } else {
    return false;// unexpected end
  }
}

int readToTheEOL(File& f, byte& statusRes) {
  int readChars = 0;
  while (f.available()) {
    byte c = f.read();
    if (c == 0x0A) {
      statusRes = 0x30;
      return readChars;
    }
    if (c == 0x0D) {
      if (f.available()) c = f.read();
      statusRes = (c == 0x0A) ? 0 : 0x30;
      return readChars;
    }
    readChars++;
  }
  statusRes = 0x30;// unexpected end
}

/////////// get actual directory name for selected programm

String getDirectoryName(byte* data, byte idType, byte& statusRes) {
  if (idType == 1) {
    
  }
}

/////////// COMMAND BUTTONS AND LEDS

const int BUTTON_UPLOAD_VERIFY_TRESHOLD = 512; // when button pressed, voltage level drops below middle value, closer to 0. Note: 1023 - 5v
// return 1 - UPLOAD; 2 - VERIFY; 3 - BACKUP
byte waitForUserCommand() {
  pinMode(BUTTON_UPLOAD__LED_READY__LED_SUCCESS, INPUT);
  pinMode(BUTTON_VERIFY__LED_AUTO__LED_FAIL, INPUT);
  pinMode(BUTTON_BACKUP, INPUT);
  delayMicroseconds(5);
  if (analogRead(BUTTON_UPLOAD__LED_READY__LED_SUCCESS) < BUTTON_UPLOAD_VERIFY_TRESHOLD) {
    return 1;
  }
  if (analogRead(BUTTON_VERIFY__LED_AUTO__LED_FAIL) < BUTTON_UPLOAD_VERIFY_TRESHOLD) {
    return 2;
  }
  if (!digitalRead(BUTTON_BACKUP)) {
    return 3;
  }
}


/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
///    LOOP
///    LOOP
///    LOOP
///    LOOP
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

void loop()
{
}
