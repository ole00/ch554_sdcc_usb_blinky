# ch554_sdcc_usb_blinky
This is a demo Blinky project for CH55x MCU. It's based on ch554_sdcc sdk (https://github.com/Blinkinlabs/ch554_sdcc.git) , but improves the code clarity of the USB handling part. 


Features:

- setups GPIO and controls LED 
- setups custom USB device on CH55x MCU
- handles custom USB data transfers, in both directions (from and to the MCU)
- enters the bootloader triggered via USB control transfer
- contains PC Host program for communicating with the MCU over USB


The goal of this demo project was to improve the USB handling code, which - at the point
of writing - was hard to maintain and not very practical for beginners to reuse in other
projects. This code separates the common USB code into its own source file, which - in 
theory - does not need to be changed and should work straight away. The customistaion 
of the USB code is done via preprocessor variables which can be defined in the main.c
program, or if they are undefined, a default placeholder is used.

The customisable options are:

- endpoint0 buffer size
- vendor and product ids
- vendor and product names
- device power consumption
- USB speed (Low speed / Full speed)
- number of endpoints (optional)
- endpoint definitions (optional)
- control transfer data hanndler for endpoint0 (optional)
- endpoint data handlers (optional)

PC Host controller program:
---------------------------

The program interacts with the CH55x USB Blinky program and controlls the following properties
in order to demonstrate how to do custom data transfers between the Host (PC) and the
USB device (MCU):
- toggles the LED blinking speed (fast /slow) - a 1 byte control transfer; from Host to MCU
- sets a custom blinking speed  - a 3 byte control transfer; from Host to MCU
- reads current value of blinking speed - a 1 byte control transfer that sends back 2 bytes
  via buffer (up to 32 bytes); from MCU to Host
- transfers a bliking data sequence into MCU and starts it - a 1 byte control transfer + up
  to 32 bytes of data buffer; fom Host to MCU

Building and running:
---------------------
1) setup the ch554_sdcc sdk (https://github.com/Blinkinlabs/ch554_sdcc.git) and make sure you
   can compile and run the examples in the example directory
2) copy the 'projects' directory from this repo into the root of the ch554_sdcc sdk (examples
   and projects directories will be on the same level)
3) enter projects/usb_blink directory and run 'make' to build, then 'make flash' to upload
   the binary to your CH55x device
4) disconnect and connect CH55x device from your PC
5) enter projects/usb_blink_pc_host directory and run 'compile.sh' to produce usb_blink_pc
   executable
6) run './usb_blink_pc -h' for options how to interact with the blinky demo
