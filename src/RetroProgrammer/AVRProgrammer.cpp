#include "AVRProgrammer.h"

// ERR() List:
// 0x10 - System error - wrong parameters, etc
// 0x17 - Target Programming failure - Wrong state
// 0x13 -       >>>>
// 0x14 -       >>>>
// 0x11 - Target Programming failure - communication failure
// 0x12 -       >>>>
// 0x15 -       >>>>
// 0x16 -       >>>>

void AVRProgrammer::__translateErrorsToDisplayErrorCode(byte err, byte& mainErrCode, byte& subErrCode, byte& okCode) {
  mainErrCode = 0;
  subErrCode = 0;
  okCode = 0;
  if (err == 0x10) {
    mainErrCode = 0xA; subErrCode = 0x3;
  } else if (err == 0x17 || err == 0x13 || err == 0x14) {
    mainErrCode = 0x8;
  } else if (err == 0x11 || err == 0x12 || err == 0x15 || err == 0x16) {
    mainErrCode = 0x8;
  } else {
    mainErrCode = 0xA;
  }
}

/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
///    AVR PROGRAMMING ROUTINES 
///    AVR PROGRAMMING ROUTINES 
///    AVR PROGRAMMING ROUTINES 
///    AVR PROGRAMMING ROUTINES 
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

boolean AVRProgrammer::targetMcuProgMode = false; // If true - Target MCU is in Programming mode
boolean AVRProgrammer::targetMcuOutOfSync = false; // If true - Target MCU Programming mode is out of sync due to communication error, etc

void AVRProgrammer::setup() {
  pinMode(pinVccEnable, OUTPUT);
  pinMode(pinAVccEnable, OUTPUT);
  digitalWrite(pinVccEnable, LOW);
  digitalWrite(pinAVccEnable, LOW);

  pinMode(pinMosi, OUTPUT);
  pinMode(pinMiso, INPUT);
  pinMode(pinSck, OUTPUT);
  pinMode(pinReset, OUTPUT);
  digitalWrite(pinMosi, LOW);
  //digitalWrite(pinMiso, HIGH);// enable pull-up on MISO line
  digitalWrite(pinSck, LOW);
  digitalWrite(pinReset, HIGH);
}

void AVRProgrammer::shutdownTargetMcu() {
  digitalWrite(pinReset, HIGH);
  digitalWrite(pinMosi, LOW);
  digitalWrite(pinSck, LOW);

  digitalWrite(pinVccEnable, LOW);
  digitalWrite(pinAVccEnable, LOW);
  targetMcuProgMode = false;
  targetMcuOutOfSync = false;
}

void AVRProgrammer::startupTargetMcuProgramming(byte& statusRes) {
  initStatus();
  if (targetMcuProgMode) {
    logError("MCU In PMODE!");
    returnStatus(ERR(0x17));
  }
  
  digitalWrite(pinVccEnable, HIGH);
  digitalWrite(pinAVccEnable, HIGH);
  
  // special case (when we cannot operate with Vcc and Avcc pins of target mcu)
  digitalWrite(pinReset, HIGH);
  delay(3);
  digitalWrite(pinReset, LOW);
  
  delay(30); // minimum 20ms is required by spec
  
  byte byte1 = sendReadByte(0xAC);
  byte byte2 = sendReadByte(0x53);
  byte byte3 = sendReadByte(0x00);
  byte byte4 = sendReadByte(0x00);

  targetMcuProgMode = true;
  targetMcuOutOfSync = (byte3 != 0x53 || byte4 != 0x00);
  if (targetMcuOutOfSync) {
    returnStatus(ERR(0x11));
  }
}

void AVRProgrammer::issueByteWriteCmd(byte b1, byte b2, byte b3, byte b4, byte& statusRes) {
  byte r = issueByteReadCmd4(b1, b2, b3, b4, statusRes);
  if (r != b3) {
    targetMcuOutOfSync = true;
    logErrorB("OutOfSync b4:",b3);// Out of sync on 4th byte
    returnStatus(ERR(0x12));
  }
}

