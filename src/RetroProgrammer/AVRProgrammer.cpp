#include "AVRProgrammer.h"

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
    returnStatus(0x10);
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
    returnStatus(0x11);
  }
}

void AVRProgrammer::issueByteWriteCmd(byte b1, byte b2, byte b3, byte b4, byte& statusRes) {
  byte r = issueByteReadCmd4(b1, b2, b3, b4, statusRes);
  if (r != b3) {
    targetMcuOutOfSync = true;
    logErrorB("OutOfSync b4:",b3);// Out of sync on 4th byte
    returnStatus(0x12);
  }
}

byte AVRProgrammer::issueByteReadCmd4(byte b1, byte b2, byte b3, byte b4, byte& statusRes) {
  initStatus();
  if (!targetMcuProgMode) {
    logError("MCU !ProgMode");
    returnStatusV(0x13, 0);
  }
  if (targetMcuOutOfSync) {
    logError("MCU OutOfSync!");
    returnStatusV(0x14, 0);
  }
  
  // Send Byte #1
  sendReadByte(b1);
  
  // Send Byte #2 and check result with Byte #1
  if (sendReadByte(b2) != b1) {
    targetMcuOutOfSync = true;
    logErrorB(("OutOfSync b2:"),b1);// Out of sync on 2nd byte
    returnStatusV(0x15, 0);
  }

  // Send Byte #3 and check result with Byte #2
  if (sendReadByte(b3) != b2) {
    targetMcuOutOfSync = true;
    logErrorB(("OutOfSync b3:"),b2);// Out of sync on 3rd byte
    returnStatusV(0x16, 0);
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
    returnStatus(0x19);
  }
  byte addrHigh = pageNumber >> (8 - pageSize);
  byte addrLowPage = pageNumber << pageSize;
  byte maxPageAddr = 1 << pageSize;
  for (byte i = 0; i < maxPageAddr; i++) {
    byte addrLow = addrLowPage | i;
    pageBuffer[i * 2] = readProgramMemoryByte(true, addrHigh, addrLow, statusRes);
    checkStatus();
    pageBuffer[i * 2 + 1] = readProgramMemoryByte(false, addrHigh, addrLow, statusRes);
    checkStatus();
  }
}

void AVRProgrammer::readEepromMemoryPage(byte* pageBuffer, byte pageNumber, byte pageSize, byte& statusRes) {
  if (pageSize != AVR_MEM_PAGE_SIZE_4) {
    returnStatus(0x1A);
  }
  byte addrHigh = pageNumber >> (8 - pageSize);
  byte addrLowPage = pageNumber << pageSize;
  byte maxPageAddr = 1 << pageSize;
  for (byte i = 0; i < maxPageAddr; i++) {
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

