#include "Tests.h"

char __testMcuModelBuf[UtilsAVR::MCU_MODEL_BUFFER_SIZE];
char __testMcuModelNameBuf[UtilsAVR::MCU_MODEL_HUMAN_NAME_BUFFER_SIZE];
char __testFilePathBuf[FILE_PATH_BUFFER_SIZE];

/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
///    UTILS
///    UTILS
///    UTILS
///    UTILS
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

void testUtilsGen() {
  logInfo(">> Utils - GEN");
  __testConvertByteToHexChar();
  __testConvertHexCharToByte();
  __testConvertTwoHexCharsToByte();
  __testConvert3DigitsToInt();
  __testIsWhiteChar();
  __testIsValidNameChar();
  __testStrLength();
  logInfo("-- END");
  return;
}

void testUtilsAVR() {
  logInfo(">> Utils - AVR");
  __testGetAVRModelIdBySignature();
  __testGetAVRModelAndConf();
  __testGetAVRModelNameById();
  __testGetAVRModelIdByName();
  logInfo("-- END");
  return;
}


void __testConvertByteToHexChar() {
  // convertByteToHexChar
  logInfo(">utils_01");
  char c = convertByteToHexChar(0xEA, true);
  if (c != 'E') {
    logError("#E");
  }
  c = convertByteToHexChar(0xE8, false);
  if (c != '8') {
    logError("#8");
  }
  c = convertByteToHexChar(0xA0, true);
  if (c != 'A') {
    logError("#A");
  }
  c = convertByteToHexChar(0xA0, false);
  if (c != '0') {
    logError("#0");
  }
  logInfo("- END");
  return;
}

void __testConvertHexCharToByte() {
  byte statusRes;
  logInfo(">utils_02");
  byte r = convertHexCharToByte('0', statusRes);
  if (statusRes == 7) {
    logError("#I");//statusInit didn't work properly
  }
  if (statusRes != 0) {
    logError("#0a");
  }
  if (r != 0) {
    logError("#0b");
  }
  r = convertHexCharToByte('1', statusRes);
  if (r != 1) {
    logError("#1");
  }
  r = convertHexCharToByte('9', statusRes);
  if (r != 9) {
    logError("#9");
  }
  r = convertHexCharToByte('A', statusRes);
  if (r != 0xA) {
    logError("#A");
  }
  r = convertHexCharToByte('a', statusRes);
  if (r != 0xA) {
    logError("#a");
  }
  r = convertHexCharToByte('F', statusRes);
  if (r != 0xF) {
    logError("#F");
  }
  r = convertHexCharToByte('e', statusRes);
  if (r != 0xE) {
    logError("#e");
  }
  r = convertHexCharToByte('Z', statusRes);
  if (statusRes == 0) {
    logError("#Z");
  }
  logInfo("- END");
  return;
}

void __testConvertTwoHexCharsToByte() {
  logInfo(">utils_03");

  byte statusRes;
  char r[] = "A3";
  byte res = convertTwoHexCharsToByte(r, statusRes);
  if (res != 0xA3) {
    logErrorB("#A3:", res);
  }

  char r2[] = "jhkjF4";
  res = convertTwoHexCharsToByte(r2 + 4, statusRes);
  if (res != 0xF4) {
    logErrorB("#F4:", res);
  }

  logInfo("- END");
  return;
}

void __testConvert3DigitsToInt() {
  logInfo(">utils_04");

  byte statusRes;
  char r[] = "jhkjh_456";
  int res = convert3DigitsToInt(r + 6, statusRes);
  if (res != 456) {
    logErrorD("#456:", res);
  }

  logInfo("- END");
  return;
}

void __testIsWhiteChar() {
  return;
}

void __testIsValidNameChar() {
  return;
}

void __testStrLength() {
  logInfo(">utils_05");
  char s[] = "123";
  int l = strLength(s);
  if (l != 3) {
    logErrorD("strLength", l);
  }
  logInfo("- END");
  return;
}

void __testGetAVRModelIdBySignature() {
  logInfo(">utils_06");
  byte statusRes;
  // return model ID by signature
  //        byte signBytes[3];
  //        readSignatureBytes(signBytes, statusRes); checkStatus();
  //        byte modelId = getAVRModelIdBySignature(signBytes, statusRes); checkStatus();
  byte signBytes[3] = { MCU_AVR_TYPES[MCU_AVR_ATmega88PA - 1][0], MCU_AVR_TYPES[MCU_AVR_ATmega88PA - 1][1], MCU_AVR_TYPES[MCU_AVR_ATmega88PA - 1][2] } ;
  byte modelId = UtilsAVR::getAVRModelIdBySignature(signBytes, statusRes);
  if (statusRes != 0) {
    logErrorB("#mbysign1:", statusRes);
  }
  if (modelId != MCU_AVR_ATmega88PA) {
    logErrorD("#mbysign2:", modelId);
  }
  logInfo("- END");
  return;
}

void __testGetAVRModelAndConf() {
  //inline byte getAVRModelAndConf(byte* signBytes, byte& flashPageSize, byte& flashPagesCount, byte& eepromPageSize, byte& eepromPagesCount, byte& statusRes) {
  return;
}

