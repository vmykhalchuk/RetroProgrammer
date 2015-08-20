#ifndef HWInterface_h
#define HWInterface_h

  #include <Arduino.h>
  #include "LoggerA.h"
  #include "AVRConstants.h"
  #include "Utils.h"

  // Define what pins are for which button / led
  const byte BUTTON_UPLOAD__LED_RDY__LED_AUTO = A5;
  const byte BUTTON_VERIFY__LED_ERR__LED_OK = A4;
  const byte BUTTON_BACKUP = 2;
  
  const byte LED_RDY   = 0x11;
  const byte LED_AUTO  = 0x12;
  const byte LED_ERR   = 0x21;
  const byte LED_OK    = 0x22;

  const byte BTN_UPLOAD = 0x1;
  const byte BTN_VERIFY = 0x2;
  const byte BTN_BACKUP = 0x4;

  // waits for a user command and maintains LEDs statuses
  byte waitForUserCommand();

  // returns which BTN_XXXX is pressed
  byte readButtons();

  // led - LED_XXXX
  // turnOn - true if On, false if Off
  void setLedOnOff(byte led, boolean turnOn);

  void runLeds(byte times);

#endif
