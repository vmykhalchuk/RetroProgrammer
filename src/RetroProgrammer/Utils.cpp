#include "Utils.h"

// ERR() List:
// 0x80 - System error
// 0x81 - wrong hex character
// 0x82 - wrong AVR signature - unknown
// 0x83 - AVR modelId is wrong - unknown
// 0x84 - AVR model name is wrong - corrupted
// 0x85 - AVR model name is wrong - modelId unknown (AVR@NNN, NNN is wrong)
// 0x86 - AVR model name is wrong - signBytes[0] data wrong (AVR[zzhhhh] zz is wrong)
// 0x87 - AVR model name is wrong - signBytes[1] data wrong (AVR[hhzzhh] zz is wrong)
// 0x88 - AVR model name is wrong - signBytes[2] data wrong (AVR[hhhhzz] zz is wrong)
// 0x89 - SD file read - 3digit read failed
// 0x8A - SD file read - hed byte(s) read failure
// 0x8B - SD file read - failure reading to EOL

void Utils::__translateErrorsToDisplayErrorCode(byte err, byte& mainErrCode, byte& subErrCode, byte& okCode) {
  mainErrCode = 0;
  subErrCode = 0;
  okCode = 0;
  if (err >= 0x80 && err <= 0x88) {
    mainErrCode = 0xA; subErrCode = 0x4;
  } else if (err >= 0x89 && err <= 0x8B) {
    mainErrCode = 0x2; subErrCode = 0xA;
  } else {
    mainErrCode = 0xA; subErrCode = 0x0;
  }
}

// freeRam: http://playground.arduino.cc/Code/AvailableMemory
int freeRam_1() {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
int freeRam() {
  int size = 800; // Use 2048 with ATmega328, was 1024
  byte *buf;
  while ((buf = (byte *) malloc(--size)) == NULL) ;
  free(buf);
  return size;
}

char convertByteToHexChar(byte b, boolean high4bits) {
  if (high4bits) {
    b = b >> 4;
  }
  b &= 0x0F;
  if (b < 10) {
    return ('0' + b);
  } else {
    return ('A' + (b - 10));
  }
};

byte convertHexCharToByte(byte c, byte& statusRes) {
  initStatus();
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'a' && c <= 'f') {
    return (c - 'a') + 10;
  }
  if (c >= 'A' && c <= 'F') {
    return (c - 'A') + 10;
  }
  logDebugB("Bad c:", c);
  returnStatusV(ERR(0x81), 0);
};

boolean isWhiteChar(char c) {
  return (c==' ')||(c=='\t');
}

boolean isValidNameChar(char c) {
  if (c >= '0' && c <= '9') {
    return true;
  }
  if (c >= 'a' && c <= 'z') {
    return true;
  }
  if (c >= 'A' && c <= 'Z') {
    return true;
  }
  if (c == '.' || c == '_' || c == '-') {
    return true;
  }
  return false;
}


  //////////////////////////
  //   AVR Related
  //////////////////////////


byte UtilsAVR::getAVRModelIdBySignature(byte* signBytes, byte& statusRes) {
  initStatus();
  for (byte i = 0; i < MCU_AVR_TYPES_COUNT; i++) {
    if (signBytes[0] == MCU_AVR_TYPES[i][0] &&
        signBytes[1] == MCU_AVR_TYPES[i][1] &&
        signBytes[2] == MCU_AVR_TYPES[i][2]) {
      return i + 1;
    }
  }
  logDebugB("signMissmatch:0",signBytes[0]);
  logDebugB("1",signBytes[1]);
  logDebugB("2",signBytes[2]);
  returnStatusV(ERR(0x82), 0);
}

byte UtilsAVR::getAVRModelAndConf(byte* signBytes, byte& flashPageSize, byte& flashPagesCount, byte& eepromPageSize, 
                                  byte& eepromPagesCount, byte& statusRes) {
  byte modelId = getAVRModelIdBySignature(signBytes, statusRes); checkStatusV(0);
  flashPageSize = MCU_AVR_CONFIGS[modelId - 1][0];
  flashPagesCount = MCU_AVR_CONFIGS[modelId - 1][1];
  eepromPageSize = MCU_AVR_CONFIGS[modelId - 1][2];
  eepromPagesCount = MCU_AVR_CONFIGS[modelId - 1][3];
  return modelId;
}

byte __getAVRModleName_pos;
inline void _aC(char* modelName, char c) {
  modelName[__getAVRModleName_pos++] = c;
}

