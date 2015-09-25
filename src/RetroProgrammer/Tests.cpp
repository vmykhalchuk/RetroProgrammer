#include "Tests.h"

char __testMcuModelBuf[UtilsAVR::MCU_MODEL_BUFFER_SIZE];
char __testMcuModelNameBuf[UtilsAVR::MCU_MODEL_HUMAN_NAME_BUFFER_SIZE];
char __testFilePathBuf[Utils::FILE_PATH_BUFFER_SIZE];

/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
///    DIRECT BOARD TESTS
///    DIRECT BOARD TESTS
///    DIRECT BOARD TESTS
///    DIRECT BOARD TESTS
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////


void Tests_Board::testManualProgramSelectorPorts() {
  logInfo("Setup test: testManualProgramSelectorPorts");
  pinMode(A5, INPUT);
  while (true) {
    int a5Sum = 0;
    int a5Min = 1023;
    int a5Max = 0;
    for (int i = 0; i < 10; i++) {
      int r = analogRead(A5);
      if (r < a5Min) a5Min = r;
      if (r > a5Max) a5Max = r;
      a5Sum += r;
    }
    logInfoD("a5Min: ", a5Min);
    logInfoD("a5Aver: ", a5Sum / 10);
    logInfoD("a5Max: ", a5Max);
    delay(1500);
  }
  
  /*pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  while(true) {
    int a3Res = analogRead(A3);
    logInfoD("A3 result: ", a3Res);
    int a4Res = analogRead(A4);
    logInfoD("A4 result: ", a4Res);
    delay(1500);
  }*/
}

void Tests_Board::testSDCardPorts() {
  logInfo("Setup test: Test SD Card ports");
  pinMode(4, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, INPUT);

  int i = 0;
  while (true) {
    if (i == 0) {
      digitalWrite(4, HIGH);
      logInfo("CS: On");
    } else if (i == 1) {
      digitalWrite(4, LOW);
      logInfo("CS: Off");
    } else if (i == 2) {
      digitalWrite(13, HIGH);
      logInfo("CLK: On");
    } else if (i == 3) {
      digitalWrite(13, LOW);
      logInfo("CLK: Off");
    } else if (i == 4) {
      digitalWrite(11, HIGH);
      logInfo("MOSI: On");
    } else if (i == 5) {
      digitalWrite(11, LOW);
      logInfo("MOSI: Off");
    }
    int r = digitalRead(12);
    if (r == LOW) {
      logInfo("MISO: 0");
    } else {
      logInfo("MISO: 1");
    }
    delay(1500);

    i++;
    if (i == 6) {
      i = 0;
    }
  }
}


/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
///    TEST AVR PROGRAMMER
///    TEST AVR PROGRAMMER
///    TEST AVR PROGRAMMER
///    TEST AVR PROGRAMMER
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

void Tests_AVRProgrammer::testAVRSignatureRead() {
  byte statusRes;
  AVRProgrammer::startupTargetMcuProgramming(statusRes);// Prepare Target MCU for Programming
  if (statusRes > 0) {
    logError("ProgEn failed!");
    AVRProgrammer::shutdownTargetMcu();
    return;
  } else {
    logInfo("Started prog!");
  }
  byte signBytes[3];
  // make sure it is ATmega328P
  AVRProgrammer::readSignatureBytes(signBytes,statusRes);
  if (statusRes > 0) {
    logError("signature error!");
    return;
  }
  logInfoB("sig0:", signBytes[0]);
  logInfoB("sig1:", signBytes[1]);
  logInfoB("sig2:", signBytes[2]);
  AVRProgrammer::shutdownTargetMcu();
}

