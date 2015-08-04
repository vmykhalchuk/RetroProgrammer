/*
 * Author Volodymyr Mykhalchuk (vmykhalchuk@gmail.com)
 * 
 * 1) This logger actively uses macros F() to wrap every String argument! If you wish to switch it off use macro before #include:
 *    #define LOGGER_USE_MACRO_F 0
 *    (default 1 - ON)
 *    
 * 2) Logging level can be changed by defining macro before #include:
 *    #define LOGGER_LEVEL 3
 *    where X:
 *       0 - OFF; 1 - ERROR; 2 - WARN,ERROR; 3 - INFO,WARN,ERROR; 4 - DEBUG,INFO,WARN,ERROR;
 *    (default 1 - only ERRORs)
 * 
 * 3) Always call Logger_setup() method from setup() method of your sketch!
 */

#ifndef LoggerA_h
#define LoggerA_h


  #define LOGGER_LEVEL 4

  #include <Arduino.h>
  #include "Utils.h"

  //////////////////////////////////
  // Validation part

  #ifndef LOGGER_LEVEL
    #define LOGGER_LEVEL 1
  #endif

  #ifndef LOGGER_USE_MACRO_F
    #define LOGGER_USE_MACRO_F 1
  #endif

  #if LOGGER_LEVEL < 0 || LOGGER_LEVEL > 4
    #error "Wrong LOGGER_LEVEL"
  #endif

  //////////////////////////////////
  // Implementation part

  #if LOGGER_USE_MACRO_F == 1
    #define LOGGER_F(str) F(str)
  #else
    #define LOGGER_F(str) str
  #endif

  #if LOGGER_LEVEL > 0
    #define Logger_setup() Serial.begin(9600); while (!Serial) { ; } // wait for serial port to connect. Needed for Leonardo only
  #else
    #define Logger_setup() ;
  #endif

  //////////////////////////////////
  // Define stubs (empty methods)
  
  #define logDebug(str) ;
  #define logDebugB(str,b) ;
  #define logDebugD(str,d) ;
  #define logDebugS(str,s) ;
  #define logInfo(str) ;
  #define logInfoB(str,b) ;
  #define logInfoD(str,d) ;
  #define logInfoS(str,s) ;
  #define logWarn(str) ;
  #define logWarnB(str,b) ;
  #define logWarnD(str,d) ;
  #define logError(str) ;
  #define logErrorB(str,b) ;
  #define logErrorD(str,d) ;
    
  //////////////////////////////////
  // Define Active methods


  //#define delayLog() delay(100);
  #define delayLog() ;

  #define _logAbstract(type, str)     { Serial.print(LOGGER_F(type)); delayLog(); Serial.println(LOGGER_F(str)); delayLog(); }
  #define _logAbstractB(type, str, b) { Serial.print(LOGGER_F(type)); delayLog(); Serial.print(LOGGER_F(str)); delayLog(); Serial.print(LOGGER_F("0x")); delayLog(); Serial.println(b, HEX); delayLog(); }
  #define _logAbstractD(type, str, d) { Serial.print(LOGGER_F(type)); delayLog(); Serial.print(LOGGER_F(str)); delayLog(); Serial.println(d, DEC); delayLog(); }
  #define _logAbstractS(type, str, s) { Serial.print(LOGGER_F(type)); delayLog(); Serial.print(LOGGER_F(str)); delayLog(); Serial.println(s); delayLog(); }

  #if LOGGER_LEVEL >= 1
    #define logError(str)     _logAbstract("E: ", str);
    #define logErrorB(str,b)  _logAbstractB("E: ", str, b);
    #define logErrorD(str,d)  _logAbstractD("E: ", str, d);
    #define logErrorS(str,s)  _logAbstractS("E: ", str, s);
  #endif

  #if LOGGER_LEVEL >= 2
    #define logWarn(str)     _logAbstract("W: ", str);
    #define logWarnB(str,b)  _logAbstractB("W: ", str, b);
    #define logWarnD(str,d)  _logAbstractD("W: ", str, d);
    #define logWarnS(str,s)  _logAbstractS("W: ", str, s);
  #endif
  #if LOGGER_LEVEL >= 3
    #define logInfo(str)     _logAbstract("I: ", str);
    #define logInfoB(str,b)  _logAbstractB("I: ", str, b);
    #define logInfoD(str,d)  _logAbstractD("I: ", str, d);
    #define logInfoS(str,s)  _logAbstractS("I: ", str, s);
  #endif
  #if LOGGER_LEVEL >= 4
    #define logDebug(str)     _logAbstract("D: ", str);
    #define logDebugB(str,b)  _logAbstractB("D: ", str, b);
    #define logDebugD(str,d)  _logAbstractD("D: ", str, d);
    #define logDebugS(str,s)  _logAbstractS("D: ", str, s);
  #endif


/*
  //////////////////////////////////
  // Define stubs (empty methods)
  
  #if LOGGER_LEVEL < 4
    #define logDebug(str) ;
    #define logDebugB(str,b) ;
    #define logDebugD(str,d) ;
  #endif
  
  #if LOGGER_LEVEL < 3
    #define logInfo(str) ;
    #define logInfoB(str,b) ;
    #define logInfoD(str,d) ;
    #define logInfoS(str,s) ;
  #endif
  
  #if LOGGER_LEVEL < 2
    #define logWarn(str) ;
    #define logWarnB(str,b) ;
    #define logWarnD(str,d) ;
  #endif
  
  #if LOGGER_LEVEL < 1
    #define logError(str) ;
    #define logErrorB(str,b) ;
    #define logErrorD(str,d) ;
  #endif
*/
  
#endif
