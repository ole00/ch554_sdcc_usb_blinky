// Blink a LED connected to pin 1.4
// an control the blink speed via USB

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_ENDP0_SIZE 32

#include <ch554.h>
#include <ch554_usb.h>
#include <bootloader.h>
#include <debug.h>

// standard USB descriptor definitions
#include "usb_desc.h"

// custom USB definitions, must be set before the "usb_intr.h" is included



#define USB_CUST_PRODUCT_ID                 0x001e
#define USB_CUST_CONF_POWER                 120
//#define USB_CUST_PRODUCT_NAME               { 'B', 'l', 'i', 'n', 'k', 'y', 0 }
#define USB_CUST_PRODUCT_NAME               u"Blinky"
#define USB_CUST_CONTROL_TRANSFER_HANDLER   handleVendorControlTransfer()
#define USB_CUST_CONTROL_DATA_HANDLER       handleVendorDataTransfer()

// function declaration for custom USB transfer handlers
static uint16_t handleVendorControlTransfer();
static void handleVendorDataTransfer();

// USB interrupt handlers - does the most of the USB grunt work
#include "usb_intr.h"

// GPIO setup
#define PORT1 0x90
#define PORT3 0xb0

#define LED_PIN 4
SBIT(LED, PORT1, LED_PIN);

#define COMMAND_TOGGLE_BLINK  0xD1
#define COMMAND_READ_BLINK_TIME 0xD0
#define COMMAND_SET_BLINK_TIME 0xD3
#define COMMAND_SET_BLINK_SEQUENCE 0xD4
#define COMMAND_JUMP_TO_BOOTLOADER 0xB0

__xdata __at (0x0020) uint8_t seqBuf[DEFAULT_ENDP0_SIZE]; 

volatile __idata uint16_t blinkTime = 250;
volatile __idata uint8_t command;



/*******************************************************************************
* Jump to bootloader
*******************************************************************************/
static void jumpToBootloader()
{
    USB_INT_EN = 0;
    USB_CTRL = 0x6;
    EA = 0;
    mDelaymS(100);
    bootloader();
    while(1);
}

/*******************************************************************************
* Handler of the vendor Control transfer requests sent from the Host to 
* Endpoint 0
*
* Returns : the length of the response that is stored in Ep0Buffer  
*******************************************************************************/

static uint16_t handleVendorControlTransfer()
{
    switch (UsbIntrSetupReq) {
    // read blink time and send it back to the Host
    case COMMAND_READ_BLINK_TIME : {
        uint16_t* dst = (uint16_t*) Ep0Buffer;
        *dst = blinkTime; // write the blikTime to the Ep0buffer
        return 2; // request to transfer 2 bytes back to the host
    }; break;

    // toggle blink time
    case COMMAND_TOGGLE_BLINK : {
        blinkTime = (blinkTime == 250) ? 100 : 250;
    } break;

    //set blink time
    case COMMAND_SET_BLINK_TIME : {
        // read the value from the wValue of the control transfer
        blinkTime = ((uint16_t)UsbSetupBuf->wValueH<<8) | (UsbSetupBuf->wValueL);;      
    } break;
    case COMMAND_SET_BLINK_SEQUENCE : {
        //nothing to do, just wait for the data and confirm this transfer by returning 0
    } break;
    //jump to bootloader - remotely triggered from the Host!
    case COMMAND_JUMP_TO_BOOTLOADER : {
        jumpToBootloader();
    } break;
    default:
        return 0xFF; // Command not supported
    } // end of the switch
    command = 0;
    return 0; // no data to transfer back to the host
}

static void handleVendorDataTransfer()
{
    switch (UsbIntrSetupReq) {
        // Ah! The data for blink sequence arrived.
        case COMMAND_SET_BLINK_SEQUENCE : {
            // copy the contents of the EP0 buffer into the sequence buffer
            memcpy(seqBuf, Ep0Buffer, USB_RX_LEN);
            command = COMMAND_SET_BLINK_SEQUENCE;
            blinkTime = 0; //interrupt the default blinking
        } break;
    }
}

static void setupGPIO()
{
    // Configure pin 1.4 as GPIO output
    P1_DIR_PU = 0;
    P1_MOD_OC &=  ~(1 << LED_PIN);
    P1_DIR_PU |= (1 << LED_PIN);

}

// this delay can be interrupted, so we can exit earlier if needed
static void delayNonBlocking(uint16_t d)
{
    uint16_t bt = blinkTime;

    while (d > 100) {
        mDelaymS(100);
        // delay interrupted when a new value is set into the blinkTime 
        if (bt != blinkTime) {
            return;
        }
        d -= 100;
    }
    if (d) {
       mDelaymS(d);
    }
}

static void playBlinkySequence()
{
    uint8_t seqPos = 0;

    // re-initialise blinkTime so we can detect an interrupt within the non-blocking delay    
    blinkTime = 100;

    // play the whole sequence buffer
    while (seqPos < DEFAULT_ENDP0_SIZE) {
        uint16_t opcode = seqBuf[seqPos++];
        //early exit on 0 sequence 'opcode' or when the playback was cancelled
        if (0 == opcode || COMMAND_SET_BLINK_SEQUENCE != command) {
            break;
        }
        // handle the 'jump' opcode
        if (opcode & (1<<7)) {
            seqPos = opcode & 0x1F; // jump only to a sequence offset between 0-32 
        } else {
            //turn the LED on or off and then wait
            LED = (opcode & 0x10) ? 1 : 0;
            delayNonBlocking((opcode & 0xF) << 6); // delay in units of 64 milliseconds
        }
    }

    //turn off the led
    LED = 0;

    command = 0;
}

void main() {

    CfgFsys();   // CH55x main frequency setup
    mDelaymS(5); // wait for the internal crystal to stabilize.

    // configure GPIO ports
    setupGPIO();

    // configure USB
    USBDeviceCfg();
 
    while (1) {
        if (command == COMMAND_SET_BLINK_SEQUENCE) {
            //this will block until the sequence is finished
            playBlinkySequence();
        }
        delayNonBlocking(blinkTime);
        LED = !LED; 
    }
}
