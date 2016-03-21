#include "ProgramFile.h"

// ERR() List:
// 0x50 - Unknown system error
// 0x51 - Cannot open Program File (either for open or write)
// 0x52 - Cannot open backup file for write - most likely it is SD card problem
// 0x53 - Cannot find next backup file - not enough file names
// 0x54 - Corrupted Program File - wrong format

void ProgramFile::__translateErrorsToDisplayErrorCode(byte err, byte& mainErrCode, byte& subErrCode, byte& okCode) {
  mainErrCode = 0;
  subErrCode = 0;
  okCode = 0;
  if (err == 0x50) {
    mainErrCode = 0xA; subErrCode = 0x2;
  } else if (err == 0x51) {
    mainErrCode = 0x2; subErrCode = 0x7;
  } else if (err == 0x52) {
    mainErrCode = 0x2; subErrCode = 0x6;
  } else if (err == 0x53) {
    mainErrCode = 0x2; subErrCode = 0x4;
  } else if (err == 0x54) {
    mainErrCode = 0x2; subErrCode = 0x3;
  } else {
    mainErrCode = 0xA;
    AVRProgrammer::__translateErrorsToDisplayErrorCode(err, mainErrCode, subErrCode, okCode);
    //FIXME call Utils after AVRProgrammer handlers!
  }
}

byte ProgramFile::programBuffer[1 << (AVR_MEM_PAGE_SIZE_64 + 1)];//128 bytes


  ////////////////////////////////////////////////
  // MAIN/PUBLIC FUNCTIONS
  ////////////////////////////////////////////////

void ProgramFile::openFile2(File& f, String fileName, int mode, byte& statusRes) {
  initStatus();
  f = SD.open(fileName, FILE_READ);
  if (mode == FILE_WRITE) {
    if (!f) {
      logDebugS("File exists! override protection!:", fileName);
      returnStatus(ERR(0x50));//system error, calling program must check if file exists
    }
    f = SD.open(fileName, FILE_WRITE);
  }
  if (!f) {
    logDebugS("!open:",fileName);
    returnStatus(ERR(0x51)); //Cannot open ProgramFile
  } else {
    logInfoS("Opened file:", fileName);
  }
}

File ProgramFile::backupFile;

void ProgramFile::findAndOpenNextFileBackupFile(String filePref, byte& statusRes) {
  initStatus();
  // Create file
  if (filePref.length() > 4) {
    logDebug("Wrong file pref used");
    returnStatus(ERR(0x50)); // system error, wrong filePref used
  }
  //char buf[13];
  String fileName;
  boolean isF = false;
  int minAvNo = 10000;
  for (int n = 300; n > 0; n--) {//maximum 300 files allowed, for faster search (possible up to 9999)
    fileName = String(filePref + String(n / 1000) + String(n / 100 % 10) + String(n / 10 % 10) + String(n % 10) + ".BKP");
    logDebugS("checking:", fileName);
    backupFile = SD.open(fileName, FILE_READ);
    if (backupFile) {
      logDebugD("found",n);
      break;
    }
    minAvNo = n;
  }
  if (minAvNo == 10000) {
    returnStatus(ERR(0x53)); // not enough space for backup files
  }
  
  fileName = String(filePref + String(minAvNo / 1000) + String(minAvNo / 100 % 10) + String(minAvNo / 10 % 10) + String(minAvNo % 10) + ".BKP");
  logDebugS("Next free file:", fileName);
  backupFile = SD.open(fileName, FILE_WRITE);
  if (!backupFile) {
    logDebug("!writeFile");
    returnStatus(ERR(0x52)); // cannot open backup file for write
  }
}

void ProgramFile::backupMcuData(String filePref, const char* programId, byte& statusRes) {
  findAndOpenNextFileBackupFile(filePref, statusRes); checkStatus();

  byte progMemPageSize, progMemPagesCount, eepromMemPageSize, eepromMemPagesCount;
  byte signBytes[3];
  AVRProgrammer::readSignatureBytes(signBytes, statusRes); checkStatus();
  byte modelId = UtilsAVR::getAVRModelAndConf(signBytes, progMemPageSize, progMemPagesCount, eepromMemPageSize, eepromMemPagesCount, statusRes); checkStatus();
  backupMcuDataToFile(programId, progMemPageSize, progMemPagesCount, eepromMemPageSize, eepromMemPagesCount, statusRes);
}

