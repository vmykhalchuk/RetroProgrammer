#include "HWInterface.h"

// ERR() List:
// 0x3? - JKHLKJHLJHLJHJ - JKHLJKHLKJH

boolean HWInterface::__LED_RDY   = false;
boolean HWInterface::__LED_AUTO  = false;
boolean HWInterface::__LED_ERR   = false;
boolean HWInterface::__LED_OK    = false;

void HWInterface::setup() {
  pinMode(BUTTON_UPLOAD__LED_RDY__LED_AUTO, INPUT);
  pinMode(BUTTON_VERIFY__LED_ERR__LED_OK, INPUT);
  pinMode(BUTTON_BACKUP, INPUT);
}

// return 1 - UPLOAD; 2 - VERIFY; 3 - BACKUP
byte HWInterface::waitForUserCommand() {
  byte res;
  while ((res = readButtons()) == 0) {
    runLeds(1);
  }
  return res;
}

byte HWInterface::readButtons() {
  //pinMode(BUTTON_UPLOAD__LED_RDY__LED_AUTO, INPUT);
  //pinMode(BUTTON_VERIFY__LED_ERR__LED_OK, INPUT);
  //pinMode(BUTTON_BACKUP, INPUT);
  delayMicroseconds(5);
  byte res = 0;
  if (analogRead(BUTTON_UPLOAD__LED_RDY__LED_AUTO) < BUTTON_UPLOAD_VERIFY_TRESHOLD) {
    res |= BTN_UPLOAD;
  }
  if (analogRead(BUTTON_VERIFY__LED_ERR__LED_OK) < BUTTON_UPLOAD_VERIFY_TRESHOLD) {
    res |= BTN_VERIFY;
  }
  if (digitalRead(BUTTON_BACKUP) == LOW) {
    res |= BTN_BACKUP;
  }
  return res;
}

void HWInterface::setLedOnOff(byte led, boolean turnOn) {
  if ((led & LED_RDY) != 0) {
    __LED_RDY = turnOn;
  }
  if ((led & LED_AUTO) != 0) {
    __LED_AUTO = turnOn;
  }
  if ((led & LED_ERR) != 0) {
    __LED_ERR = turnOn;
  }
  if ((led & LED_OK) != 0) {
    __LED_OK = turnOn;
  }
}

void HWInterface::runLeds(byte times) {
  while (times-- > 0) {
    if (__LED_RDY) {
      pinMode(BUTTON_UPLOAD__LED_RDY__LED_AUTO, OUTPUT);
      digitalWrite(BUTTON_UPLOAD__LED_RDY__LED_AUTO, HIGH);
    }
    if (__LED_ERR) {
      pinMode(BUTTON_VERIFY__LED_ERR__LED_OK, OUTPUT);
      digitalWrite(BUTTON_VERIFY__LED_ERR__LED_OK, LOW);
    }
    delay(10);
    pinMode(BUTTON_UPLOAD__LED_RDY__LED_AUTO, INPUT);
    pinMode(BUTTON_VERIFY__LED_ERR__LED_OK, INPUT);

    if (__LED_AUTO) {
      pinMode(BUTTON_UPLOAD__LED_RDY__LED_AUTO, OUTPUT);
      digitalWrite(BUTTON_UPLOAD__LED_RDY__LED_AUTO, LOW);
    }
    if (__LED_OK) {
      pinMode(BUTTON_VERIFY__LED_ERR__LED_OK, OUTPUT);
      digitalWrite(BUTTON_VERIFY__LED_ERR__LED_OK, HIGH);
    }
    delay(10);
    pinMode(BUTTON_UPLOAD__LED_RDY__LED_AUTO, INPUT);
    pinMode(BUTTON_VERIFY__LED_ERR__LED_OK, INPUT);
  }
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void HWInterface_Test::debugWhatLedsAreOn() {
  logDebug("debugWhatLedsAreOn");
  if (HWInterface::__LED_RDY) {
    logDebug("__LED_RDY");
  }
  if (HWInterface::__LED_ERR) {
    logDebug("__LED_ERR");
  }
  if (HWInterface::__LED_AUTO) {
    logDebug("__LED_AUTO");
  }
  if (HWInterface::__LED_OK) {
    logDebug("__LED_OK");
  }
}