void UtilsAVR::getAVRModelNameById(char* mn, byte modelId, byte& statusRes) {
  initStatus();
  __getAVRModleName_pos = 0;
  _aC(mn, 'A'); _aC(mn, 'T');
  if (modelId >= MCU_AVR_ATmega48A && modelId <= MCU_AVR_ATmega328P) {
    _aC(mn, 'm'); _aC(mn, 'e'); _aC(mn, 'g'); _aC(mn, 'a');
  } else {
    _aC(mn, 't'); _aC(mn, 'i'); _aC(mn, 'n'); _aC(mn, 'y');
  }
  
  if (modelId == MCU_AVR_ATmega48A) {
    _aC(mn, '4'); _aC(mn, '8'); _aC(mn, 'A');
  } else if (modelId == MCU_AVR_ATmega48PA) {
    _aC(mn, '4'); _aC(mn, '8'); _aC(mn, 'P'); _aC(mn, 'A');
  } else if (modelId == MCU_AVR_ATmega88A) {
    _aC(mn, '8'); _aC(mn, '8'); _aC(mn, 'A');
  } else if (modelId == MCU_AVR_ATmega88PA) {
    _aC(mn, '8'); _aC(mn, '8'); _aC(mn, 'P'); _aC(mn, 'A');
  } else if (modelId == MCU_AVR_ATmega168A) {
    _aC(mn, '1'); _aC(mn, '6'); _aC(mn, '8'); _aC(mn, 'A');
  } else if (modelId == MCU_AVR_ATmega168PA) {
    _aC(mn, '1'); _aC(mn, '6'); _aC(mn, '8'); _aC(mn, 'P'); _aC(mn, 'A');
  } else if (modelId == MCU_AVR_ATmega328) {
    _aC(mn, '3'); _aC(mn, '2'); _aC(mn, '8');
  } else if (modelId == MCU_AVR_ATmega328P) {
    _aC(mn, '3'); _aC(mn, '2'); _aC(mn, '8'); _aC(mn, 'P');
  } else if (modelId == MCU_AVR_ATtiny25) {
    _aC(mn, '2'); _aC(mn, '5');
  } else if (modelId == MCU_AVR_ATtiny45) {
    _aC(mn, '4'); _aC(mn, '5');
  } else if (modelId == MCU_AVR_ATtiny85) {
    _aC(mn, '8'); _aC(mn, '5');
  } else {
    mn[0] = '\0'; returnStatus(ERR(0x83)); // AVR modelId is wrong - unknown
  }
  _aC(mn, '\0');
}

byte UtilsAVR::getAVRModelIdByName(const char* mcuModelStr, byte& statusRes) {
  initStatus();
  int l = strLength(mcuModelStr);
  if (l < 4) {
    returnStatusV(ERR(0x84), 0);
  }
  if (mcuModelStr[0] != 'A' || mcuModelStr[1] != 'V' || mcuModelStr[2] != 'R') {
    returnStatusV(ERR(0x84), 0);
  }
  if (mcuModelStr[3] == '@') {
    // read 3 digits
    if (l != 4+3) {
      returnStatusV(ERR(0x84), 0);
    }
    int modelId = convert3DigitsToInt(mcuModelStr+4, statusRes); checkStatusV(0);
    if (modelId <= 0 || modelId > MCU_AVR_TYPES_COUNT) {
      returnStatusV(ERR(0x85), 0);
    }
    return modelId; // SUCCESS
    
  } else if (mcuModelStr[3] == '[') {
    // read hex signature
    if (l != 4+6+1) {
      returnStatusV(ERR(0x84), 0);
    }
    if (mcuModelStr[10] != ']') {
      returnStatusV(ERR(0x84), 0);
    }
    byte signBytes[3];
    signBytes[0] = convertTwoHexCharsToByte(mcuModelStr+4+0, statusRes); checkOverrideStatusV(ERR(0x86), 0);
    signBytes[1] = convertTwoHexCharsToByte(mcuModelStr+4+2, statusRes); checkOverrideStatusV(ERR(0x87), 0);
    signBytes[2] = convertTwoHexCharsToByte(mcuModelStr+4+4, statusRes); checkOverrideStatusV(ERR(0x88), 0);
    return getAVRModelIdBySignature(signBytes, statusRes);
    
  } else if (mcuModelStr[3] == '-') {
    // read full name
    if (l < 5) {
      returnStatusV(ERR(0x84), 0);
    }
    returnStatusV(ERR(0x80), 0); // FIXME Unimplemented!!!
  }
  return 0;
}


  //////////////////////////
  //   SD Files Related
  //////////////////////////

