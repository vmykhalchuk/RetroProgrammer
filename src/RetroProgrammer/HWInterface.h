/*
 * Always call HWInterface::setup() method from your setup sketch to initialize communication ports
 */

#ifndef HWInterface_h
#define HWInterface_h

  #include <Arduino.h>
  #include "LoggerA.h"
  #include "AVRConstants.h"
  #include "Utils.h"

  class HWInterface_Test;

  class HWInterface {
    public:
    friend HWInterface_Test;
    
    // Define what pins are for which button / led
    static const byte BUTTON_UPLOAD__LED_RDY__LED_AUTO = A5;
    static const byte BUTTON_VERIFY__LED_ERR__LED_OK = A4;
    static const byte BUTTON_BACKUP = 2;
  
    static const byte LED_RDY   = 0x01;
    static const byte LED_AUTO  = 0x02;
    static const byte LED_ERR   = 0x04;
    static const byte LED_OK    = 0x08;

    static const byte BTN_UPLOAD = 0x1;
    static const byte BTN_VERIFY = 0x2;
    static const byte BTN_BACKUP = 0x4;

    static void setup();

    // waits for a user command and maintains LEDs statuses (by calling runLeds() till button is pressed)
    // returns which BTN_XXXX is pressed
    static byte waitForUserCommand();

    // returns which BTN_XXXX is pressed
    static byte readButtons();

    // led - LED_XXXX
    // turnOn - true if On, false if Off
    static void setLedOnOff(byte led, boolean turnOn);

    // run Leds for <times> cycle(s) (around 20ms)
    static void runLeds(byte times);

    private:
    static const int BUTTON_UPLOAD_VERIFY_TRESHOLD = 200; // when button pressed, voltage level drops below middle value, closer to 0. Note: 1023 - 5v
                                                         // treshold must be low, in order to avoid high noise on the line!
                                                         // this value is very hard to predict, because for red leds it should be higher, for green and blue leds - lower, etc

    static boolean __LED_RDY;
    static boolean __LED_AUTO;
    static boolean __LED_ERR;
    static boolean __LED_OK;
  };

  class HWInterface_Test {
    public:
    static void debugWhatLedsAreOn();
  };

#endif