byte AVRProgrammer::issueByteReadCmd4(byte b1, byte b2, byte b3, byte b4, byte& statusRes) {
  initStatus();
  if (!targetMcuProgMode) {
    logError("MCU !ProgMode");
    returnStatusV(ERR(0x13), 0);
  }
  if (targetMcuOutOfSync) {
    logError("MCU OutOfSync!");
    returnStatusV(ERR(0x14), 0);
  }
  
  // Send Byte #1
  sendReadByte(b1);
  
  // Send Byte #2 and check result with Byte #1
  if (sendReadByte(b2) != b1) {
    targetMcuOutOfSync = true;
    logErrorB(("OutOfSync b2:"),b1);// Out of sync on 2nd byte
    returnStatusV(ERR(0x15), 0);
  }

  // Send Byte #3 and check result with Byte #2
  if (sendReadByte(b3) != b2) {
    targetMcuOutOfSync = true;
    logErrorB(("OutOfSync b3:"),b2);// Out of sync on 3rd byte
    returnStatusV(ERR(0x16), 0);
  }

  // Send Byte #4 and return result
  return sendReadByte(b4);
}

boolean AVRProgrammer::isTargetMcuBusy(byte& statusRes) {
  return (issueByteReadCmd2(0xF0,0x00,statusRes) & B00000001) != 0;
}

void AVRProgrammer::waitForTargetMCU(byte& statusRes) {
  do {
    if (!isTargetMcuBusy(statusRes)) return;
    delay(1);
  } while(statusRes == 0);
}

void AVRProgrammer::readSignatureBytes(byte* signBytes, byte& statusRes) {
  signBytes[0] = issueByteReadCmd3(0x30,0x00,0,statusRes);
  checkStatus();
  signBytes[1] = issueByteReadCmd3(0x30,0x00,1,statusRes);
  checkStatus();
  signBytes[2] = issueByteReadCmd3(0x30,0x00,2,statusRes);
}

void AVRProgrammer::readProgramMemoryPage(byte* pageBuffer, byte pageNumber, byte pageSize, byte& statusRes) {
  if (pageSize < AVR_MEM_PAGE_SIZE_16 || pageSize > AVR_MEM_PAGE_SIZE_64) {
    logDebugD("readProgramMemoryPage:pageSize:", pageSize);
    returnStatus(ERR(0x10));
  }
  byte addrHigh = pageNumber >> (8 - pageSize);
  byte addrLowPage = pageNumber << pageSize;
  byte maxPageAddr = 1 << pageSize;
  for (int i = 0; i < maxPageAddr; i++) {
    byte addrLow = addrLowPage | i;
    logDebugB("addrHigh:", addrHigh);
    logDebugB("addrLow:", addrLow);
    pageBuffer[i * 2] = readProgramMemoryByte(false, addrHigh, addrLow, statusRes);
    checkStatus();
    pageBuffer[i * 2 + 1] = readProgramMemoryByte(true, addrHigh, addrLow, statusRes);
    checkStatus();
  }
}

void AVRProgrammer::readEepromMemoryPage(byte* pageBuffer, byte pageNumber, byte pageSize, byte& statusRes) {
  if (pageSize != AVR_MEM_PAGE_SIZE_4) {
    logDebugD("readEepromMemoryPage:pageSize:", pageSize);
    returnStatus(ERR(0x10));
  }
  byte addrHigh = pageNumber >> (8 - pageSize);
  byte addrLowPage = pageNumber << pageSize;
  byte maxPageAddr = 1 << pageSize;
  for (int i = 0; i < maxPageAddr; i++) {
    byte addrLow = addrLowPage | i;
    pageBuffer[i] = readEepromMemoryByte(addrHigh, addrLow, statusRes);
    checkStatus();
  }
}

/*boolean loadProgramMemoryPageWord(byte adrLSB, int w) {
  int lowB = w & 0xFF;
  int highB = (w & 0xFF00) >> 8;
  if (!loadProgramMemoryPageByte(false, adrLSB, lowB)) {
    return false;
  }
  if (!loadProgramMemoryPageByte(true, adrLSB, highB)) {
    return false;
  }
  return true;
}*/

