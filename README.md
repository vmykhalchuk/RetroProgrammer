Ever wondered what happens when my DIY MCU based device:
* after a years of fruitfull work, something bad happens to MCU?
* or this device gets into hands of other DIY electrician for repairing?
- How then shell I find the binary of firmware used in this particular device?
- Is it located on my hard-drive, or on some of backup CDs, or ... baby elephant know where?
- What if I have many devices or single device with many MCUs, each requiring own firmware?
* Now I have to:
 - find required binaries
 - find proper software to upload it over my Programmer
 - make sure MCU model is same, and proper version of firmware is selected

Well, if this ever bothered you - then Retro Programmer is right for you!

## RetroProgrammer
Board for Arduino Nano to convert Arduino into handheld programmer (SD card support, Target IDentification, no PC required, easy to repeat, cheap)

### Idea behind Retro Programmer
 # Short: You want your DIY project to be repairable by any electrician, without skills in MCUs and deep knowledge of your device? Then Retro Programmer is for you!

 # Long story:
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

### What is Target IDentification (TID for short)
TID is interface with Target Board to Identify program required to upload to MCU.
TID can be of two types:
* Resistive divider - from 4 to 64 TargetIDs supported
* 1wire ID tag - any number of TaregtIDs

Resistive divider can run over Single or Three lines, each representing values from 0 (not connected), 1 to 4.
If single line is used, then only 4 TargetIDs are supported.
If 3 lines are used, then up to 4x4x4=64 TargetIDs are supported.

However in order to support Any number of TargetIDs, the best choice would be 1wire ID tag.

Thanks to TID, Programmer knows what MCU model is expected on Target board and which program to be uploaded.
This eliminates need in searching for a right firmware, and making sure correct MCU is being used.

### Yes I want it!
* DO NOT! UNDER CONSTRUCTION!!! You can order board from here: http://dirtypcbs.com/view.php?share=9566&accesskey=096bf07bd5bc00aa1c8b3e8581d80cb4
* Components - I might share some in very little quantity
* Firmware - pending... draft version see in sources, feel free to contact me (vmykhalchuk_at_gmail_dot_com) for status
* Manual - describes how to use Retro Programmer and how to prepare your project to be supported by Retro Programmer - pending...

### History
When I was developing smart house solution, main concept that I wanted to build into it was - maintainability.
Maintainability is critical factor of having successful system.
It must be easily repairable without special skills needed.
And since Smart House says by itself - it is smart - so it must use a bunch of different microcontroller units.
The problem with MCUs is that to replace such a component on broken board - you need to have a firmware, pc, knowledge on how to upload new firmware.
This dramatically limits a number of electricians who can service your system.

As a solution, I have designed and developed this Handheld programmer shield for Arduino Nano/Micro.

## Main features
* Fully automated, no PC required to operate
* Autodetection of target MCU (ID of target MCU can be autodetected by using extra pin(s) on ICSP). This lets you UPLOAD programm automatically and in one button press.
* Supports AVR microcontrollers (...TBD...)

## Difference between Board v0.1 and v0.2
* v0.2 board has support for Arduino Micro (A4 and A6 are swapped comparing to v0.1 board).
  This gives possibility to support arduino micro for a sake of lost manual programs choices (not possible to select C,D,3,4), only A,B,1,2 will be active.
* Now it also has fixed pinout of ICSP port (mirrored to represent Socket - not Header on Target board).
  This allows user to attach any other Target device with ICSP header directly to Programmer.

## To be implemented
* Add autocalibration routine to calibrate position of DIP switches
 * add same for LEDs and buttons (in case position of buttons was changed)
* CRC every HRP file before programming/verifying it!
 * CRC every conf file
* Auto detection of programming speed
* Auto detection of power present on Target device - to avoid turning programming voltage ON
* Support of 3.3v target MCU
 * How to autodetect if this is 3.3v or 5v device?
 * Then we need level shifter to work on two separate supply levels.
  - For this the ideal solution is: 74LVC2T45; 74LVCH2T45
  - Price on Aliexperess is OK for 50pieces only.
  - Or search for “dual supply level” shifter/translator.
* Extend AVR supported list
* Support PDI interface (http://www.atmel.com/images/doc8282.pdf)
* PIC support
* Support 1-wire EEPROM instead of ID (this will let manufacturer to write version of software required to be uploaded, without need to define every single ID).
 * This is more costly then ID device, however gives more flexibility and allows to support many devices of same type
* USB mode 2: Upload directly from manufacturer web-site (will connect to main website where IDs are registered, and will download program specific for given device).
 * 1-wire eeprom would be beneficial here if manufacturer produced many devices which basically require single version of firmware. So manufacturer can register unique device prototype ID which can be stored on eeprom and uploaded to every such eeprom for every similar device. Then such ID can be used to download firmware required.
* USB mode: Upload to Target MCU from Arduino dev environment via USB cable (no SD involved)
* Add support of ZIF sockets
 * can be implemented as a standalone board connected to ICSP+TD socket
 * there can be also additional DIP switches to select program
 * alternatively mcu with 7segment digits and roller to select program, mcu will have to emulate 1wire interface
* Add two 7segment displays, to monitor status and program selected
* Or add LCD display for this
* Add rotary decoder to quickly select manual programm
* Implement Stack-like error handling with arbitrary data supported for debugging

### To be implemented (DONE)
* Support for Arduino Micro - DONE!!! Board since v0.2 supports Arduino Micro or equivalent.
 * software has to be modified slightly
 * pin A4 must be swapped with A6 (Arduino Micro has no A6 nor A7, so we will have limited Manual programs selection)

## Board improvements / fixes
* Swap UPLOAD <=> BACKUP!!!! This is critical, when ERR led is lit - it reads as UPLOAD button is pressed!
  * Check what happens if other combination of LEDs is lit, which button is then assumed as pressed!
* Add jumper to block Manual program selector - when blocked - only Auto TID is expected to work, otherwise failure!
  - this can be easily done by attaching A3 to ground via jumper, then A3 will read 0, which will indicate program to not read Manual program, and rely on Auto selector only.
* Add jumper to block TargetVPP from being supplied by Programmer
  - this is for precaution, when Target board is externally powered
  - easy to implement by removing jumper which connects D3 to transistor base. When connection is broken, transistor will never open to supply TargetVPP.
  - still MCU can monitor if VPP is provided to MCU by reading voltage on ICSP_VPP directly
* Move LEDs to the edge of board, so they are move visible and distinguishable
* Fix TargetVPP - when Arduino board is not connected, or is RESET - TargetVPP is Enabled till MCU gets control over ports. This must be redesigned! Just remove one transistor, thats it.
* Add LED to show that TargetVPP is ON
* Add switch to turn-off power supply to target MCU (in case it uses own power supply)
  * additionally auto-sense voltage on VPP and disable VPP-on switch to prevent failures
  * also add this option into automatic TargetID detection algorithm - to check if it is expected to have VPP on target board or not, and what level of voltage required (5v or 3.3v)
* Add LED to show when SD card bussy (attach to CS pin)
