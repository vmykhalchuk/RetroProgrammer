#include "TargetProgramDetector.h"

byte _pinW0;
byte _pinW1;
byte _pinW2;
byte _pinManualM0;
byte _pinManualM1;

void TargetProgramDetector::setup(byte pinW0, byte pinW1, byte pinW2, byte pinManualM0, byte pinManualM1)
{
  pinMode(pinW0, INPUT);
  pinMode(pinW1, INPUT);
  pinMode(pinW2, INPUT);
  pinMode(pinManualM0, INPUT);
  pinMode(pinManualM1, INPUT);
  _pinW0 = pinW0;
  _pinW1 = pinW1;
  _pinW2 = pinW2;
  _pinManualM0 = pinManualM0;
  _pinManualM1 = pinManualM1;
}

void TargetProgramDetector::getProgId(char* progId, boolean& autoSelected, byte& statusRes) {
  #if !OPTIMIZE
    initStatus();
  #endif
  autoSelected = true;
  byte id[8];
  byte idType;
  readTargetMcuId(id, idType, statusRes); checkStatus();
  
  byte progIdLength = 3;
  
  if (idType == 0) {
    // Read Manual program (no autodetection)
    autoSelected = false;
    progId[0] = 'M';
    progId[1] = 'N';
    progIdLength += 4;
    readManualProgrammSelector(progId + 3, statusRes);
    #if !OPTIMIZE
      checkStatus();
    #endif
  } else if (idType == 1) {
    // 1Wire ID detected (first 6 bytes of id)
    progId[0] = 'I';
    progId[1] = 'D';
    progIdLength += 12;
    for (byte i = 0; i < 6; i++) {
      progId[3 + i*2] = convertByteToHexChar(id[i + 1], true);
      progId[3 + i*2 + 1] = convertByteToHexChar(id[i + 1], false);
    }
  } else if (idType == 2) {
    // Resistive ID
    progId[0] = 'R';
    progIdLength += 1;
    progId[3] = 'E' + id[0];
    if (id[1] != 127) {
      progId[1] = '1';
    } else {
      progId[1] = '2';
      progIdLength += 2;
      progId[4] = '1' + id[1];
      progId[5] = '1' + id[2];
    }
  }
  progId[2] = '_';
  progId[progIdLength] = '\0';
  return;
}

void TargetProgramDetector::readTargetMcuId(byte* data, byte& idType, byte& statusRes) {
  initStatus();
  int v;

  // Reset ID Chip (or see if W0 is loaded on resistor divider instead)
  pinMode(_pinW0, INPUT);
  delay(1);
  boolean highV = false;
  v = analogRead(_pinW0);
  if (v == 1023) {
    boolean is1wireDevPresent = read1WireId(data, statusRes);
    checkStatus();
    idType = is1wireDevPresent ? 1 : 0;
  } else {
    // most likely it is resistor divider, but check it
    idType = 2;
  }

  if (idType == 0) {
    return;
  } else if (idType == 1) {
    read1WireId(data, statusRes);
  } else { // idType == 2
    // Read Resistive Data
    data[0] = readResistiveDividerValue(_pinW0, statusRes);
    checkStatus();
    if (data[0] == 127) {
      returnStatus(ERROR_TARGET_DETECTOR + 0x0);
    }
    data[1] = readResistiveDividerValue(_pinW1, statusRes);
    checkStatus();
    data[2] = readResistiveDividerValue(_pinW2, statusRes);
    checkStatus();
    if (data[1] == 127 || data[2] == 127) {
      if (data[1] == 127 && data[2] == 127) {
        // fine, limited version of identification used
      } else {
        returnStatus(ERROR_TARGET_DETECTOR + 0x1);
      }
    }
  }
  return;
}

