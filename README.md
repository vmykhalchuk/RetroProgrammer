# RetroProgrammer
Board for Arduino Nano to convert Arduino into handheld programmer (SD card support, Auto Target detection)

#Steps
* You can order board from here: http://dirtypcbs.com/view.php?share=9566&accesskey=096bf07bd5bc00aa1c8b3e8581d80cb4
* Firmware - pending...

# Main features
* Fully automated, no PC required to operate
* Autodetection of target MCU (ID of target MCU can be autodetected by using extra pin(s) on ICSP). This lets you UPLOAD programm automatically and in one button press.
* Supports AVR microcontrollers (...TBD...)

# To be implemented
* CRC every HRP file before programming/verifying it!
** CRC every conf file
* Support for Arduino Micro
** software has to be modified slightly
** pin A4 must be swapped with A6 (Arduino Micro has no A6 nor A7, so we will have limited Manual programs selection)
** alternative we can swap A2 with A6, so we will have limited set of target programs when using Arduino Micro
** second approach might cause target detection error because ICSP pin 10 cannot be floating if pin 9 is set to certain value, so if target device is using resistive target program selection and uses pin9 - then error will occur
** so first approach is better, however to increase amount of program selection - we can have resistive program selector on place of target resistive selector!
* Auto detection of programming speed
* Support of 3.3v target MCU
* Extend AVR supported list
* Support PDI interface (http://www.atmel.com/images/doc8282.pdf)
* PIC support
* USB mode: Upload to Target MCU from Arduino via USB cable (no SD involved)

# Optimizations
* Move statusRes as a global variable, and don't toss around all methods!
* convert initStatus, checkStatus, returnStatus into inline functions
