/* usb_blink_pc - control app CH55x blink demo
 *
 * Copyright (C) 2019 Ole
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Build with:
 *
 *      gcc -o usb_blink_pc usb_blink_pc.c -lusb-1.0  -lpthread -lrt
 *
 * USB lib API reference:
 * 		http://libusb.sourceforge.net/api-1.0
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#ifdef MINGW
#include <libusbx-1.0/libusb.h>
#else
#include <libusb-1.0/libusb.h>
#endif


#define VENDOR_ID 0xFFFF
#define PRODUCT_ID 0x001e

//see usb1.1 page 183: value bitmap: Host->Device, Vendor request, Recipient is interface
#define TYPE_OUT_ITF		0x41

//see usb1.1 page 183: value bitmap: Device->Host, Vendor request, Sender is interface
#define TYPE_IN_ITF		(0x41 | (1 << 7))

#define COMMAND_TOGGLE_BLINK  0xD1
#define COMMAND_READ_BLINK_TIME 0xD0
#define COMMAND_SET_BLINK_TIME 0xD3
#define COMMAND_SET_BLINK_SEQUENCE 0xD4
#define COMMAND_JUMP_TO_BOOTLOADER 0xB0

#define ACTION_PRINT_HELP			1
#define ACTION_SET_VERBOSE			2

static uint8_t descriptor[256];

static uint8_t outBuf[32]; //output (command) buffer
static uint8_t resBuf[32]; //input (response) buffer

static const char *const strings[2] = { "info", "fatal" };

// some fancy blinking sequence
static const char sequence[] = {
	0x04, // off for 4 units

	0x16, // on for 6 units
	0x06, // off for 6 units
	0x15, // on for 5 units
	0x05, // off for 5 units
	0x14, // on for 4 units
	0x04, // off for 4 units
	0x13, // on for 3 units
	0x03, // off for 3 units
	0x12, // on for 2 units
	0x02, // off for 2 units

	0x11, // on for 1 units
	0x01, // off for 1 units
	0x11, // on for 1 units
	0x01, // off for 1 units
	0x11, // on for 1 units
	0x01, // off for 1 units
	0x11, // on for 1 units
	0x01, // off for 1 units

	0x12, // on for 2 units
	0x02, // off for 2 units
	0x13, // on for 3 units
	0x03, // off for 3 units
	0x14, // on for 4 units
	0x04, // off for 4 units
	0x15, // on for 5 units
	0x05, // off for 5 units

	(1<<7) | 1, //jump to address 1
	0 //safety terminator
};

char debug = 0;
char verbose = 0;
int action = 0;
int blinkTime = 0;


static void infoAndFatal(const int s, char *f, ...) {
    va_list ap;
    va_start(ap,f);
    fprintf(stderr, "arvic-usb: %s: ", strings[s]);
    vfprintf(stderr, f, ap);
    va_end(ap);
    if (s) exit(s);
}

#define info(...)   infoAndFatal(0, __VA_ARGS__)
#define fatal(...)  infoAndFatal(1, __VA_ARGS__)

static void usage(void) {
    info("\n"
    "*** [usb-blink] **************************************************\n"
    "command line tool for testing CH55x blink demo\n"
    "ver. 0.1 \n"
    "***************************************************************\n"
    "usage: [sudo] usb-blink command [parameter]\n"
    "commands:\n"
    "  -h     : prints this help \n"
    "  -v     : set verbose mode \n"
    "  -debug : print USB library debugging info \n"
    "  -pc    : print device configuration \n"
    "  -boot  : reset the CH55x into bootloader mode \n"
    "  -w ms  : send the blink time in milliseconds to the device\n"
    "  -r     : read the current blink time from the device\n"
    "  -t     : toggle between 100 / 250 ms blink time\n"
    "  -seq   : send a blink sequnce to the device\n"
    );
    exit(1);
}

static int dumpBuffer(uint8_t* buf, int size) {
	int i;
	for (i = 0; i < size; i++) {
		printf("%02X ", buf[i]);
		if (i % 16 == 15) {
			printf("\n");
		}
	}
	printf("\n");
	return 0;
} 

static int sendControlTransfer(libusb_device_handle *h, uint8_t command, uint16_t param1, uint16_t param2, uint8_t len) {
	int ret;

	ret = libusb_control_transfer(h, TYPE_OUT_ITF, command, param1, param2, outBuf, len, 50);
	if (verbose) {
	    info("control transfer out:  result=%i \n", ret);
	}
    return ret;
}

static int recvControlTransfer(libusb_device_handle *h, uint8_t command) {
	int ret;
	memset(resBuf, 0, sizeof(resBuf));

	ret = libusb_control_transfer(h, TYPE_IN_ITF, command, 0, 0, resBuf, sizeof(resBuf), 50);
	if (verbose) {
	    info("control transfer (0x%02x) incoming:  result=%i\n", command, ret);
		dumpBuffer(resBuf, sizeof(resBuf));
	}
    return ret;
}

//try to find the blinky usb device
static libusb_device_handle* getDeviceHandle(libusb_context* c) {
	int max;
	int ret;
	int device_index = -1;
	int i;
	libusb_device** dev_list = NULL;
	struct libusb_device_descriptor des;
	struct libusb_device_handle* handle;
	

	ret = libusb_get_device_list(c, &dev_list);
	if (verbose) {
		info("total USB devices found: %i \n", ret);
	}
	max = ret;
	//print all devices
	for (i = 0; i < max; i++) {
		ret = libusb_get_device_descriptor(dev_list[i],  & des);
		if (des.idVendor == VENDOR_ID && des.idProduct == PRODUCT_ID) {
			if (verbose) {
				info("device %i  vendor=%04x, product=%04x bus:device=%i:%i\n",
						i, des.idVendor, des.idProduct,
						libusb_get_bus_number(dev_list[i]),
						libusb_get_device_address(dev_list[i])
				);
			}
			if (device_index == -1) {
				device_index = i;
			}
		}
	}

	if (device_index < 0) {
		libusb_free_device_list(dev_list, 1);
		fatal("no device found\n");
	}

	if (verbose) {
		info("using device: %i \n", device_index);
	}

	ret = libusb_open(dev_list[device_index], &handle);
	if (verbose) {
		info("open device result=%i\n", ret);
	}
	if (ret) {
		libusb_free_device_list(dev_list, 1);
		fatal("device open failed\n");
	}

	libusb_free_device_list(dev_list, 1);

	//get config
	ret = libusb_get_descriptor(handle, LIBUSB_DT_DEVICE, 0, descriptor, 18);
	if (verbose) {
		info("get device descriptor 0 result=%i\n", ret);
	}
	ret = libusb_get_descriptor(handle, LIBUSB_DT_CONFIG, 0, descriptor, 255);
	if (verbose) {
		info("get device configuration 0 result=%i\n", ret);
	}
    usleep(20*1000);

	return handle;
}


static void checkArgumentValue(int i, int argc, char** argv, char* fatalText) {
	if (i >= argc || argv[i][0] == '-') {
		fatal(fatalText);
	}
}

static void checkArguments(int argc, char** argv) {
	int i;
	char* arg;

	action = 0;

	if (argc <= 1) {
		return;
	}
	//skip argument 0 which is the program name
	//process all arguments
	for (i = 1; i < argc; i++) {
		arg = argv[i];
		//all arguments start with dash
		if (arg[0] == '-') {
			if (strcmp("-h", arg) == 0) {
				action = ACTION_PRINT_HELP;
			} else
			if (strcmp("-v", arg) == 0) {
				verbose = 1;
			} else
			if (strcmp("-debug", arg) == 0) {
				debug = 1;
			} else
			if (strcmp("-t", arg) == 0) {
				action = COMMAND_TOGGLE_BLINK;
			} else
            if (strcmp("-w", arg) == 0) {
				checkArgumentValue(i + 1, argc, argv, "-ow: missing blink time value in milli secs\n");
				action = COMMAND_SET_BLINK_TIME;
                blinkTime = (int) strtol(argv[++i], NULL, 0);
			} else
            if (strcmp("-r", arg) == 0) {
				action = COMMAND_READ_BLINK_TIME;
			} else
			if (strcmp("-seq", arg) == 0) {
				action = COMMAND_SET_BLINK_SEQUENCE;
			} else
			if (strcmp("-boot", arg) == 0) {
				action = COMMAND_JUMP_TO_BOOTLOADER;
			}
			
			else {
				fatal("unknown parameter: %s\n" , arg);
			}
		} else {
			fatal("unknown parameter: %s\n" , arg);
		}
	}
}

int main(int argc, char** argv) {
    libusb_context* c = NULL;
    libusb_device_handle *h;
    int offset = 0, size = 0;
	int ret;
	int i;

	checkArguments(argc, argv);
    if (action == 0 || action == ACTION_PRINT_HELP) {
		usage();
	}

	//initialize libusb 
    if (libusb_init(&c)) {
    	fatal("can not initialise libusb\n");
    }

	//set debugging state
	if (debug) {
	    libusb_set_debug(c, 4);
	}

	//get the handle of the connected Glo USB device
	h = getDeviceHandle(c);

	//try to detach existing kernel driver if kernel is already handling 
	//the device
    if (libusb_kernel_driver_active(h, 0) == 1) {
    	if (verbose) {
    		info("kernel driver active\n");
    	}
        if (!libusb_detach_kernel_driver(h, 0)) {
        	if (verbose) {
        		info("driver detached\n");
        	}
        }
    }


	//set the first configuration -> initialize USB device
	if (libusb_set_configuration (h, 1) != 0) {
		fatal("cannot set device configuration\n");
	}

	if (verbose) {
		info("device configuration set\n");
	}
    usleep(20*1000);

	//get the first interface of the USB configuration
    if (libusb_claim_interface(h, 0) < 0) {
		 fatal("cannot claim interface\n");
	}

    if (verbose) {
    	info("interface claimed\n");
    }

	if (libusb_set_interface_alt_setting(h, 0, 0) < 0) {
		fatal("alt setting failed\n");
	}

    switch(action) {
	case COMMAND_SET_BLINK_SEQUENCE : {
		int ret;
		int len = sizeof(sequence);
		if (len > 32) {
			fatal("The sequence is longer than 32 bytes - this would fail to play!");
		}
		memcpy(outBuf, sequence, len);
		ret = sendControlTransfer(h, COMMAND_SET_BLINK_SEQUENCE, 0, 0 , len);
		info("Set blink sequence result=%i (%s) \n", ret, ret == len ? "OK" : "Failed");
	} break;

    case COMMAND_READ_BLINK_TIME : {
        int ret = recvControlTransfer(h, COMMAND_READ_BLINK_TIME);
        if (ret != 2) {
            info("Blink time failed. result=%i\n", ret); 
        } else {
            int v = resBuf[1];
            v <<= 8;
            v |= resBuf[0];
            info("Blink time: %i\n", v); 
        }
    } break;

    case COMMAND_SET_BLINK_TIME : {
        int ret;
		ret = sendControlTransfer(h, COMMAND_SET_BLINK_TIME, blinkTime, 0 , 0);
        info("Set blink time (%i) result=%i\n", blinkTime, ret);
    } break;

	case COMMAND_TOGGLE_BLINK : {
		sendControlTransfer(h, COMMAND_TOGGLE_BLINK, 0, 0, 0);
	} break;

	case COMMAND_JUMP_TO_BOOTLOADER : {
		sendControlTransfer(h, COMMAND_JUMP_TO_BOOTLOADER, 0, 0, 0);
	} break;


    } //end of switch

    libusb_release_interface(h, 0);
    libusb_close(h);
    libusb_exit(c);
    return 0;
}