void ProgramFile::backupMcuDataToFile(const char* programId, byte progMemPageSize, byte progMemPagesCount, 
                        byte eepromMemPageSize, byte eepromMemPagesCount, byte& statusRes) {
  byte t;
  
  // start file
  backupFile.println(F("# AUTOGENERATED FILE"));

  backupFile.print(F("TID:")); backupFile.println(programId);
  
  backupFile.println(F("TYP:AVR"));

  // get device signature
  byte signBytes[3];
  AVRProgrammer::readSignatureBytes(signBytes, statusRes);
  if (statusRes != 0) {
    logErrorB("Sig read failed!",statusRes);
    goto f_close;
  }
  // get device model ID
  t = UtilsAVR::getAVRModelIdBySignature(signBytes, statusRes);
  if (statusRes != 0) {
    logErrorB("Model read failed!",t);
    goto f_close;
  }
  UtilsSD::fPrintBln(backupFile,F("MDL:"),t); // print Model
  backupFile.print(F("SGN:"));       // print Signature
  UtilsSD::fPrintB(backupFile,signBytes[0]);
  UtilsSD::fPrintB(backupFile,signBytes[1]);
  UtilsSD::fPrintB(backupFile,signBytes[2]);
  backupFile.println();
  
  // save LOCK bits
  t = AVRProgrammer::readLockBits(statusRes);
  if (statusRes != 0) {
    logErrorB("LOCKBITS failed!",t);
    goto f_close;
  }
  UtilsSD::fPrintBln(backupFile, F("LKB:"),t);
  
  // save FUSE bits
  t = AVRProgrammer::readFuseBits(statusRes);
  if (statusRes != 0) {
    logErrorB("FUSEBITS failed!",t);
    goto f_close;
  }
  UtilsSD::fPrintBln(backupFile, F("FSB:"),t);
  
  // save FUSE HIGH bits
  t = AVRProgrammer::readFuseHighBits(statusRes);
  if (statusRes != 0) {
    logErrorB("FUSEHIGHBITS failed!",t);
    goto f_close;
  }
  UtilsSD::fPrintBln(backupFile, F("FHB:"),t);

  // save EXTENDED FUSE bits
  t = AVRProgrammer::readExtendedFuseBits(statusRes);
  if (statusRes != 0) {
    logErrorB("EXTFUSEBITS failed!",t);
    goto f_close;
  }
  UtilsSD::fPrintBln(backupFile, F("EFB:"),t);

  // save Calibration Byte
  t = AVRProgrammer::readCalibrationByte(statusRes);
  if (statusRes != 0) {
    logErrorB("CLBRTN failed!",t);
    goto f_close;
  }
  UtilsSD::fPrintBln(backupFile, F("CLB:"),t);

  // write random
  /*backupFile.println(F("#"));
  backupFile.println(F("# RANDOM GOES BELOW"));
  backupFile.println(F("#"));
  for (int r = 0; r < 10; r++) {
    backupFile.print(F("#"));
    for (byte i = 0; i < 64; i++) {
      UtilsSD::fPrintB(backupFile,analogRead(pinRandomRead));
    }
    backupFile.println();
  }*/
  
  // write programm memory
  backupFile.println(F("#"));
  backupFile.println(F("# PROGRAMM MEMORY"));
  backupFile.print(F("# pages count: "));
  backupFile.println(1 << progMemPagesCount);
  backupFile.print(F("# page size: "));
  backupFile.println(1 << progMemPageSize);
  for (int p = 0; p < (1 << progMemPagesCount); p++) {
    logInfoD("PMPage: ",p);
    AVRProgrammer::readProgramMemoryPage(programBuffer, p, progMemPageSize, statusRes);
    if (statusRes != 0) {
      logErrorB("Page read failed!",t);
      goto f_close;
    }
    
    backupFile.print(F("PRM:"));
    UtilsSD::fPrint3Dig(backupFile, p);
    backupFile.print(F(":"));
    
    byte maxByte = 1 << (progMemPageSize + 1);
    for (byte i = 0; i < maxByte; i++) {
      UtilsSD::fPrintB(backupFile, programBuffer[i]);
    }
    
    backupFile.println();
  }
  backupFile.println(F("#"));

  // write EEPROM memory
  backupFile.println(F("#"));
  backupFile.println(F("# EEPROM MEMORY"));
  backupFile.print(F("# pages count: "));
  backupFile.println(1 << eepromMemPagesCount);
  backupFile.print(F("# page size: "));
  backupFile.println(1 << eepromMemPageSize);
  byte eepromBuffer[1 << eepromMemPageSize];
  for (int p = 0; p < (1 << eepromMemPagesCount); p++) {
    logInfoD("EEPROMPage: ",p);
    AVRProgrammer::readEepromMemoryPage(eepromBuffer, p, eepromMemPageSize, statusRes);
    if (statusRes != 0) {
      logErrorB("Page read failed!",t);
      goto f_close;
    }
    
    backupFile.print(F("ERM:"));
    UtilsSD::fPrint3Dig(backupFile, p);
    backupFile.print(F(":"));
    
    byte maxByte = 1 << eepromMemPageSize;
    for (byte i = 0; i < maxByte; i++) {
      UtilsSD::fPrintB(backupFile, eepromBuffer[i]);
    }
    
    backupFile.println();
  }
    
  f_close: backupFile.close();
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


/*void ProgramFile::uploadMcuDataFromFile(String fileName, byte* targetMcuSign, byte progMemPagesCount, byte progMemPageSize,
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
}*/

void ProgramFile::uploadMcuDataFromFile(String fileName, byte targetMcuModelId, byte& statusRes) {
  uploadMcuDataFromFile_internal(false, fileName, targetMcuModelId, statusRes);
  if (statusRes > 0) {
    logError("File corrupted!");
    return;
  }

  uploadMcuDataFromFile_internal(true, fileName, targetMcuModelId, statusRes);
  if (statusRes > 0) {
    logError("Programming failed!");
    return;
  }
}

void ProgramFile::uploadMcuDataFromFile_internal(boolean progMode, String fileName, byte targetMcuModelId, byte& statusRes) {
  if (targetMcuModelId < 1 || targetMcuModelId > MCU_AVR_DATA_LENGTH) {
    logDebugD("modelId bad:", targetMcuModelId);
    returnStatus(ERR(0x50));
  }
  byte m = targetMcuModelId - 1;
  byte signBytes[3];
  signBytes[0] = MCU_AVR_DATA[m][4];
  signBytes[1] = MCU_AVR_DATA[m][5];
  signBytes[2] = MCU_AVR_DATA[m][6];
  byte progMemPageSize = MCU_AVR_DATA[m][0], progMemPagesCount = MCU_AVR_DATA[m][1];
  byte eepromMemPageSize = MCU_AVR_DATA[m][2], eepromMemPagesCount = MCU_AVR_DATA[m][3];
  uploadMcuDataFromFile_internal(progMode, fileName, targetMcuModelId, signBytes, 
            progMemPagesCount, progMemPageSize, eepromMemPagesCount, eepromMemPageSize, statusRes);
}

// targetMcuSign - byte[3] with sign of MCU e.g. {0x1e,0x95,0x0f}
void ProgramFile::uploadMcuDataFromFile_internal(boolean progMode, String fileName, byte targetMcuModelId, byte* targetMcuSign,
                        byte progMemPagesCount, byte progMemPageSize,
                        byte eepromMemPagesCount, byte eepromMemPageSize, byte& statusRes) {
  File f;
  openFile2(f, fileName, FILE_READ, statusRes);
  checkOverrideStatus(ERR(0x51));
  
  byte lkb,fsb,fhb,efb;
  boolean isSign=false,isLkb=false,isFsb=false,isFhb=false,isEfb=false; // flag represents if any of this value is already loaded from ProgramFile
  
  boolean isSignValid = false; // if true - then signature is loaded from file and matches targetMcuSign provided for this method
  boolean startedProgramming = false; // if programming already started
  
  byte buf[LINE_READ_BUFFER_SIZE];
  byte lineType;
  int resSize, pageNo;
  while(f.available()) {
    resSize = pageNo = 0;
    readLine(f,lineType,buf,resSize,pageNo,statusRes);
    checkOverrideStatus(ERR(0x54)); // FIXME it is supposed to perform "goto f_close;" not return!
    logDebugB("Line type: ", lineType);
    logDebugD("Res size: ", resSize);
    logDebugD("PageNo: ", pageNo);
    logDebugB("buf[0]: ", buf[0]);
    logDebugB("buf[1]: ", buf[1]);
    
    // verify buffer result
    if (lineType == LINE_TYPE_PRM) {
      if (resSize != (1 << (progMemPageSize + 1))) {
        logDebugD("resSize bad:",resSize);
        returnStatus(ERR(0x54));
      }
      if (pageNo >= (1 << progMemPagesCount)) {
        logDebugD("pageNo bad:", pageNo);
        returnStatus(ERR(0x54));
      }
    } else if (lineType == LINE_TYPE_ERM) {
      if (resSize != (1 << eepromMemPageSize)) {
        logDebugD("EEPROM resSize bad:", resSize);
        returnStatus(ERR(0x54));
      }
      if (pageNo >= (1 << eepromMemPagesCount)) {
        logDebugD("EEPROM pageNo bad:", pageNo);
        returnStatus(ERR(0x54));
      }
    }
    
    // TODO Implement lineType: ERS - if is set to true - will Erase whole chip before programming!
    if (lineType == LINE_TYPE_PRM) {
      startedProgramming = true;
      if (!isSignValid) { logDebug("!isSignValid_1"); returnStatus(ERR(0x54)); }
      if (progMode && (pageNo < 5)) {//FIXME something wrong here, it must work for all pages
        logInfoD("PRM Page!",pageNo);
        AVRProgrammer::loadAndWriteProgramMemoryPage(buf, resSize, pageNo, targetMcuModelId, statusRes);
        checkStatus();
      }
    } else if (lineType == LINE_TYPE_ERM) {
      startedProgramming = true;
      if (!isSignValid) { logDebug("!isSignValid_2"); returnStatus(ERR(0x54)); }
      if (progMode) {
        logInfoD("ERM Page!",pageNo);
      }
    } else if (lineType == LINE_TYPE_SGN) {
      if (isSign || startedProgramming) { logDebug("isSign||startedProg"); returnStatus(ERR(0x54)); }
      isSign = true;
      isSignValid = true;
      for (byte i = 0; i < 3; i++) isSignValid = (targetMcuSign[i] == buf[i]) && isSignValid;
    } else if (lineType == LINE_TYPE_LKB) {
      if (isLkb || startedProgramming) { logDebug("isLkb||startedProg"); returnStatus(ERR(0x54)); }
      isLkb = true;
      lkb = buf[0];
    } else if (lineType == LINE_TYPE_FSB) {
      if (isFsb || startedProgramming) { logDebug("isLkb||startedProg"); returnStatus(ERR(0x54)); }
      isFsb = true;
      fsb = buf[0];
    } else if (lineType == LINE_TYPE_FHB) {
      if (isFhb || startedProgramming) { logDebug("isFhb||startedProg"); returnStatus(ERR(0x54)); }
      isFhb = true;
      fhb = buf[0];
    } else if (lineType == LINE_TYPE_EFB) {
      if (isEfb || startedProgramming) { logDebug("isEfb||startedProg"); returnStatus(ERR(0x54)); }
      isEfb = true;
      efb = buf[0];
    }
  }
  
  // now lets programm fuses and lock bits
  if (!isSignValid) { logDebug("!isSignValid_3"); returnStatus(ERR(0x54)); }; // before programming fuses we must check signature
  if (isFsb && progMode) {
    logInfoB("FSB!",fsb);
    // TODO Implement this!
  }
  if (isFhb && progMode) {
    logInfoB("FHB!",fhb);
    // TODO Implement this!
  }
  if (isEfb && progMode) {
    logInfoB("EFB!",efb);
    // TODO Implement this!
  }
  if (isLkb && progMode) { // lock bits should be programmed last!
    logInfoB("LKB!",lkb);
    // TODO Implement this!
  }
  
  AVRProgrammer::waitForTargetMCU(statusRes); // wait for last operation to finish
  
  f_close: f.close(); return;
  //f_error: statusRes = ERR(0x50); goto f_close;
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
      const byte ProgramFile::line_types[11][4] = 
            {{'T','Y','P',ProgramFile::LINE_TYPE_TYP}, {'M','D','L',ProgramFile::LINE_TYPE_MDL}, {'S','G','N',ProgramFile::LINE_TYPE_SGN},
             {'L','K','B',ProgramFile::LINE_TYPE_LKB}, {'F','S','B',ProgramFile::LINE_TYPE_FSB}, {'F','H','B',ProgramFile::LINE_TYPE_FHB},
             {'E','F','B',ProgramFile::LINE_TYPE_EFB}, {'C','L','B',ProgramFile::LINE_TYPE_CLB},
             {'P','R','M',ProgramFile::LINE_TYPE_PRM}, {'E','R','M',ProgramFile::LINE_TYPE_ERM},
             {'T','I','D',ProgramFile::LINE_TYPE_TID}};

void ProgramFile::readLine(File& f, byte& lineType, byte* buffer, int& resSize, int& pageNo, byte& statusRes) {
  byte c[4];
  logDebug("readLine");
  if (!UtilsSD::readChar(f,c[0])) { logDebug("rl0"); returnStatus(ERR(0x54)); }
  logDebugC("read c[0]:", c[0]);
  if (c[0] == '#') {
    lineType = LINE_TYPE_COMMENT;
    int chars = UtilsSD::readToTheEOL(f,statusRes);
    logDebugS("readToEOL:", String(chars) + "," + String(statusRes,HEX));
    return;
  }
  if (!UtilsSD::readChar(f,c[1])) returnStatus(ERR(0x54));
  logDebugC("read c[1]:", c[1]);
  if (!UtilsSD::readChar(f,c[2])) returnStatus(ERR(0x54));
  logDebugC("read c[2]:", c[2]);
  if (!UtilsSD::readChar(f,c[3])) returnStatus(ERR(0x54));
  logDebugC("read c[3]:", c[3]);
  if (c[3] != ':') returnStatus(ERR(0x54));
  logDebug("read 3 char");
  
  lineType = 0xFF;
  for (byte i = 0; i < 11; i++) {
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
    logWarn("Bad line"); // No match of valid line type found!
    returnStatus(ERR(0x54));
  }
  
  if (lineType == LINE_TYPE_TID) {
    UtilsSD::readToTheEOL(f,statusRes);
    return;
  } else if (lineType == LINE_TYPE_TYP) {
    UtilsSD::readToTheEOL(f,statusRes);
    return;
    // TODO Implement verification of string: it should be "AVR" or "PIC" for PICs
  } else if (lineType == LINE_TYPE_MDL) {
    UtilsSD::readToTheEOL(f,statusRes);
    return;
    // TODO Implement verification of string: it should be "AVR" or "PIC" for PICs
  } else if (lineType == LINE_TYPE_SGN) {
    resSize = 3;
    buffer[0] = UtilsSD::readHexByte(f,statusRes); checkStatus();
    logDebugB("byte0:", buffer[0]);

    buffer[1] = UtilsSD::readHexByte(f,statusRes); checkStatus();
    logDebugB("byte1:", buffer[1]);

    buffer[2] = UtilsSD::readHexByte(f,statusRes); checkStatus();
    logDebugB("byte2:", buffer[2]);

  } else if (lineType == LINE_TYPE_LKB || lineType == LINE_TYPE_FSB || 
             lineType == LINE_TYPE_FHB || lineType == LINE_TYPE_EFB || lineType == LINE_TYPE_CLB) {
    resSize = 1;
    buffer[0] = UtilsSD::readHexByte(f,statusRes); checkStatus();

  } else if (lineType == LINE_TYPE_PRM || lineType == LINE_TYPE_ERM) {

    pageNo = UtilsSD::read3DigByte(f,statusRes); checkStatus();
    //logDebugD("pageNo:", pageNo);

    byte cs;
    if (!UtilsSD::readChar(f,cs)) returnStatus(ERR(0x54));
    //logDebugB("read cs:", cs);
    if (cs != ':') returnStatus(ERR(0x54)); // No ':'
  
    resSize = 0;
    do {
      boolean isEOL = false;
      byte b = UtilsSD::readHexByteOrEOL(f,isEOL,statusRes);
      if (statusRes > 0) return;
      if (isEOL) return; // its end of line, we should not continue from this place, otherwise line will get corrupted!
      
      if (resSize == LINE_READ_BUFFER_SIZE) {
        // too long line - buffer cannot hold that amount!
        logDebug("TOO LONG!!!!");
        returnStatus(ERR(0x54));
      }
      buffer[resSize++] = b;
    } while(true);
  }

  int z = UtilsSD::readToTheEOL(f,statusRes); checkStatus();
  if (z != 0) {
    returnStatus(ERR(0x54));
  }
}


/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
///         TESTS OF THIS
///         FUNCTIONALITY
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////


void ProgramFile_TestStub::_testProgramming(byte& statusRes) {
  byte progMemPageSize = 6;
  byte progMemPagesCount = 8;
  byte targetPage = 2;
  
  delay(100);
  AVRProgrammer::loadProgramMemoryPageByte(false, 128, 0x12, statusRes); if (statusRes > 0) { logDebug("bad L0"); return; }
  AVRProgrammer::loadProgramMemoryPageByte(true, 128, 0x01, statusRes); if (statusRes > 0) { logDebug("bad H0"); return; }
  AVRProgrammer::loadProgramMemoryPageByte(false, 129, 0x34, statusRes); if (statusRes > 0) { logDebug("bad L1"); return; }
  AVRProgrammer::loadProgramMemoryPageByte(true, 129, 0x23, statusRes); if (statusRes > 0) { logDebug("bad H1"); return; }
  AVRProgrammer::loadProgramMemoryPageByte(false, 130, 0x56, statusRes); if (statusRes > 0) { logDebug("bad L2"); return; }
  AVRProgrammer::loadProgramMemoryPageByte(true, 130, 0x45, statusRes); if (statusRes > 0) { logDebug("bad H2"); return; }

  AVRProgrammer::loadProgramMemoryPageByte(false, 131, 0x68, statusRes); if (statusRes > 0) { logDebug("bad L6"); return; }
  AVRProgrammer::loadProgramMemoryPageByte(true, 131, 0x67, statusRes); if (statusRes > 0) { logDebug("bad H6"); return; }
  AVRProgrammer::writeProgramMemoryPage(0, 128, statusRes); if (statusRes > 0) { logDebug("bad Page"); return; }
  delay(500);

  AVRProgrammer::readProgramMemoryPage(ProgramFile::programBuffer, targetPage, progMemPageSize, statusRes);
  if (statusRes > 0) {
    logErrorB("Page read failed!",targetPage);
    return;
  }
  byte maxByte = 1 << (progMemPageSize + 1);
  for (byte i = 0; i < maxByte; i++) {
    logDebugB("",ProgramFile::programBuffer[i]);
  }
  logDebug("Done!");
  
  // now lets try manual!
  byte bh = AVRProgrammer_TestStub::_testReadProgramMemoryByte(true, 0, 128, statusRes); if (statusRes > 0) { logErrorB("read bh 128 F!",targetPage); return; }
  byte bl = AVRProgrammer_TestStub::_testReadProgramMemoryByte(false, 0, 128, statusRes); if (statusRes > 0) { logErrorB("read bh 128 F!",targetPage); return; }
  logDebugB("bh128:",bh);
  logDebugB("bl128:",bl);
  bh = AVRProgrammer_TestStub::_testReadProgramMemoryByte(true, 0, 129, statusRes); if (statusRes > 0) { logErrorB("read bh 129 F!",targetPage); return; }
  bl = AVRProgrammer_TestStub::_testReadProgramMemoryByte(false, 0, 129, statusRes); if (statusRes > 0) { logErrorB("read bh 129 F!",targetPage); return; }
  logDebugB("bh129:",bh);
  logDebugB("bl129:",bl);
}

