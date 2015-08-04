# RetroProgrammer
Board for Arduino Nano to convert Arduino into handheld programmer (SD card support, Auto Target detection)

# Idea behind
Short: You want your DIY project to be repairable by any electrician, without skills in MCUs?

Long story:
You have this device which you will share with others, or make it for your firends or family.
Still you are wandering, what would they do, if it gets broken, MCU in the device sets on fire or anything else happens that will require MCU to be replaced and firmware uploaded?
Without your help - this device is beyound repair, and your friends cannot just ask any electrician to repair it without having skills uploading frimware to your device.

However there is a solution - meet Retro Programmer!

This super-powerefull and low-cost solution will make it super-easy to upload firmware to your device.
* no need to select type of target MCU
* no need to know which firmware has to be uploaded
* no need to use PC
* no need to install uploader software
* no need to search and download your firmware
* no need to make sure many switches are in right position
* Just SINGLE BUTTON operation
* Is not limited to number of MCUs used in your project or projects

All you need is:
* RetroProgrammer Arduino shield
* Arduino Nano or similar
* MicroSD card
* 1-wire ID Tag or set of resistors (one set for every MCU)

# Yes I want it!
* You can order board from here: http://dirtypcbs.com/view.php?share=9566&accesskey=096bf07bd5bc00aa1c8b3e8581d80cb4
* Components - I might share some in very little quantity
* Firmware - pending... feel free to contact me (vmykhalchuk<at>gmail<d>com) for status
* Manual - describes how to use Retro Programmer and how to prepare your project to be supported by Retro Programmer - pending...

# History
When I was developing smart house solution, main concept that I wanted to build into it was - maintainability.
Maintainability is critical factor of having successful system.
It must be easily repairable without special skills needed.
And since Smart House says by itself - it is smart - so it must use a bunch of different microcontroller units.
The problem with MCUs is that to replace such a component on broken board - you need to have a firmware, pc, knowledge on how to upload new firmware.
This dramatically limits a number of electricians who can service your system.

As a solution, I have designed and developed this Handheld programmer shield for Arduino Nano/Micro.

# Main features
* Fully automated, no PC required to operate
* Autodetection of target MCU (ID of target MCU can be autodetected by using extra pin(s) on ICSP). This lets you UPLOAD programm automatically and in one button press.
* Supports AVR microcontrollers (...TBD...)

# Difference between Board v0.1 and v0.2
v0.2 board has support for Arduino Micro (A4 and A6 are swapped comparing to v0.1 board) which gives possibility to support arduino micro for a sake of lost manual programs choices (not possible to select C,D,3,4), only A,B,1,2 will be active.

# To be implemented
* CRC every HRP file before programming/verifying it!
** CRC every conf file
* Auto detection of programming speed
* Auto detection of power present on Target device - to avoid turning programming voltage ON
* Support of 3.3v target MCU
** How to autodetect if this is 3.3v or 5v device?
* Extend AVR supported list
* Support PDI interface (http://www.atmel.com/images/doc8282.pdf)
* PIC support
* USB mode: Upload to Target MCU from Arduino via USB cable (no SD involved)
* Add support of ZIF sockets
** can be implemented as a standalone board connected to ICSP+TD socket
** there can be also additional DIP switches to select program
** alternatively mcu with 7segment digits and roller to select program, mcu will have to emulate 1wire interface
* Add two 7segment displays, to monitor status and program selected
* Or add LCD display for this
* Add rotary decoder to select quickly programm

# To be implemented (DONE)
* Support for Arduino Micro - DONE!!!
** software has to be modified slightly
** pin A4 must be swapped with A6 (Arduino Micro has no A6 nor A7, so we will have limited Manual programs selection)
** alternative we can swap A2 with A6, so we will have limited set of target programs when using Arduino Micro
** second approach might cause target detection error because ICSP pin 10 cannot be floating if pin 9 is set to certain value, so if target device is using resistive target program selection and uses pin9 - then error will occur
** so first approach is better, however to increase amount of program selection - we can have resistive program selector on place of target resistive selector!

# Board improvements
* Add LED to show that power supply to target MCU is ON
* Add switch to turn-off power supply to target MCU (in case it uses own power supply)
** also add this option into automatic target detection algorithm

# Optimizations
* Move statusRes as a global variable, and don't toss around all methods!
* convert initStatus, checkStatus, returnStatus into inline functions
