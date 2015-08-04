#include "ConfFile.h"

  ////////////////////////////////////////////////
  // File related procedures!!!
  ////////////////////////////////////////////////
File _confFile;

boolean initConfFileSD() {
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

boolean openConfFile() {
  _confFile = SD.open("CONFIG.TXT", FILE_READ);
  if (_confFile) {
    return true;
  } else {
    return false;
  }
}

boolean openConfFile(const char* fname) {
  _confFile = SD.open(fname, FILE_READ);
  if (_confFile) {
    return true;
  } else {
    return false;
  }
}

void closeConfFile() {
  if (_confFile) {
    _confFile.close();
  } else {
    logError("!close confF");
  }
}

#define _confFileAvailable() _confFile.available()

// returns false if there are no more characters to read, true otherwise
// This method: skips all white spaces and comments, positions on the first valid character to read
// NOTE: this method expects to be on the beginning of file, or on the beginning of new line
boolean skipToValidData(byte& statusRes) {
  initStatus();
  boolean av = true;
  while (_confFile.available()) {
    av = skipWhiteCharacters(statusRes); checkStatusV(false);
    if (!av) return false;
    
    char c = _confFile.peek();
    if (c != '#') {
      return true;
    } else {
      skipLine(statusRes); checkStatusV(false);
    }
  }
  return false;
}

// returns false if there are no more characters to read, true otherwise
void skipLine(byte& statusRes) {// skips to the next line, positions on first character of next line
  initStatus();
  while (_confFile.available()) {
    byte c = _confFile.read();
    if (c == 0x0A) {
      return;
    }
    if (c == 0x0D) {
      if (_confFile.available()) c = _confFile.read();
      if (c != 0x0A) {
        returnStatus(0x27);
      }
      return;
    }
  }
}

// returns false if there are no more characters to read, true otherwise
void skipWhiteCharactersExpectNoEolNoEof(byte& statusRes) {// skip all white characters (except EOL), positions on the first non-white character. Expect no EOL or EOF!
  initStatus();
  int skippedChars = 0;
  while (_confFile.available()) {
    char c = _confFile.peek();
    if (c == 0x0A || c == 0x0D) {
      returnStatus(0x28);
    } else {
      if (!isWhiteChar(c)) {
        return;
      }
    }
    _confFile.read(); skippedChars++;
  }
  returnStatus(0x29);
}

// returns false if there are no more characters to read, true otherwise
boolean skipWhiteCharacters(byte& statusRes) {// skip all white characters (including EOL), positions on the first non-white character
  initStatus();
  int skippedChars = 0;
  while (_confFile.available()) {
    char c = _confFile.peek();
    if (c == 0x0A) {
      // Linux EOL, also a white char, so skip it
    } else if (c == 0x0D) {
      // Windows EOL, two chars
      _confFile.read(); skippedChars++;
      if (!_confFile.available() || _confFile.peek() != 0x0A) {
        returnStatusV(0x2A,false);// no 0A after 0D
      }
    } else {
      if (!isWhiteChar(c)) {
        return true;
      }
    }
    _confFile.read(); skippedChars++;
  }
  return false;
}

char readCharSafe(boolean& eol, byte& statusRes) {
  eol = true;
  if (!_confFile.available()) {
    returnStatusV(0x2C,'\0');// unexpected EOF
  }
  char c = _confFile.read();
  if (c == 0x0A) {
    // Linux EOL
    return c;
  } else if (c == 0x0D) {
    // Windows EOL, two chars
    if (!_confFile.available() || _confFile.peek() != 0x0A) {
      returnStatusV(0x2B,c);// no 0A after 0D
    }
  } else {
    eol = false;
    return c;
  }
}

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  // IMPLEMENTATION GOES HERE!
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////

boolean getFilePathByProgIdAndMcuModel(const char* progId, byte mcuModelId, char* mcuModelBuf, char* filePath, byte& statusRes) {
  while (true) {
    readRootConfFile(progId, mcuModelBuf, filePath, statusRes); checkStatusV(false);
    if (strLength(mcuModelBuf) == 0) {
      return false; // No match found!
    }

    // check if mcuModel matches mcuModelBuf
    byte detectedMcuModelId = getAVRModelIdByName(mcuModelBuf, statusRes); checkStatusV(false);
    logDebugD("mcuModel:", detectedMcuModelId);
    if (mcuModelId == detectedMcuModelId) {
      return true; // SUCCESS
    }
  }
}

void getDirNameByProgId(const char* progId, char* dir1Name, char* dir2Name, boolean& matchDetected, byte& statusRes) {
  /*initStatus();
  dir1Name[0] = 0;
  dir2Name[0] = 0;
  readRootConfFile(progId, dir1Name, matchDetected, statusRes); checkStatus();
  if (!matchDetected) {
    autogenerateDirNamesByProgId(progId, dir1Name, dir2Name);
  }*/
}
void autogenerateDirNamesByProgId(const char* progId, char* dir1Name, char* dir2Name) {
  // autogenerate dir name
  const byte progIdLength = strLength(progId);
  if (progIdLength > 7) {
    // split by two
    for (byte i = 0; i < 7; i++) {
      dir1Name[i] = progId[i];
    }
    dir1Name[7] = '.';
    dir1Name[8] = '_';
    dir1Name[9] = '_';
    dir1Name[10] = '_';
    dir1Name[11] = 0;

    for (byte i = 0; i < 8; i++) {
      dir2Name[i] = progId[i + 7];
    }
    dir2Name[8] = 0;
  } else {
    for (byte i = 0; i < progIdLength; i++) {
      dir1Name[i] = progId[i];
    }
    dir1Name[progIdLength] = 0;
  }
}

void readRootConfFile(const char* progId, char* mcuModel, char* filePath, byte& statusRes) {
  initStatus();
  mcuModel[0] = '\0';
  filePath[0] = '\0';
  byte pos = 0;
  boolean eol;
  char c;
  byte progIdLength = strLength(progId);
  logDebugD("l:", progIdLength);
  
  boolean matchDetected = false;
  while (!matchDetected && skipToValidData(statusRes)) { // loop for every valid line
    checkStatus(); // for skipToValidData

    logDebug("skipped");
    // read PROG_ID and match it
    pos = 0;
    while(true) {
      if (!_confFile.available()) {
        returnStatus(0x20); // unexpected EOF
      }
      logDebug("avail");
      c = readCharSafe(eol, statusRes); checkStatus();
      logDebugB("readChar",c);
      if (eol) returnStatus(0x21); // unexpected EOL
      if ((pos >= progIdLength) || (progId[pos] != c) || isWhiteChar(c)) {
           // if we are above expected progId length, no need to continue this line, or
           // if char at pos didn't match progId[pos], or
           // if white char
        break;
      }
      pos++;
    }

    logDebugD("step1:",pos);

    if (pos != progIdLength) {
      goto _skipLine;
    } else {
      matchDetected = true;
    }
      
    logDebugD("step2:",pos);
    
    // read MCU_MODEL
    skipWhiteCharactersExpectNoEolNoEof(statusRes); checkStatus();
    pos = 0;
    while (true) {
      if (pos >= MCU_MODEL_BUFFER_SIZE) {// too long MCU_MODEL in conf file!
        mcuModel[MCU_MODEL_BUFFER_SIZE-1] = '\0';
        returnStatus(0x22);
      }
      
      if (!_confFile.available()) {
        returnStatus(0x23); // unexpected EOF
      }
      mcuModel[pos] = readCharSafe(eol, statusRes); checkStatus();
      if (eol) {
        returnStatus(0x24); // unexpected EOL
      }
      if (isWhiteChar(mcuModel[pos])) {
        mcuModel[pos] = '\0';
        break;
      }
      pos++;
    }

    logDebugD("step3:",pos);
    
    // read FILE_PATH
    skipWhiteCharactersExpectNoEolNoEof(statusRes); checkStatus();
    pos = 0;
    while (true) {
      if (pos >= FILE_PATH_BUFFER_SIZE) {// too long FILE_PATH in conf file!
        filePath[FILE_PATH_BUFFER_SIZE-1] = '\0';
        returnStatus(0x25);
      }
      
      if (!_confFile.available()) {
        returnStatus(0x26); // unexpected EOF
      }
      filePath[pos] = readCharSafe(eol, statusRes); checkStatus();
      if (eol || isWhiteChar(filePath[pos])) {
        filePath[pos] = '\0';
        break;
      }
      pos++;
    }

    logDebugD("step4:",pos);
    
    // Skip this line and proceed to next line
    _skipLine: skipLine(statusRes); checkStatus(); // skip the line, and continue to next one
  }
}


  ////////////////////////////////////////////////
  // Test hooks
  ////////////////////////////////////////////////
  
boolean __test_confFileOpen() {
  if (_confFile) {
    return true;
  } else {
    return false;
  }
}
boolean __test_confFileAvailable() {
  return _confFile.available();
}

char __test_confFileReadChar() {
  return _confFile.read();
}
