#include "HEXFile.h"

// ERR() List:
// 0x90 - Unknown system error
// 0x91 - Cannot open HEX File (either for open or write)
// 0x92 - Cannot open backup file for write - most likely it is SD card problem
// 0x93 - Cannot find next backup file - not enough file names
// 0x94 - Corrupted HEX File - wrong format

void HEXFile::__translateErrorsToDisplayErrorCode(byte err, byte& mainErrCode, byte& subErrCode, byte& okCode) {
  mainErrCode = 0;
  subErrCode = 0;
  okCode = 0;
  
  // FIXME Implement
  if (err == 0x90) {
    mainErrCode = 0xA; subErrCode = 0x2;
  } else if (err == 0x91) {
    mainErrCode = 0x2; subErrCode = 0x7;
  } else if (err == 0x92) {
    mainErrCode = 0x2; subErrCode = 0x6;
  } else if (err == 0x93) {
    mainErrCode = 0x2; subErrCode = 0x4;
  } else if (err == 0x94) {
    mainErrCode = 0x2; subErrCode = 0x3;
  } else {
    mainErrCode = 0xA;
    AVRProgrammer::__translateErrorsToDisplayErrorCode(err, mainErrCode, subErrCode, okCode);
    //FIXME call Utils after AVRProgrammer handlers!
  }
}

  ////////////////////////////////////////////////
  // MAIN/PUBLIC FUNCTIONS
  ////////////////////////////////////////////////

void HEXFile::openFile(File& f, String fileName, int mode, byte& statusRes) {
  initStatus();
  f = SD.open(fileName, FILE_READ);
  if (mode == FILE_WRITE) {
    if (!f) {
      logDebugS("File exists! override protection!:", fileName);
      returnStatus(ERR(0x90));//system error, calling program must check if file exists
    }
    f = SD.open(fileName, FILE_WRITE);
  }
  if (!f) {
    logDebugS("!open:",fileName);
    returnStatus(ERR(0x91)); //Cannot open ProgramFile
  } else {
    logInfoS("Opened file:", fileName);
  }
}


byte HEXFile::_crc8 = 0;
void HEXFile::crc8Init() {
  _crc8 = 0;
}
byte HEXFile::crc8Append(byte b) {
  _crc8 += b;
  return b;
}
byte HEXFile::crc8Close() {
  return 1 + (_crc8 ^ 0xFF);
}

byte HEXFile::LINE_TYPE_DATA = 0;
byte HEXFile::LINE_TYPE_EOF = 1;

void HEXFile::readLine(File& f, byte& lineType, byte* buf, byte bufSize, byte& resSize, unsigned int& address, byte& statusRes) {
  byte i;
  crc8Init();
  logDebug("readLine");
  // :LLAAAATTDDDD.....DDCC
  // c[0] - LL - Length of Data
  // c[1-2] - AAAA - Address
  // c[3] - TT - Line Type (0x00 - Data; 0x01 - End Of File; ...)
  if (!UtilsSD::readChar(f,i)) { logDebug("rl0"); returnStatus(ERR(0x9F)); }
  if (i != ':') {
    logDebugB("badStart:",i);
    returnStatus(ERR(0x9F));
  }

  // read data length
  resSize = crc8Append(UtilsSD::readHexByte(f,statusRes)); checkOverrideStatus(ERR(0x9F));
  if (resSize > bufSize) {
    logErrorD("too big line data!", resSize);
    returnStatus(ERR(0x9F));
  }

  // read address
  logDebug("addrH");
  address = crc8Append(UtilsSD::readHexByte(f,statusRes)) << 8; checkOverrideStatus(ERR(0x9F));
  logDebug("addrL");
  address |= crc8Append(UtilsSD::readHexByte(f,statusRes)); checkOverrideStatus(ERR(0x9F));
  logDebugD("addr:", address);

  logDebug("type");
  lineType = crc8Append(UtilsSD::readHexByte(f,statusRes)); checkOverrideStatus(ERR(0x9F));
  if (lineType != LINE_TYPE_DATA && lineType != LINE_TYPE_EOF) {
    logDebugB("bad!:", lineType);
    returnStatus(ERR(0x9F));
  }
  
  logDebug("data");
  for (i = 0; i < resSize; i++) {
    buf[i] = crc8Append(UtilsSD::readHexByte(f,statusRes)); checkOverrideStatus(ERR(0x9F));
  }

  logDebug("crc");
  byte crc = UtilsSD::readHexByte(f,statusRes); checkOverrideStatus(ERR(0x9F));
  byte calcCrc = crc8Close();
  if (crc != calcCrc) {
    logDebug("Wrong CRC!");
    logDebugB("expected:", crc);
    logDebugB("calculated:", calcCrc);
    returnStatus(ERR(0x9F));
  }
  
  UtilsSD::readCharOrEolOrEof(f, i, crc, statusRes); checkOverrideStatus(ERR(0x9F));
  if (crc == UtilsSD::IS_CHAR) {
    logDebugC("c:",i);
    returnStatus(ERR(0x9F));
  }

  returnStatusOK();
}