byte AVRProgrammer::sendReadByte(byte byteToSend) {
  byte resByte = 0;
  byte mask = B10000000;

  byte outBit;
  int inBit;

  for (byte i = 0; i < 8; i++) {
    outBit = byteToSend & mask;
    if (outBit == 0) {
      digitalWrite(pinMosi, LOW);
    } else {
      digitalWrite(pinMosi, HIGH);
    }
    delayMicroseconds(2);
    digitalWrite(pinSck, HIGH);
    delayMicroseconds(6);
    inBit = digitalRead(pinMiso);
    if (inBit != 0) {
      resByte = resByte | mask;
    }
    delayMicroseconds(2);
    digitalWrite(pinSck, LOW);
    delayMicroseconds(6);
    
    mask = mask >> 1;
  }
  
  return resByte;
}

void AVRProgrammer::loadAndWriteProgramMemoryPage(byte* buf, int bufSize, int pageNo, byte mcuModelId, byte& statusRes) {
  if (pageNo < 0) {
    logErrorD("wrongPageNo:", pageNo);
    returnStatus(ERR(0x10));
  }
  if (mcuModelId < 1 || mcuModelId > MCU_AVR_DATA_LENGTH) {
    logErrorD("wrongMcuModelId:", mcuModelId);
    returnStatus(ERR(0x10));
  }

  byte progMemPageSize   = MCU_AVR_DATA[mcuModelId-1][0];
  byte progMemPagesCount = MCU_AVR_DATA[mcuModelId-1][1];
  byte expectedBufSize = (1 << (progMemPageSize + 1));
  if (bufSize != expectedBufSize) {
    logErrorD("!bufSize:", bufSize);
    logInfoD("expected:", expectedBufSize);
    returnStatus(ERR(0x10));
  }

  /* (should only run when Full erase was performed before!)
  // Check is all FF, then no need to program this buffer
  boolean allFF = true;
  for (int i = 0; i < expectedBufSize; i++) {
    if (buf[i] != 0xFF) { allFF = false; break; }
  }
  if (allFF) return; // nothing to programm, all FF
  */
  
  int addr = pageNo << progMemPageSize;
  byte addrMsb = addr >> 8;
  byte addrLsb = addr & 0xFF;
  //logDebugD("addr:",addr);
  //logDebugD("addrMsb:",addrMsb);
  //logDebugD("addrLsb:",addrLsb);
  
  // load into buffer
  waitForTargetMCU(statusRes); checkStatus();
  for (int i = 0; i < (1 << progMemPageSize); i++) {
    byte addrLow = addrLsb | i;
    //logDebugB("i:", i);
    //logDebugB("addrLow:", addrLow);
    loadProgramMemoryPageByte(false, i, buf[i * 2], statusRes); checkStatus();
    loadProgramMemoryPageByte(true, i, buf[i * 2 + 1], statusRes); checkStatus();
  }
  // programm
  writeProgramMemoryPage(addrMsb, addrLsb, statusRes); checkStatus();
}

/**
  addr - 0..3
*/
void AVRProgrammer::loadEEPROMMemoryPageByte(byte addr, byte b, byte& statusRes) { 
  if (addr > 3) {
    logDebug("loadEEPROMMemoryPageByte, addr > 3");
    returnStatus(ERR(0x10));// system error - addr too high
  }
  issueByteWriteCmd(0xC1, 0x00, addr, b, statusRes); 
}

/**
  addrMsb - 0000 00aa
  addrLsb - aaaa aa00
*/
void AVRProgrammer::writeEEPROMMemoryPage(byte addrMsb, byte addrLsb, byte& statusRes) {
  if (addrMsb & B00000011 > 0) {
    logDebug("addrMsb is wrong");
    returnStatus(ERR(0x10)); // system error, wrong addrMsb
  }
  if (addrLsb & B11111100 > 0) {
    logDebug("addrLsb is wrong");
    returnStatus(ERR(0x10)); // system error, wrong addrLsb
  }
  issueByteWriteCmd(0xC2, addrMsb, addrLsb, 0x00, statusRes);
}

