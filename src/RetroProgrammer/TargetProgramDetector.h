/*
 * Detects via Automatic method or Manual program selector, which program number to upload/verify/backup to/from target MCU.
 * For backups, mcu content will be stored to backup folder, with program detected information (for later use if needed).
 * 
 * 1) Always create new instance as a first thing in your sketch to avoid uncertain conditions on TargetMCU
 */
#ifndef TargetProgramDetector_h
#define TargetProgramDetector_h

  ///// TWIKING OF THIS LIBRARY
  #define OPTIMIZE 0
  const int ONE_WIRE_TRESHOLD_HIGH = 1010;
  #define USE_ONE_WIRE_LIBRARY 0

  #include <Arduino.h>
  #include "Statuses.h"
  #include "LoggerA.h"
  #include "Utils.h"

  #define ONEWIRE_SEARCH 0
  #define ONEWIRE_CRC8_TABLE 0
  #define ONEWIRE_CRC16 0
  #include <OneWire.h>

  class TargetProgramDetector
  {
    public:

      static void __translateErrorsToDisplayErrorCode(byte err, byte& mainErrCode, byte& subErrCode, byte& okCode);
      
      //BUFFER SIZE FOR MCU MODEL CHAR STRING
      // 'TT_' + '112233445566' + '\0' - here TT is 'MN','R1', 'R2', 'ID', and 112233445566 - is a maximum length for 'ID' case; (see TargetProgramDetector#getTargetIdName)
      //          // 3 chars for type (ID_, R1_, R2_, MN_), 12 chars as a max length for ID - 6 bytes, 1 char '/0'
      static const byte PROG_ID_BUFFER_SIZE = (3+12+1);
      
      static void setup(byte pinW0, byte pinW1, byte pinW2, byte pinManualM0, byte pinManualM1);

      /* 
       *  Call:
       *  char progIdBuf[PROG_ID_BUFFER_SIZE]; // 3 chars for type (ID_, R1_, R2_, MN_), 12 chars as a max length for ID - 6 bytes, 1 char '/0'
       *  boolean autoSelected;
       *  getProgId(progIdBuf, autoSelected, statusRes);
       *  
       *  Returns:
       *  ID_HHHHHHHHHHHH - 6 bytes of ID
       *  R1_T            - Resistive ID v1 (only pin W0 active) (e.g. R1_H1, R1_H2, R1_H1
       *  R3_TUV          - Resistive ID v3 (all pin W0-W2 active)
       *  MN_MNOP         - Manual Program selected (e.g. A1C3, B2D4, etc)
       *
       *                         Where:
       *
       *                      M   N   O   P
       *                     ---------------
       *                      A   1   C   3
       *                      B   2   D   4
       *
       *                        T   U   V
       *                       -----------
       *                        E   1   1
       *                        F   2   2
       *                        G   3   3
       *                        H   4   4
       */
      static void getProgId(char* progId, boolean& autoSelected, byte& statusRes);
      
      /*
       * id - byte array of size 8
       *       if idType = 1 - then it contains:
       *             id[0] - 1Wire device family code
       *             id[1-6] - 6 bytes of 1Wire device ID
       *             id[7] - CRC for id[0-6]
       *       if idType = 2:
       *             id[0] - W0 value (0,1,2 or 3)
       *             id[1] - W1 value (0,1,2,3 or 127-N/A)
       *             id[2] - W1 value (0,1,2,3 or 127-N/A)
       * idType - 0 - none (W0 is floating, so autodetection is Off); 1 - 1WireID; 2 - ResistiveID
       * 
       * byte id[8];
       * byte idType;
       * readTargetMcuId(id, idType, statusRes);
       */
      static void readTargetMcuId(byte* data, byte& idType, byte& statusRes);

      /*
       * byte statusRes;
       * byte id[8]; // id[0] -family code; id[1-6] - 6 bytes of ID; id[7] - CRC of id[0-6]
       * boolean present = readOneWireId(id, statusRes);
       */
      static boolean read1WireId(byte* data, byte& statusRes);

    private:
      static const byte READ_ID_COMMAND = 0x33;

      static byte _pinW0;
      static byte _pinW1;
      static byte _pinW2;
      static byte _pinManualM0;
      static byte _pinManualM1;

      // returns 0 - floating - N/A; or 1 - 1Wire Device present
      static boolean reset1Wire(byte& statusRes);
      // will charge 1wire safely, and will switch pin to INPUT mode and turn-off pull-up resistor
      static void charge1Wire(int delay, byte& statusRes);
      static void write1Wire(byte b, byte& statusRes);
      static byte read1Wire(byte& statusRes);
      
      // returns 0,1,2,3 or 127-N/A
      static byte readResistiveDividerValue(byte portW, byte& statusRes);
      
      static void readManualProgrammSelector(char* data, byte& statusRes);
      static void readManualProgrammSelectorPort(char* data, byte port, byte& statusRes);
  };

#endif