#if USE_ONE_WIRE_LIBRARY
boolean TargetProgramDetector::read1WireId(byte* data, byte& statusRes) {
  initStatus();
  OneWire ds(_pinW0);

  //1-Wire bus reset, needed to start operation on the bus,
  //returns a 1/TRUE if presence pulse detected
  if (ds.reset() != TRUE) {
    return false;
  }
  
  logDebug("- Device present -");
  ds.write(READ_ID_COMMAND);  //Send Read data command
  
  data[0] = (byte) ds.read();
  logDebugB("Family code: ", data[0]);
    
  logDebug("Hex ROM data: ");
  for (byte i = 1; i <= 6; i++) {
    data[i] = (byte) ds.read(); //store each byte in different position in array
    logDebugB("data[]", data[i]);
  }
    
  data[7] = (byte) ds.read(); //read CRC, this is the last byte
  byte crc_calc = OneWire::crc8(data, 7); //calculate CRC of the data

  logDebugB("Calculated CRC: ", crc_calc);
  logDebugB("Actual CRC: ", data[7]);

  if (data[7] != crc_calc) {
    returnStatusV(ERROR_TARGET_DETECTOR + 0x2, false);
  }
  return true;
}
#else
boolean TargetProgramDetector::read1WireId(byte* data, byte& statusRes) {
  boolean res = reset1Wire(statusRes);
  checkStatusV(false);
  if (!res) {
    return false; // no 1wire device detected
  }
  
  logDebug("- Device present -");
  write1Wire(READ_ID_COMMAND, statusRes);  //Send Read data command
  checkStatusV(false);

  data[0] = read1Wire(statusRes);// Read 1wire device Family code
  checkStatusV(false);
  logDebugB("Family code: ", data[0]);
    
  logDebug("Hex ROM data: ");
  // Read device ID (6 bytes)
  for (byte i = 1; i <= 6; i++) {
    data[i] = read1Wire(statusRes); //store each byte in different position in array
    checkStatusV(false);
    logDebugB("data[]", data[i]);
  }
 
  data[7] = read1Wire(statusRes); //read CRC, this is the last byte
  checkStatusV(false);
  byte crc_calc = OneWire::crc8(data, 7); //calculate CRC of the data

  logDebugB("Calculated CRC: ", crc_calc);
  logDebugB("Actual CRC: ", data[7]);

  if (data[7] != crc_calc) {
    returnStatusV(ERROR_TARGET_DETECTOR + 0x3, false);
  }
  return true;
}
#endif

// return 0 - floating - N/A; or 1 - 1Wire Device present
boolean TargetProgramDetector::reset1Wire(byte& statusRes) {
  #if !OPTIMIZE
    initStatus();
  #endif
  int v;
  
  // try if W0 is connected to ID chip or maybe its just floating
  pinMode(_pinW0, INPUT);
  
  byte retries = 100;
  do {
    delayMicroseconds(5);
    v = analogRead(_pinW0);
  } while (retries-- > 0 && v != 1023);
  if (v != 1023) returnStatusV(ERROR_TARGET_DETECTOR + 0x3, false);
  
  pinMode(_pinW0, OUTPUT);
  digitalWrite(_pinW0, LOW);
  delayMicroseconds(450);
  //digitalWrite(_pinW0, HIGH); // no writing, we now must see reaction from device, otherwise we get shorting
  pinMode(_pinW0, INPUT);
  delayMicroseconds(60);
  v = analogRead(_pinW0);
  delayMicroseconds(400);
  if (v == 1023) {
    return false; // still floating, so there is no Target ID auto defined
  } else if (v == 0) {
    charge1Wire(1000, statusRes);
    return true; // ID chip detected
  } else {
    returnStatusV(ERROR_TARGET_DETECTOR + 0x4, false);
  }
}

void TargetProgramDetector::charge1Wire(int delay, byte& statusRes) {
  /*initStatus();
  pinMode(_pinW0, INPUT);
  delayMicroseconds(20);
  int v = analogRead(_pinW0);
  if (v <= ONE_WIRE_TRESHOLD_HIGH) {
    returnStatus(ERROR_TARGET_DETECTOR + 0x5); // strange state of device
  }
  pinMode(_pinW0, OUTPUT);
  digitalWrite(_pinW0, HIGH);
  delayMicroseconds(delay);
  pinMode(_pinW0, INPUT);
  digitalWrite(_pinW0, LOW);//turn off pullup resistor, and prepare for OUTPUT LOW
  return;*/
}

