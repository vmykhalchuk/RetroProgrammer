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
  /*if (c == '0') {
    return 0;
  } else if (c == '1') {
    return 1;
  } else if (c == '2') {
    return 2;
  } else if (c == '3') {
    return 3;
  } else if (c == '4') {
    return 4;
  } else if (c == '5') {
    return 5;
  } else if (c == '6') {
    return 6;
  } else if (c == '7') {
    return 7;
  } else if (c == '8') {
    return 8;
  } else if (c == '9') {
    return 9;
  } else if (c == 'A' || c == 'a') {
    return 10;
  } else if (c == 'B' || c == 'b') {
    return 11;
  } else if (c == 'C' || c == 'c') {
    return 12;
  } else if (c == 'D' || c == 'd') {
    return 13;
  } else if (c == 'E' || c == 'e') {
    return 14;
  } else if (c == 'F' || c == 'f') {
    return 15;
  }*/
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