boolean UtilsSD::initSDCard() {
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

// Print 3 digits
void UtilsSD::fPrint3Dig(File& f, byte b) {
  f.print(b / 100);
  f.print((b / 10) % 10);
  f.print(b % 10);
}

//  Print String + Byte + EOL
void UtilsSD::fPrintBln(File& f, String str, byte b) {
  f.print(str);
  fPrintB(f, b);
  f.println();
}

// Print Byte
void UtilsSD::fPrintB(File& f, byte b) {
  f.print(String((b & 0xF0)>>4,HEX));
  f.print(String(b & 0x0F,HEX));
}

byte UtilsSD::read3DigByte(File& f, byte& statusRes) {
  byte b = readHexByte(f,statusRes);
  if (statusRes > 0) return 0;
  byte c1,c2,c3;
  c1 = b >> 4;
  c2 = b & 0x0F;
  
  if (!readChar(f,c3)) returnStatusV(ERR(0x89),0);
  c3 = convertHexCharToByte(c3,statusRes); checkOverrideStatusV(ERR(0x89), 0);
  
  if (c1 > 9 || c2 > 9 || c3 > 9) returnStatusV(ERR(0x89), 0);
  int r = c1 * 100 + c2 * 10 + c3;
  if (r > 255) returnStatusV(ERR(0x89), 0);
  return r;
}

byte UtilsSD::readHexByte(File& f, byte& statusRes) {
  boolean isEOL;
  byte b = readHexByteOrEOL(f,isEOL,statusRes);
  if (isEOL) {
    logDebug("isEOL");
    returnStatusV(ERR(0x8A), 0);
  }
  return b;
}

// remember to check isEOL before statusRes!!! When EOL, statusRes is also an error!
byte UtilsSD::readHexByteOrEOL(File& f, boolean& isEOL, byte& statusRes) {
  statusRes = 0;
  byte c1,c2;
  if (!readChar(f,c1)) returnStatusV(ERR(0x8A), 0);
  logDebugB("readHexByte_1:", c1);
  
  if (!readChar(f,c2)) returnStatusV(ERR(0x8A), 0);
  logDebugB("readHexByte_2:", c2);
    
  if (c1 <= 0x0D || c2 <= 0x0D) {
    if (c1 == 0x0D && c2 == 0x0A) {
      logDebug("readHexByte_EOL");
      isEOL = true;
    } else {
      logDebug("readHexByte_EOL_corrupted");
      returnStatusV(ERR(0x8A), 0);
    }
    return 0;
  }
  c1 = convertHexCharToByte(c1,statusRes);
  checkOverrideStatusV(ERR(0x8A), 0);
  logDebugB("c1", c1);
  
  c2 = convertHexCharToByte(c2,statusRes);
  checkOverrideStatusV(ERR(0x8A), 0);
  logDebugB("c2", c2);
  return (c1 << 4) | c2;
}

boolean UtilsSD::readChar(File& f, byte& c) {
  if (f.available()) {
    c = f.read();
    return true;
  } else {
    return false;// unexpected end
  }
}

int UtilsSD::readToTheEOL(File& f, byte& statusRes) {
  int readChars = 0;
  while (f.available()) {
    byte c = f.read();
    if (c == 0x0A) {
      logDebug("unexp0A");
      returnStatusV(ERR(0x8B), readChars);
    }
    if (c == 0x0D) {
      if (f.available()) c = f.read();
      if (c == 0x0A) {
        return readChars;
      } else {
        logDebug("unexpChar");
        returnStatusV(ERR(0x8B), readChars);
      }
    }
    readChars++;
  }
  logDebug("unexp_EOF");
  returnStatusV(ERR(0x8B), 0);// unexpected end
}


  //////////////////////////
  //   Tests Related
  //////////////////////////

void UtilsTests::_testStringMatches_LogDebug(char c1, char c2) {
  char str[] = "c-c";
  str[0] = c1; str[2] = c2;
  logDebugS("_", str);
  return;
}

boolean UtilsTests::assertStringMatches(const char* str1, const char* str2, boolean debugMode) {
  byte pos = 0;
  while (str1[pos] == str2[pos]) {
    if (debugMode) _testStringMatches_LogDebug(str1[pos], str2[pos]);
    if (str1[pos] == '\0') {
      return true;
    }
    if (pos++ > 125) return false;
  }
  if (debugMode) _testStringMatches_LogDebug(str1[pos], str2[pos]);
  return false;
}