void TargetProgramDetector::write1Wire(byte b, byte& statusRes) {
  #if !OPTIMIZED
    initStatus();
  #endif
  pinMode(_pinW0, OUTPUT);
  byte mask;
  for (byte i = 0; i < 8; i++) {
    mask = 1 << i;
    if (b & mask) {
      digitalWrite(_pinW0, LOW);
      delayMicroseconds(10);
      digitalWrite(_pinW0, HIGH);
      delayMicroseconds(55);
    } else {
      digitalWrite(_pinW0, LOW);
      delayMicroseconds(65);
      digitalWrite(_pinW0, HIGH);
      delayMicroseconds(5);
    }
  }
  charge1Wire(100, statusRes);
  pinMode(_pinW0, INPUT);
  return;
}

byte TargetProgramDetector::read1Wire(byte& statusRes) {
  #if !OPTIMIZED
    initStatus();
  #endif
  byte res = 0;
  for (byte i = 0; i < 8; i++) {
    pinMode(_pinW0, OUTPUT);
    digitalWrite(_pinW0, LOW);
    delayMicroseconds(3);
    pinMode(_pinW0, INPUT);
    delayMicroseconds(6);
    int v = digitalRead(_pinW0);
    delayMicroseconds(33);
    if (v == HIGH) {
      res |= 1 << i;
    }
  }
  charge1Wire(100, statusRes);
  return res;
}

/////////// RESISTIVE DIVIDER ON TARGET BOARD

byte TargetProgramDetector::readResistiveDividerValue(byte portW, byte& statusRes) {
  initStatus();
  int v = analogRead(portW);
  if (v == 0) {
    return 0;
  } else if (v > 0 && v <= 150) {
    logDebug("t1"); // value is in between!
    returnStatusV(ERROR_TARGET_DETECTOR + 0x7, 127);
  } else if (v > 150 && v <= 300) {
    return 1;
  } else if (v > 300 && v <= 450) {
    logDebug("t2"); // value is in between!
    returnStatusV(ERROR_TARGET_DETECTOR + 0x7, 127);
  } else if (v > 450 && v <= 600) {
    return 2;
  } else if (v > 600 && v <= 750) {
    logDebug("t3"); // value is in between!
    returnStatusV(ERROR_TARGET_DETECTOR + 0x7, 127);
  } else if (v > 750 && v <= 900) {
    return 3;
  } else if (v > 900 && v < 1023) {
    logDebug("t4"); // value is in between!
    returnStatusV(ERROR_TARGET_DETECTOR + 0x7, 127);
  } else if (v == 1023) {
    return 127;
  } else {
    logDebug("tF"); // very strange value! programm error!
    returnStatusV(ERROR_TARGET_DETECTOR + 0x7, 127);
  }
}

/////////// RESISTIVE DIVIDER ON PROGRAMMER SHIELD FOR MANUAL SELECTION

void TargetProgramDetector::readManualProgrammSelector(char* data, byte& statusRes) {
  readManualProgrammSelectorPort(data + 0, _pinManualM0, statusRes);
  checkStatus();
  readManualProgrammSelectorPort(data + 2, _pinManualM1, statusRes);
  checkStatus();
  data[2] = data[2] + 2;
  data[3] = data[3] + 2;
  return;
}

void TargetProgramDetector::readManualProgrammSelectorPort(char* data, byte port, byte& statusRes) {
  initStatus();
  int v = analogRead(port);
  if (v >= 0 && v <= 150) {
    logDebug("t1"); // value is in between!
    returnStatus(ERROR_TARGET_DETECTOR + 0x8);
  } else if (v > 150 && v <= 300) {
    data[0] = 'A';
    data[1] = '1';
  } else if (v > 300 && v <= 450) {
    logDebug("t2"); // value is in between!
    returnStatus(ERROR_TARGET_DETECTOR + 0x8);
  } else if (v > 450 && v <= 600) {
    data[0] = 'B';
    data[1] = '1';
  } else if (v > 600 && v <= 750) {
    logDebug("t3"); // value is in between!
    returnStatus(ERROR_TARGET_DETECTOR + 0x8);
  } else if (v > 750 && v <= 900) {
    data[0] = 'A';
    data[1] = '2';
  } else if (v > 900 && v < 1023) {
    logDebug("t4"); // value is in between!
    returnStatus(ERROR_TARGET_DETECTOR + 0x8);
  } else if (v == 1023) {
    data[0] = 'B';
    data[1] = '2';
  } else {
    logDebug("tF"); // very strange value!
    returnStatus(ERROR_TARGET_DETECTOR + 0x8);
  }
  return;
}