void __testGetAVRModelNameById() {
  logInfo(">utils_07");
  byte statusRes;
  UtilsAVR::getAVRModelNameById(__testMcuModelNameBuf, MCU_AVR_ATmega168PA, statusRes);
  if (statusRes != 0) {
    logErrorB("#avrname1:", statusRes);
  }
  if (__testMcuModelNameBuf[3] != 'e') {
    logErrorB("#avrname2:", __testMcuModelNameBuf[3]);
  }
  logInfo("- END");
  return;
}

void __testGetAVRModelIdByName() {
  logInfo(">utils_08");
  byte statusRes;
  byte res = UtilsAVR::getAVRModelIdByName("AVR@002", statusRes);
  if (statusRes != 0) {
    logErrorB("#avrm_id1:", statusRes);
  }
  if (res != 2) {
    logErrorD("#avrm_id2:", res);
  }

  res = UtilsAVR::getAVRModelIdByName("AVR[1e9514]", statusRes);
  if (statusRes != 0) {
    logErrorB("#avrm_id3:", statusRes);
  }
  if (res != MCU_AVR_ATmega328) {
    logErrorD("#avrm_id4:", res);
  }
  logInfo("- END");
  return;
}


/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
///    TARGET PROGRAM DETECTOR
///    TARGET PROGRAM DETECTOR
///    TARGET PROGRAM DETECTOR
///    TARGET PROGRAM DETECTOR
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

void testTargetProgramDetector() {
  logDebug(">> TargetPD");
  // TARGET_MCU_ID__W0, TARGET_MCU_ID__W1, TARGET_MCU_ID__W2, TARGET_MCU_ID__MANUAL0, TARGET_MCU_ID__MANUAL1
  TargetProgramDetector* pd = new TargetProgramDetector(A0, A1, A2, A3, A4);
  logFreeRam();
  logDebug("TargetPD#01");
  __testOneWireID(pd, 2000);
  logDebug("TargetPD#02");
  __testGetProgId(pd, 2000);
  logDebug("-- END");
  return;
}

// requires programDetecter
void __testOneWireID(TargetProgramDetector* pd, int d) {
  logInfo(">TPD#01");
  byte statusRes;
  byte id[8];
  boolean idPresent = pd->read1WireId(id, statusRes);
  if (statusRes != 0) {
    logInfo("Error detecting ID chip!");
  } else {
    if (idPresent) {
      logInfo("ID chip data:");
      logDebugB("[0] ", id[0]);
      logDebugB("[1] ", id[1]);
      logDebugB("[5] ", id[5]);
      logDebugB("[6] ", id[6]);
      if (id[0] != 0x81) {
        logError("Wrong family code!");
      }
    } else {
      logError("No ID chip!");
    }
  }
  logInfo("-END"); delay(d);
  return;
}

// requires programDetecter
void __testGetProgId(TargetProgramDetector* pd, int d) {
  logInfo(">TPD#02");
  byte statusRes;
  boolean autoSelected;
  char progIdBuf[16];
  logFreeRam();
  pd->getProgId(progIdBuf, autoSelected, statusRes);
  logFreeRam();
  if (statusRes != 0) {
    logErrorB("Error! ", statusRes);
  } else {
    logInfoS("Good! ", progIdBuf);
  }
  logInfo("-END"); delay(d);
  return;
}

/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
///    CONF FILE
///    CONF FILE
///    CONF FILE
///    CONF FILE
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

void testConfFile() {
  logInfo(">> ConfFile");
  logFreeRam();
  __testAutogenerateDirNamesByProgId();
  __testSkipToValidData();
  __testReadRootConfFile();
  logInfo("-- END");
  return;
}


char __testConfFileName[] = "TEST-XX.TXT";
boolean __testOpenConfFile(byte pos) {
  __testConfFileName[5] = '0' + (pos / 10); __testConfFileName[6] = '0' + (pos % 10);
  return openConfFile(__testConfFileName);
}
void __testStringMatches_LogDebug(char c1, char c2) {
  char str[] = "c-c";
  str[0] = c1; str[2] = c2;
  logDebugS("", str);
  return;
}

boolean __testStringMatches(const char* str1, const char* str2, boolean debugMode) {
  byte pos = 0;
  while (str1[pos] == str2[pos]) {
    if (debugMode) __testStringMatches_LogDebug(str1[pos], str2[pos]);
    if (str1[pos] == '\0') {
      return true;
    }
    if (pos++ > 125) return false;
  }
  if (debugMode) __testStringMatches_LogDebug(str1[pos], str2[pos]);
  return false;
}


