#include "Utils.h"

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

boolean initSDCard() {
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
  logDebugB("Wrong character:", c);
  returnStatusV(0x30, 0);
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


byte __getAVRModleName_pos;
inline void _aC(char* modelName, char c) {
  modelName[__getAVRModleName_pos++] = c;
}

void getAVRModelNameById(char* mn, byte modelId, byte& statusRes) {
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
    mn[0] = '\0'; returnStatus(0x30);
  }
  _aC(mn, '\0');
}

byte getAVRModelIdByName(const char* mcuModelStr, byte& statusRes) {
  initStatus();
  int l = strLength(mcuModelStr);
  if (l < 4) {
    returnStatusV(0x30, 0);
  }
  if (mcuModelStr[0] != 'A' || mcuModelStr[1] != 'V' || mcuModelStr[2] != 'R') {
    returnStatusV(0x30, 0);
  }
  if (mcuModelStr[3] == '@') {
    // read 3 digits
    if (l != 4+3) {
      returnStatusV(0x30, 0);
    }
    int modelId = convert3DigitsToInt(mcuModelStr+4, statusRes); checkStatusV(0);
    if (modelId <= 0 || modelId > MCU_AVR_TYPES_COUNT) {
      returnStatusV(0x30, 0);
    }
    return modelId; // SUCCESS
    
  } else if (mcuModelStr[3] == '[') {
    // read hex signature
    if (l != 4+6+1) {
      returnStatusV(0x30, 0);
    }
    if (mcuModelStr[10] != ']') {
      returnStatusV(0x30, 0);
    }
    byte signBytes[3];
    signBytes[0] = convertTwoHexCharsToByte(mcuModelStr+4+0, statusRes); checkOverrideStatusV(0x31, 0);
    signBytes[1] = convertTwoHexCharsToByte(mcuModelStr+4+2, statusRes); checkOverrideStatusV(0x32, 0);
    signBytes[2] = convertTwoHexCharsToByte(mcuModelStr+4+4, statusRes); checkOverrideStatusV(0x33, 0);
    return getAVRModelIdBySignature(signBytes, statusRes);
    
  } else if (mcuModelStr[3] == '-') {
    // read full name
    if (l < 5) {
      returnStatusV(0x30, 0);
    }
    returnStatusV(0xFF, 0); // FIXME Unimplemented!!!
  }
  return 0;
}


  //////////////////////////
  //   SD Files Related
  //////////////////////////


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

