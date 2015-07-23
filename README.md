# RetroProgrammer
Board for Arduino Nano to convert Arduino into handheld programmer (SD card support, Auto Target detection)

# Main features
* Fully automated, no PC required to operate
* Autodetection of target MCU (ID of target MCU can be autodetected by using extra pin(s) on ICSP). This lets you UPLOAD programm automatically and in one button press.
* Supports AVR microcontrollers (...TBD...)

# To be implemented
* CRC every HRP file before programming/verifying it!
** CRC every conf file
* Auto detection of programming speed
* Support of 3.3v target MCU
* Extend AVR supported list
* Support PDI interface (http://www.atmel.com/images/doc8282.pdf)
* PIC support
* USB mode: Upload to Target MCU from Arduino via USB cable (no SD involved)

# Optimizations
* Move statusRes as a global variable, and don't toss around all methods!
* convert initStatus, checkStatus, returnStatus into inline functions