void __testAutogenerateDirNamesByProgId() {
  logInfo(">autogenDirNames");
  char dir1Name[8 + 4 + 1];
  char dir2Name[8 + 1];
  char s1[] = "ID_A1B2C3D4E5F6";
  char s1a[] = "ID_A1B2.___";
  char s1b[] = "C3D4E5F6";
  autogenerateDirNamesByProgId(s1, dir1Name, dir2Name);
  logDebugS("dir1Name", dir1Name);
  logDebugS("dir2Name", dir2Name);
  logInfo("#1a");
  __testStringMatches(s1a, dir1Name, true);
  logInfo("#1b");
  __testStringMatches(s1b, dir2Name, true);

  char s2[] = "MN_A1C3";
  autogenerateDirNamesByProgId(s2, dir1Name, dir2Name);
  logDebugS("dir1Name", dir1Name);
  logDebugS("dir2Name", dir2Name);
  logInfo("#2");
  __testStringMatches(s2, dir1Name, true);
  logInfo("- END");
  return;
}

#define assertStatusOKOrGoto(statusRes, msg, _goto) if (statusRes!=0) { logErrorB(msg, statusRes); goto _goto; }
#define assertOKOrGoto(ok, msg, b, _goto) if (!ok) { logErrorB(msg, b); goto _goto; }
#define assertStatusOKOrReturn(statusRes, msg) if (statusRes!=0) { logErrorB(msg, statusRes); return; }
#define assertOKOrReturn(ok, msg, b) if (!ok) { logErrorB(msg, b); return; }
#define assertStatusOKOrReturnV(statusRes, msg, v) if (statusRes!=0) { logErrorB(msg, statusRes); return v; }
#define assertOKOrReturnV(ok, msg, b, v) if (!ok) { logErrorB(msg, b); return v; }

void __testSkipToValidData() {
  logInfo(">skipToValidData");
  byte statusRes;
  char c;
  boolean av;

  if (!__testOpenConfFile(01)) {
    logError("#-1");
    goto _close;
  }

  // Skip to row#5, to R1_E token
  av = skipToValidData(statusRes);
  assertOKOrGoto(av, "#0", av, _close);
  assertStatusOKOrGoto(statusRes, "#1", _close);
  assertOKOrGoto(__test_confFileOpen(), "#2", 0, _close);
  assertOKOrGoto(__test_confFileAvailable(), "#3", 0, _close);
  // now read "R1_E"
  c = __test_confFileReadChar(); // R
  assertStatusOKOrGoto(statusRes, "#4", _close);
  assertOKOrGoto(c == 'R', "#5", c, _close);
  c = __test_confFileReadChar(); // 1
  assertStatusOKOrGoto(statusRes, "#6", _close);
  c = __test_confFileReadChar(); // _
  assertStatusOKOrGoto(statusRes, "#7", _close);
  c = __test_confFileReadChar(); // E
  assertStatusOKOrGoto(statusRes, "#8", _close);
  assertOKOrGoto(c == 'E', "#9", c, _close);

  // Skip to AVR@002
  skipToValidData(statusRes);
  assertStatusOKOrGoto(statusRes, "#10", _close);
  // now read "AVR@002 "
  c = __test_confFileReadChar(); // R
  assertStatusOKOrGoto(statusRes, "#11", _close);
  assertOKOrGoto(c == 'A', "#12", c, _close);
  for (byte i = 0; i < 6; i++) {
    c = __test_confFileReadChar(); // [VR@002]
    assertStatusOKOrGoto(statusRes, "#13", _close);
  }
  c = __test_confFileReadChar(); // ' '
  assertStatusOKOrGoto(statusRes, "#14", _close);
  assertOKOrGoto(c == ' ', "#15", c, _close);
  
  // Skip to HJKHJH
  skipToValidData(statusRes);
  assertStatusOKOrGoto(statusRes, "#16", _close);

  _close:
  if (!closeConfFile()) {
    logError("!closedFile");
  }
  logInfo("> END");
  return;
}

void __testReadRootConfFile() {
  char expectedDirName[] = "HJKHJH";
  char expectedMcuModel[] = "AVR@002";
  char expected2DirName[] = "2";
  char expected2McuModel[] = "AVR[1e34f4]";
  char progId[] = "R1_E";
  byte statusRes;
  
  logInfo(">testRRCF");
  assertOKOrReturn(__testOpenConfFile(01), "#00", 0);
  
  readRootConfFile(progId, __testMcuModelBuf, __testFilePathBuf, statusRes);
  assertStatusOKOrReturn(statusRes, "#01");
  assertOKOrReturn(strLength(__testFilePathBuf) > 0, "#02", strLength(__testFilePathBuf));
  if (!__testStringMatches(expectedDirName, __testFilePathBuf, true)) {
    logError("#03");
  }
  if (!__testStringMatches(expectedMcuModel, __testMcuModelBuf, true)) {
    logError("#04");
  }

  // read again
  readRootConfFile(progId, __testMcuModelBuf, __testFilePathBuf, statusRes);
  assertStatusOKOrReturn(statusRes, "#06");
  assertOKOrReturn(strLength(__testFilePathBuf) > 0, "#07", strLength(__testFilePathBuf));
  if (!__testStringMatches(expected2DirName, __testFilePathBuf, true)) {
    logError("#08");
  }
  if (!__testStringMatches(expected2McuModel, __testMcuModelBuf, true)) {
    logError("#09");
  }

  if (!closeConfFile()) {
    logError("!closedFile");
  }
  logInfo("> END");
  return;
}