void Tests_AVRProgrammer::testUploadProgramTestPage(int targetMcuModelId, int pageNo, byte* pageToUpload, int pageSize) {
  byte statusRes;
  AVRProgrammer::startupTargetMcuProgramming(statusRes);// Prepare Target MCU for Programming
  if (statusRes != 0) {
    logErrorB("ProgEn failed!", statusRes);
    AVRProgrammer::shutdownTargetMcu();
    goto _shutdown;
  } else {
    logInfo("Started prog!");
  }
  byte signBytes[3];
  byte modelId;
  // make sure it is ATmega328P
  AVRProgrammer::readSignatureBytes(signBytes,statusRes);
  if (statusRes != 0) {
    logErrorB("signature error!", statusRes);
    goto _shutdown;
  }
  modelId = UtilsAVR::getAVRModelIdBySignature(signBytes, statusRes);
  if (statusRes != 0) {
    logErrorB("mcuModelIdErr:", statusRes);
    goto _shutdown;
  }
  if (targetMcuModelId != modelId) {
    logErrorD("wrong mcu:", modelId);
    goto _shutdown;
  }

  AVRProgrammer::chipErase(statusRes);
  if (statusRes != 0) {
    logErrorB("chipErase:", statusRes);
    goto _shutdown;
  }

  AVRProgrammer::loadAndWriteProgramMemoryPage(pageToUpload, pageSize, pageNo, modelId, statusRes);
  if (statusRes != 0) {
    logErrorB("programming failed:", statusRes);
    goto _shutdown;
  }

  logInfo("success!");
  
  _shutdown: AVRProgrammer::shutdownTargetMcu();
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

void Tests_ConfFile::testConfFile() {
  logInfo(">> ConfFile");
  logFreeRam();
  testAutogenerateDirNamesByProgId();
  testSkipToValidData();
  testReadRootConfFile();
  logInfo("-- END");
  return;
}

char Tests_ConfFile::__testConfFileName[] = "TEST-XX.TXT";
boolean Tests_ConfFile::_testOpenConfFile(byte pos) {
  __testConfFileName[5] = '0' + (pos / 10); __testConfFileName[6] = '0' + (pos % 10);
  return ConfFile::openConfFile(__testConfFileName);
}

/*void Tests_ConfFile::__testStringMatches_LogDebug(char c1, char c2) {
  char str[] = "c-c";
  str[0] = c1; str[2] = c2;
  logDebugS("", str);
  return;
}

boolean Tests_ConfFile::__testStringMatches(const char* str1, const char* str2, boolean debugMode) {
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
}*/


void Tests_ConfFile::testAutogenerateDirNamesByProgId() {
  logInfo(">autogenDirNames");
  char dir1Name[8 + 4 + 1];
  char dir2Name[8 + 1];
  char s1[] = "ID_A1B2C3D4E5F6";
  char s1a[] = "ID_A1B2.___";
  char s1b[] = "C3D4E5F6";
  ConfFile::autogenerateDirNamesByProgId(s1, dir1Name, dir2Name);
  logDebugS("dir1Name", dir1Name);
  logDebugS("dir2Name", dir2Name);
  logInfo("#1a");
  UtilsTests::assertStringMatches(s1a, dir1Name, true);
  logInfo("#1b");
  UtilsTests::assertStringMatches(s1b, dir2Name, true);

  char s2[] = "MN_A1C3";
  ConfFile::autogenerateDirNamesByProgId(s2, dir1Name, dir2Name);
  logDebugS("dir1Name", dir1Name);
  logDebugS("dir2Name", dir2Name);
  logInfo("#2");
  UtilsTests::assertStringMatches(s2, dir1Name, true);
  logInfo("- END");
  return;
}

#define assertStatusOKOrGoto(statusRes, msg, _goto) if (statusRes!=0) { logErrorB(msg, statusRes); goto _goto; }
#define assertOKOrGoto(ok, msg, b, _goto) if (!ok) { logErrorB(msg, b); goto _goto; }
#define assertStatusOKOrReturn(statusRes, msg) if (statusRes!=0) { logErrorB(msg, statusRes); return; }
#define assertOKOrReturn(ok, msg, b) if (!ok) { logErrorB(msg, b); return; }
#define assertStatusOKOrReturnV(statusRes, msg, v) if (statusRes!=0) { logErrorB(msg, statusRes); return v; }
#define assertOKOrReturnV(ok, msg, b, v) if (!ok) { logErrorB(msg, b); return v; }

void Tests_ConfFile::testSkipToValidData() {
  logInfo(">skipToValidData");
  byte statusRes;
  char c;
  boolean av;

  if (!_testOpenConfFile(01)) {
    logError("#-1");
    goto _close;
  }

  // Skip to row#5, to R1_E token
  av = ConfFile_TestStub::skipToValidData(statusRes);
  assertOKOrGoto(av, "#0", av, _close);
  assertStatusOKOrGoto(statusRes, "#1", _close);
  assertOKOrGoto(ConfFile_TestStub::_test_confFileOpen(), "#2", 0, _close);
  assertOKOrGoto(ConfFile_TestStub::_test_confFileAvailable(), "#3", 0, _close);
  // now read "R1_E"
  c = ConfFile_TestStub::_test_confFileReadChar(); // R
  assertStatusOKOrGoto(statusRes, "#4", _close);
  assertOKOrGoto(c == 'R', "#5", c, _close);
  c = ConfFile_TestStub::_test_confFileReadChar(); // 1
  assertStatusOKOrGoto(statusRes, "#6", _close);
  c = ConfFile_TestStub::_test_confFileReadChar(); // _
  assertStatusOKOrGoto(statusRes, "#7", _close);
  c = ConfFile_TestStub::_test_confFileReadChar(); // E
  assertStatusOKOrGoto(statusRes, "#8", _close);
  assertOKOrGoto(c == 'E', "#9", c, _close);

  // Skip to AVR@002
  ConfFile_TestStub::skipToValidData(statusRes);
  assertStatusOKOrGoto(statusRes, "#10", _close);
  // now read "AVR@002 "
  c = ConfFile_TestStub::_test_confFileReadChar(); // R
  assertStatusOKOrGoto(statusRes, "#11", _close);
  assertOKOrGoto(c == 'A', "#12", c, _close);
  for (byte i = 0; i < 6; i++) {
    c = ConfFile_TestStub::_test_confFileReadChar(); // [VR@002]
    assertStatusOKOrGoto(statusRes, "#13", _close);
  }
  c = ConfFile_TestStub::_test_confFileReadChar(); // ' '
  assertStatusOKOrGoto(statusRes, "#14", _close);
  assertOKOrGoto(c == ' ', "#15", c, _close);
  
  // Skip to HJKHJH
  ConfFile_TestStub::skipToValidData(statusRes);
  assertStatusOKOrGoto(statusRes, "#16", _close);

  _close:
  if (!ConfFile::closeConfFile()) {
    logError("!closedFile");
  }
  logInfo("- END");
  return;
}

void Tests_ConfFile::testReadRootConfFile() {
  char expectedDirName[] = "HJKHJH";
  char expectedMcuModel[] = "AVR@002";
  char expected2DirName[] = "2";
  char expected2McuModel[] = "AVR[1e34f4]";
  char progId[] = "R1_E";
  byte statusRes;
  
  logInfo(">testRRCF");
  assertOKOrReturn(_testOpenConfFile(01), "#00", 0);
  
  ConfFile::readRootConfFile(progId, __testMcuModelBuf, __testFilePathBuf, statusRes);
  assertStatusOKOrReturn(statusRes, "#01");
  assertOKOrReturn(strLength(__testFilePathBuf) > 0, "#02", strLength(__testFilePathBuf));
  if (!UtilsTests::assertStringMatches(expectedDirName, __testFilePathBuf, true)) {
    logError("#03");
  }
  if (!UtilsTests::assertStringMatches(expectedMcuModel, __testMcuModelBuf, true)) {
    logError("#04");
  }

  // read again
  ConfFile::readRootConfFile(progId, __testMcuModelBuf, __testFilePathBuf, statusRes);
  assertStatusOKOrReturn(statusRes, "#06");
  assertOKOrReturn(strLength(__testFilePathBuf) > 0, "#07", strLength(__testFilePathBuf));
  if (!UtilsTests::assertStringMatches(expected2DirName, __testFilePathBuf, true)) {
    logError("#08");
  }
  if (!UtilsTests::assertStringMatches(expected2McuModel, __testMcuModelBuf, true)) {
    logError("#09");
  }

  if (!ConfFile::closeConfFile()) {
    logError("!closedFile");
  }
  logInfo("- END");
  return;
}

/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
///    HEX FILE
///    HEX FILE
///    HEX FILE
///    HEX FILE
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

void Tests_HEXFile::testHEXFile05() {
  logInfo(">> HEXFile");
  byte statusRes = 0;
  unsigned int testMatrix[][3] = {
   // expect|
   // length|addr|data[resSize - 2] (before last byte in data)
     {16,    256, 0x19},
     {16,    272, 0x01},
     {16,   3872, 0xB2},
     { 2,  29072, 0xEF},
     {14, 0x7232, 0x34}
  };
  logFreeRam();

  File hexF;
  HEXFile::openFile(hexF, "TEST-05.HEX", FILE_READ, statusRes);
  if (statusRes != 0) {
  	logErrorB("testHex_open:", statusRes);
  }
  byte data[16];
  byte lineType, resSize;
  unsigned int address;
  for (byte i = 0; i < (sizeof(testMatrix) / 6); i++) {
    logDebugD("readL:",i);
    HEXFile::readLine(hexF, lineType, data, 16, resSize, address, statusRes);
    if (statusRes != 0) {
      logErrorB("err:", statusRes);
    }
    if (resSize != testMatrix[i][0]) {
      logErrorD("resSize:",resSize);
    }
    if (lineType != HEXFile::LINE_TYPE_DATA) {
      logErrorD("type:",lineType);
    }
    if (address != testMatrix[i][1]) {
      logErrorD("address:",address);
    }
    if (data[resSize - 2] != testMatrix[i][2]) {
      logErrorD("data:",data[resSize - 2]);
    }
  }

  logDebug("read EOF line");
  HEXFile::readLine(hexF, lineType, data, 16, resSize, address, statusRes);
  if (statusRes != 0) {
  	logErrorB("err:", statusRes);
  }
  if (lineType != HEXFile::LINE_TYPE_EOF) {
    logErrorD("type:",lineType);
  }

  logInfo("-- END");
  return;
}

/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
///    HWInterface
///    HWInterface
///    HWInterface
///    HWInterface
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

void Tests_HWInterface::testLedsAndBtns() {
  logInfo(">>Tests_HWInterface::testLedsAndBtns");
  logInfo("-Blinking with all LEDs in a row! (one by one)");

    /*HWInterface::setLedOnOff(HWInterface::LED_AUTO, true);
    while(true) {
      HWInterface::runLeds(60); // around 1200ms
      delay(1200);
    }*/

  byte leds[] = {HWInterface::LED_RDY, HWInterface::LED_AUTO, HWInterface::LED_ERR, HWInterface::LED_OK};
  byte ledN = 0;
  while (true) {
    HWInterface::setLedOnOff(leds[ledN], true);
    byte btns = HWInterface::readButtons();
    char res[] = "U: 0; V: 0; B: 0";
    if (btns & HWInterface::BTN_UPLOAD) {res[3]='1';}
    if (btns & HWInterface::BTN_VERIFY) {res[9]='1';}
    if (btns & HWInterface::BTN_BACKUP) {res[15]='1';}
    logInfoS("", res);
    
    HWInterface::runLeds(120); // 1200+ms
    HWInterface::setLedOnOff(leds[ledN], false);

    ledN++;
    if (ledN == 4) {
      ledN = 0;
    }

    logDebugD("VERIFY:", analogRead(HWInterface::BUTTON_VERIFY__LED_ERR__LED_OK));
  }
  
  logInfo("- END");
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

void Tests_TargetProgramDetector::testTargetProgramDetector() {
  logDebug(">> TargetPD");
  __testOneWireID(2000);
  __testGetProgId(2000);
  logDebug("-- END");
}

// requires programDetecter
void Tests_TargetProgramDetector::__testOneWireID(int d) {
  logInfo(">TPD#01 - trying to read 1wire ID");
  byte statusRes;
  byte id[8];
  boolean idPresent = TargetProgramDetector::read1WireId(id, statusRes);
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
}

// requires programDetecter
void Tests_TargetProgramDetector::__testGetProgId(int d) {
  logInfo(">TPD#02 - getProgId (AUTO/MANUAL)");
  byte statusRes;
  boolean autoSelected;
  char progIdBuf[16];
  logFreeRam();
  TargetProgramDetector::getProgId(progIdBuf, autoSelected, statusRes);
  logFreeRam();
  if (statusRes != 0) {
    logErrorB("Error! ", statusRes);
  } else {
    logInfoS("Good! ", progIdBuf);
  }
}

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
}

void testUtilsAVR() {
  logInfo(">> Utils - AVR");
  __testGetAVRModelIdBySignature();
  __testGetAVRModelAndConf();
  __testGetAVRModelNameById();
  __testGetAVRModelIdByName();
  logInfo("-- END");
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
}

void __testIsWhiteChar() {
  // FIXME Implement this test
}

void __testIsValidNameChar() {
  // FIXME Implement this test
}

void __testStrLength() {
  logInfo(">utils_05");
  char s[] = "123";
  int l = strLength(s);
  if (l != 3) {
    logErrorD("strLength", l);
  }
  logInfo("- END");
}

void __testGetAVRModelIdBySignature() {
  logInfo(">utils_06");
  byte statusRes;
  // return model ID by signature
  //        byte signBytes[3];
  //        readSignatureBytes(signBytes, statusRes); checkStatus();
  //        byte modelId = getAVRModelIdBySignature(signBytes, statusRes); checkStatus();
  byte signBytes[3] = { MCU_AVR_DATA[MCU_AVR_ATmega88PA - 1][4], MCU_AVR_DATA[MCU_AVR_ATmega88PA - 1][5], MCU_AVR_DATA[MCU_AVR_ATmega88PA - 1][6] } ;
  byte modelId = UtilsAVR::getAVRModelIdBySignature(signBytes, statusRes);
  if (statusRes != 0) {
    logErrorB("#mbysign1:", statusRes);
  }
  if (modelId != MCU_AVR_ATmega88PA) {
    logErrorD("#mbysign2:", modelId);
  }
  logInfo("- END");
}

void __testGetAVRModelAndConf() {
  //inline byte getAVRModelAndConf(byte* signBytes, byte& flashPageSize, byte& flashPagesCount, byte& eepromPageSize, byte& eepromPagesCount, byte& statusRes) {
  // FIXME Implement this test
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
}


void someOldTests() {
  byte statusRes;
  byte signBytes[3];
  #if 1
  // make sure it is ATmega328P
  AVRProgrammer::readSignatureBytes(signBytes,statusRes);
  if (statusRes > 0) {
    logError("signature error!");
    return;
  }
  delay(2000);
  #endif

  #if 1
  // test example of programming
  ProgramFile_TestStub::_testProgramming(statusRes);
  if (statusRes > 0) { logErrorB("Failed!", statusRes); return; }
  logInfo("Success testing!");
  delay(2000);
  #endif
  
  #if 0
  logInfo("Reading...");
  ProgramFile::backupMcuDataToFile("RND" + String(analogRead(pinRandomRead),HEX), 
          AVR_MEM_PAGES_COUNT_256, AVR_MEM_PAGE_SIZE_64, 
          AVR_MEM_PAGES_COUNT_256, AVR_MEM_PAGE_SIZE_4, statusRes);
  if (statusRes > 0) {
    logError("Error reading!");
    return;
  }
  logInfo("Success reading!");
  delay(2000);
  #endif
}

