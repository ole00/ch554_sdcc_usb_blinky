#ifndef USB_INTR_H
#define USB_INTR_H


/*******************************************************************************
* EP0_BUFF_SIZE: endpoint 0 buffer size
*******************************************************************************/

#ifndef EP0_BUFF_SIZE
#define EP0_BUFF_SIZE DEFAULT_ENDP0_SIZE
#endif

/*******************************************************************************
* USB_CUST_VENDOR_ID: user defined VendorId
*******************************************************************************/
#ifndef USB_CUST_VENDOR_ID
#define USB_CUST_VENDOR_ID 0xFFFF
#endif

/*******************************************************************************
* USB_CUST_VENDOR_ID: user defined Vendor name
*******************************************************************************/
#ifndef USB_CUST_VENDOR_NAME
#define USB_CUST_VENDOR_NAME_LEN 6
#define USB_CUST_VENDOR_NAME {'C','H','5','5','x', 0}
#endif

/*******************************************************************************
* USB_CUST_PRODUCT_ID: user defined ProductId
*******************************************************************************/
#ifndef USB_CUST_PRODUCT_ID
#define USB_CUST_PRODUCT_ID 0xC551
#endif

/*******************************************************************************
* USB_CUST_VENDOR_ID: user defined Product name
*******************************************************************************/
#ifndef USB_CUST_PRODUCT_NAME
#define USB_CUST_PRODUCT_NAME_LEN 11
#define USB_CUST_PRODUCT_NAME {'C','o','o','l','_', 'b', 'o', 'a', 'r', 'd', 0}
#endif

/*******************************************************************************
* USB_CUST_CONF_POWER: user defined power consumption in milli Amps
*******************************************************************************/
#ifndef USB_CUST_CONF_POWER
#define USB_CUST_CONF_POWER 100
#endif

/*******************************************************************************
* USB_CUST_EP_COUNT: number of user defined endpoints
*******************************************************************************/
#ifndef USB_CUST_EP_COUNT
    // 1 default defined endpoint 
    #define USB_CUST_EP_COUNT 1
#endif

/*******************************************************************************
* USB_CUST_EP_DEF: definition of user defined endpoints. The number must match
*                  USB_CUST_EP_COUNT value.
* Example (USB_CUST_EP_COUNT is defined as 2)
    USB_EP_DSC              ep01o;      \
    USB_EP_DSC              ep01i;      \
*******************************************************************************/
#ifndef USB_CUST_EP_DEF
#define USB_CUST_EP_DEF USB_EP_DSC ep01o;
#endif

/*******************************************************************************
* USB_CUST_EP_DESC: definition of user defined endpoint descriptors.
* Their number must match USB_CUST_EP_COUNT value.
*
* Example of 2 bulk endpoints (USB_CUST_EP_COUNT is defined as 2)
 {sizeof(USB_EP_DSC), USB_DESC_EP, USB_EP01_OUT, USB_TRNT_BULK, MY_EP_BUF_SIZE, 0x00}, \
 {sizeof(USB_EP_DSC), USB_DESC_EP, USB_EP01_IN,  USB_TRNT_BULK, MY_EP_BUF_SIZE, 0x00}  \

*******************************************************************************/
#ifndef USB_CUST_EP_DESC
#define USB_CUST_EP_DESC {sizeof(USB_EP_DSC), USB_DESC_EP, USB_EP01_OUT, USB_TRNT_CTRL, 8 , 0x00}
#endif


/*******************************************************************************
* USB_CUST_CONTROL_TRANSFER_HANDLER: user defined handler function of basic
* Vendor type control transfer send on Endpoint 0. Can also setup outgoing
* (MCU -> Host) data in Ep0Buffer and must return the length of outgoing data.
* The defined function should return 0 for no outgoing data and 0xFF for 
* unrecognised / unhandled control tranfers.  
* It can not handle incoming data (Host -> MCU) - for that purpose use
* a function defined in USB_CUST_CONTROL_DATA_HANDLER.
* Example:
* #define USB_CUST_CONTROL_TRANSFER_HANDLER myUsbControlHandler()
*******************************************************************************/
#ifndef USB_CUST_CONTROL_TRANSFER_HANDLER
#define USB_CUST_CONTROL_TRANSFER_HANDLER 0xFF
#endif

/*******************************************************************************
* USB_CUST_CONTROL_DATA_HANDLER: user defined handler function of basic
* Vendor type data transfer sent from the Host to MCU via control Endpoint 0.
* The Ep0Buffer contains data of USB_RX_LEN size. The total data size can not
* be bigger than EP0_BUFF_SIZE which is up to 64 bytes.
* Example:
* #define USB_CUST_CONTROL_DATA_HANDLER myUsbDataInHandler()
*******************************************************************************/
#ifndef USB_CUST_CONTROL_DATA_HANDLER
#define USB_CUST_CONTROL_DATA_HANDLER
#endif


/******************************************************************************/

__xdata __at (0x0000) uint8_t Ep0Buffer[EP0_BUFF_SIZE]; //Endpoint 0 OUT&IN buffer, must be an even address



/* Device Descriptor */
__code USB_DEV_DSC device_dsc =
{    
    sizeof(USB_DEV_DSC),    // Size of this descriptor in bytes
    USB_DESC_DEV,           // DEVICE descriptor type
    0x0200,                 // USB Spec Release Number in BCD format
    0xFF,                   // Class Code
    0x00,                   // Subclass code
    0x00,                   // Protocol code
    EP0_BUFF_SIZE,          // Max packet size for EP0
    USB_CUST_VENDOR_ID,     // Vendor ID
    USB_CUST_PRODUCT_ID,    // Product ID
    0x0000,                 // Device release number in BCD format
    0x01,                   // Manufacturer string index
    0x02,                   // Product string index
    0x00,                   // Device serial number string index
    0x01                    // Number of possible configurations
};

/* Configuration 1 Descriptor */


#define CFG01 struct                                \
{   USB_CFG_DSC             cd01;                   \
    USB_INTF_DSC            i00a00;                 \
    USB_CUST_EP_DEF                                 \
} cfg01


__code CFG01 =
{
    /* Configuration descriptor */
    {
        sizeof(USB_CFG_DSC),    // Size of this descriptor in bytes
        USB_DESC_CFG,           // CONFIGURATION descriptor type
        sizeof(cfg01),          // Total length of data for this cfg
        1,                      // Number of interfaces in this cfg
        1,                      // Index value of this configuration
        0,                      // Configuration string index
        USB_CONF_DEFAULT,       // Attributes, see usb_desc.h
        (USB_CUST_CONF_POWER) / 2,  // Max power consumption (2X mA)
    },    

    /* Interface descriptor */
    {
        sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
        USB_DESC_INTF,          // INTERFACE descriptor type
        0,                      // Interface Number
        0,                      // Alternate Setting Number
        USB_CUST_EP_COUNT,      // Number of endpoints in this intf
        0xFF,                   // Class code
        0x00,                   // Subclass code
        0x00,                   // Protocol code
        0,                      // Interface string index
    },

    /* User defined endpoint descriptors */
    USB_CUST_EP_DESC

};

/* String descriptors */
__code struct {uint8_t bLength; uint8_t bDscType; uint16_t string[1];} sd000 =
{
    sizeof(sd000), USB_DESC_STR,
    {0x0409}
};

__code struct {uint8_t bLength; uint8_t bDscType; uint16_t string[USB_CUST_VENDOR_NAME_LEN];} sd001 = {
    sizeof(sd001), USB_DESC_STR,
    USB_CUST_VENDOR_NAME
};

__code struct {uint8_t bLength; uint8_t bDscType; uint16_t string[USB_CUST_PRODUCT_NAME_LEN];} sd002 = {
    sizeof(sd002), USB_DESC_STR,
    USB_CUST_PRODUCT_NAME
};


uint16_t UsbIntrSetupLen;
uint8_t UsbIntrSetupReq;
uint8_t UsbIntrConfig;
const uint8_t*  UsbIntrDescr;

#define UsbSetupBuf	 ((PUSB_SETUP_REQ)Ep0Buffer)



/*******************************************************************************
* USB device configuration
*******************************************************************************/
void USBDeviceCfg()
{
	USB_CTRL = 0x00;														 //Clear USB control register
	USB_CTRL &= ~bUC_HOST_MODE;												 //This bit is the device selection mode
	USB_CTRL |=  bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN;					 //USB device and internal pull-up enable, automatically return to NAK before interrupt flag is cleared during interrupt
	USB_DEV_AD = 0x00;  													 //Device address initialization
#ifdef USB_CUST_LOW_SPEED
	USB_CTRL |= bUC_LOW_SPEED;
	UDEV_CTRL |= bUD_LOW_SPEED;										         //Choose low speed 1.5M mode
#else
	USB_CTRL &= ~bUC_LOW_SPEED;
	UDEV_CTRL &= ~bUD_LOW_SPEED;											 //Select full speed 12M mode, default mode
#endif
	UDEV_CTRL = bUD_PD_DIS;                                                  // Disable DP/DM pull-down resistor
	UDEV_CTRL |= bUD_PORT_EN;												 //Enable physical port


    // USB device mode endpoint configuration
	UEP0_DMA = (uint16_t) Ep0Buffer;						//Endpoint 0 data transfer address
	UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;				//Manual flip, OUT transaction returns ACK, IN transaction returns NAK

#ifdef USB_CUST_EP_INIT
    // call custom initialisation
    USB_CUST_EP_INIT ; 
#endif    


    // interrupt initialisation
	USB_INT_EN |= bUIE_SUSPEND;											   //Enable device suspen interrupt
	USB_INT_EN |= bUIE_TRANSFER;										   //Enable USB transfer completion interrupt
	USB_INT_EN |= bUIE_BUS_RST;											   //Enable device mode USB bus reset interrupt
	USB_INT_FG |= 0x1F;													   //Clear interrupt flag
	IE_USB = 1;															   //Enable USB interrupt
	EA = 1;																   //Allow microcontroller interrupt

	UEP0_T_LEN = 0;
	UEP1_T_LEN = 0;													       //Pre-use send length must be cleared
	UEP2_T_LEN = 0;
    UEP3_T_LEN = 0;
}


/*******************************************************************************
* CH55xUSB interrupt handler
*******************************************************************************/
void DeviceInterrupt(void) __interrupt (INT_NO_USB)					   //USB interrupt service routine, using register set 1
{
	uint16_t len;
	if(UIF_TRANSFER)															//USB transfer completion flag
	{
		switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
		{

#ifdef USB_CUST_EP1_IN_HANDLER
        case UIS_TOKEN_IN | 1:
            // run the Enpoint handler
            USB_CUST_EP1_IN_HANDLER ;
		break;
#endif 
#ifdef USB_CUST_EP1_OUT_HANDLER
        case UIS_TOKEN_OUT | 1:
            // run the Enpoint handler
            USB_CUST_EP1_OUT_HANDLER ;
		break;
#endif 
#ifdef USB_CUST_EP2_IN_HANDLER
        case UIS_TOKEN_IN | 2:
            // run the Enpoint handler
            USB_CUST_EP2_IN_HANDLER ;
		break;
#endif 
#ifdef USB_CUST_EP2_OUT_HANDLER
        case UIS_TOKEN_OUT | 2:
            // run the Enpoint handler
            USB_CUST_EP2_OUT_HANDLER ;
		break;
#endif 
#ifdef USB_CUST_EP3_IN_HANDLER
        case UIS_TOKEN_IN | 3:
            // run the Enpoint handler
            USB_CUST_EP3_IN_HANDLER ;
		break;
#endif 
#ifdef USB_CUST_EP3_OUT_HANDLER
        case UIS_TOKEN_OUT | 3:
            // run the Enpoint handler
            USB_CUST_EP3_OUT_HANDLER ;
		break;
#endif 
#ifdef USB_CUST_EP4_IN_HANDLER
        case UIS_TOKEN_IN | 4:
            // run the Enpoint handler
            USB_CUST_EP4_IN_HANDLER ;
		break;
#endif 
#ifdef USB_CUST_EP4_OUT_HANDLER
        case UIS_TOKEN_OUT | 4:
            // run the Enpoint handler
            USB_CUST_EP4_OUT_HANDLER ;
		break;
#endif 

        // configuration transfers on EP0 
		case UIS_TOKEN_SETUP | 0:												//SETUP transaction
			len = USB_RX_LEN;
			if(len == (sizeof(USB_SETUP_REQ)))
			{
				UsbIntrSetupLen = ((uint16_t)UsbSetupBuf->wLengthH<<8) | (UsbSetupBuf->wLengthL);
				len = 0;													  // The default is success and upload 0 length
				UsbIntrSetupReq = UsbSetupBuf->bRequest;

                //handle vendor defined requests				
                if ((UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK) == USB_REQ_TYP_VENDOR) {
                   len = USB_CUST_CONTROL_TRANSFER_HANDLER;
				}
                // handle standard requests
				else															 //Standard request
				{
					switch(UsbIntrSetupReq)											 //Request code
					{
					case USB_GET_DESCRIPTOR:
						switch(UsbSetupBuf->wValueH)
						{
						case 1:													   //Device descriptor
							UsbIntrDescr = (uint8_t*) device_dsc;								   //set the device descriptor to the buffer to be sent
							len = sizeof(device_dsc);
							break;
						case 2:														//Configuration descriptor
							UsbIntrDescr = (uint8_t*) cfg01;										  //set the configuration descriptor to the buffer to be sent
							len = sizeof(cfg01);
							break;
						case 3:
							if(UsbSetupBuf->wValueL == 0)
							{
								UsbIntrDescr = (uint8_t*) sd000;
								len = sizeof(sd000);
							}
							else if(UsbSetupBuf->wValueL == 1)
							{
								UsbIntrDescr = (uint8_t*) sd001;
								len = sizeof(sd001);
							}
							else if(UsbSetupBuf->wValueL == 2)
							{
								UsbIntrDescr = (uint8_t*) sd002;
								len = sizeof(sd002);
							}
							else
							{
								len = 0xFF;
							}
							break;
						default:
							len = 0xff;												//Unsupported command or error
							break;
						}
						if ( UsbIntrSetupLen > len )
						{
							UsbIntrSetupLen = len;	//Limit total length
						}
						len = UsbIntrSetupLen >= DEFAULT_ENDP0_SIZE ? DEFAULT_ENDP0_SIZE : UsbIntrSetupLen;							//This transmission length
						memcpy(Ep0Buffer,UsbIntrDescr,len);								  //copy upload data
						UsbIntrSetupLen -= len;
						UsbIntrDescr += len;
						break;
					case USB_SET_ADDRESS:
						UsbIntrSetupLen = UsbSetupBuf->wValueL;							  //Staging USB device address
						break;
					case USB_GET_CONFIGURATION:
						Ep0Buffer[0] = UsbIntrConfig;
						if ( UsbIntrSetupLen >= 1 )
						{
							len = 1;
						}
						break;
					case USB_SET_CONFIGURATION:
						UsbIntrConfig = UsbSetupBuf->wValueL;
						break;
					case USB_GET_INTERFACE:
						break;
					case USB_CLEAR_FEATURE:											//Clear Feature
						if( ( UsbSetupBuf->bRequestType & 0x1F ) == USB_REQ_RECIP_DEVICE )				  /* Clear device */
						{
							if( ( ( ( uint16_t )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x01 )
							{
								if( cfg01.cd01.bmAttributes & 0x20 )
								{
									/* wake */
								}
								else
								{
									len = 0xFF;										/* operation failed */
								}
							}
							else
							{
								len = 0xFF;											/* operation failed */
							}
						}
						else if ( ( UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP ) // Clear Endpoint
						{
							switch( UsbSetupBuf->wIndexL )
							{
#if USB_CUST_EP_COUNT >= 4
							case 0x84:
								UEP4_CTRL = UEP4_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
								break;
							case 0x04:
								UEP4_CTRL = UEP4_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
								break;
#endif
#if USB_CUST_EP_COUNT >= 3
							case 0x83:
								UEP3_CTRL = UEP3_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
								break;
							case 0x03:
								UEP3_CTRL = UEP3_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
								break;
#endif

#if USB_CUST_EP_COUNT >= 2
							case 0x82:
								UEP2_CTRL = UEP2_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
								break;
							case 0x02:
								UEP2_CTRL = UEP2_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
								break;
#endif

							case 0x81:
								UEP1_CTRL = UEP1_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
								break;
							case 0x01:
								UEP1_CTRL = UEP1_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
								break;
							default:
								len = 0xFF;										 // Unsupported endpoint
								break;
							}
						}
						else
						{
							len = 0xFF;												// anything else is unsupported
						}
						break;
					case USB_SET_FEATURE:										  /* Set Feature */
						if( ( UsbSetupBuf->bRequestType & 0x1F ) == USB_REQ_RECIP_DEVICE )				  /* Setting up the device */
						{
							if( ( ( ( uint16_t )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x01 )
							{
								if( cfg01.cd01.bmAttributes & 0x20 )
								{
									/* Sleep */
#ifdef DE_PRINTF
									printf( "suspend\r\n" );															 //Sleep state
#endif
									while ( XBUS_AUX & bUART0_TX )
									{
										;	//Waiting for transmission to complete
									}
									SAFE_MOD = 0x55;
									SAFE_MOD = 0xAA;
									WAKE_CTRL = bWAK_BY_USB | bWAK_RXD0_LO | bWAK_RXD1_LO;					  //USB or RXD0/1 activity will exit sleep state
									PCON |= PD;																 //Sleep
									SAFE_MOD = 0x55;
									SAFE_MOD = 0xAA;
									WAKE_CTRL = 0x00;
								}
								else
								{
									len = 0xFF;										/* operation failed */
								}
							}
							else
							{
								len = 0xFF;											/* operation failed */
							}
						}
						else if( ( UsbSetupBuf->bRequestType & 0x1F ) == USB_REQ_RECIP_ENDP )			 /* Set endpoint */
						{
							if( ( ( ( uint16_t )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x00 )
							{
								switch( ( ( uint16_t )UsbSetupBuf->wIndexH << 8 ) | UsbSetupBuf->wIndexL )
								{

#if USB_CUST_EP_COUNT >= 4
								case 0x84:
									UEP3_CTRL = UEP4_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* Set endpoint 4 IN STALL */
									break;
								case 0x04:
									UEP3_CTRL = UEP4_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;/* Set endpoint 4 OUT Stall */
									break;
#endif

#if USB_CUST_EP_COUNT >= 3
								case 0x83:
									UEP3_CTRL = UEP3_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* Set endpoint 3 IN STALL */
									break;
								case 0x03:
									UEP3_CTRL = UEP3_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;/* Set endpoint 3 OUT Stall */
									break;
#endif

#if USB_CUST_EP_COUNT >= 2
								case 0x82:
									UEP2_CTRL = UEP2_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* Set endpoint 2 IN STALL */
									break;
								case 0x02:
									UEP2_CTRL = UEP2_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;/* Set endpoint 2 OUT Stall */
									break;
#endif
								case 0x81:
									UEP1_CTRL = UEP1_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* Set endpoint 1 IN STALL */
									break;
								case 0x01:
									UEP1_CTRL = UEP1_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;/* Set endpoint 1 OUT Stall */
								default:
									len = 0xFF;									/* operation failed */
									break;
								}
							}
							else
							{
								len = 0xFF;									  /* operation failed */
							}
						}
						else
						{
							len = 0xFF;										  /* operation failed */
						}
						break;
					case USB_GET_STATUS:
						Ep0Buffer[0] = 0x00;
						Ep0Buffer[1] = 0x00;
						if ( UsbIntrSetupLen >= 2 )
						{
							len = 2;
						}
						else
						{
							len = UsbIntrSetupLen;
						}
						break;
					default:
						len = 0xff;													// operation failed
						break;
					}
				}
			}
			else
			{
				len = 0xff;														 //Wrong packet length
			}
			if(len == 0xff)
			{
				UsbIntrSetupReq = 0xFF;
				UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;//STALL
			}
			else if(len <= DEFAULT_ENDP0_SIZE)				//Upload data or status stage returns 0 length package
			{
				UEP0_T_LEN = len;
				UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//The default packet is DATA1, which returns a response ACK.
			}
			else
			{
                //Although it has not yet reached the status stage, 
                //it uploads 0 length data packets in advance to prevent the host from entering the status stage in advance.
				UEP0_T_LEN = 0;  
				UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//The default packet is DATA1, which returns a response ACK.
			}
			break;

        // control endpoint 0 data tranfers
		case UIS_TOKEN_IN | 0:													  //endpoint0 IN
			switch(UsbIntrSetupReq)
			{
			case USB_GET_DESCRIPTOR:
				len = UsbIntrSetupLen >= DEFAULT_ENDP0_SIZE ? DEFAULT_ENDP0_SIZE : UsbIntrSetupLen;								 //This transmission length
				memcpy( Ep0Buffer, UsbIntrDescr, len );								   //set output data
				UsbIntrSetupLen -= len;
				UsbIntrDescr += len;
				UEP0_T_LEN = len;
				UEP0_CTRL ^= bUEP_T_TOG;											 //Sync flag bit flip
				break;
			case USB_SET_ADDRESS:
				USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | UsbIntrSetupLen;
				UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
				break;
			default:
                //The status phase completes the interrupt or is forced to upload 0 length packet end control transmission
				UEP0_T_LEN = 0;
				UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
				break;
			}
			break;
		case UIS_TOKEN_OUT | 0:  // endpoint0 OUT from the Host, IN to the MCU
                // call custom data handle if it is defined
                USB_CUST_CONTROL_DATA_HANDLER;

				UEP0_T_LEN = 0;
				UEP0_CTRL |= UEP_R_RES_ACK | UEP_T_RES_ACK;  //State stage, responding to NAK in IN
			break;



		default:
			break;
		}
		UIF_TRANSFER = 0;														   //Write 0 to clear interrupt
	}
	if(UIF_BUS_RST)																 //Device Mode USB Bus Reset Interrupt
	{
#ifdef DE_PRINTF
		printf( "reset\r\n" );															 //reset state
#endif
		UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
		UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;
		UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;
		USB_DEV_AD = 0x00;
		UIF_SUSPEND = 0;
		UIF_TRANSFER = 0;
		UIF_BUS_RST = 0;															 //Clear interrupt flag
		UsbIntrConfig = 0;		  //Clear configuration value
#ifdef USB_CUST_RESET_HANDLER
        // call custom reset handler function
        USB_CUST_RESET_HANDLER ;
#endif        
	}
	if (UIF_SUSPEND)																 //USB bus suspend/wake up
	{
		UIF_SUSPEND = 0;
		if ( USB_MIS_ST & bUMS_SUSPEND )											 //suspend
		{
#ifdef DE_PRINTF
			printf( "suspend\r\n" );															 //suspend state
#endif
			while ( XBUS_AUX & bUART0_TX )
			{
				;	//Waiting for transmission to complete
			}
			SAFE_MOD = 0x55;
			SAFE_MOD = 0xAA;
			WAKE_CTRL = bWAK_BY_USB | bWAK_RXD0_LO | bWAK_RXD1_LO;					  //USB or RXD0/1 can be woken up when there is a signal
			PCON |= PD;																 //sleep
			SAFE_MOD = 0x55;
			SAFE_MOD = 0xAA;
			WAKE_CTRL = 0x00;
		}
	}
	else {																			 //Unexpected interruption, impossible situation
		USB_INT_FG = 0xFF;															 //Clear interrupt flag

	}
}


#endif /* USB_INTR_H */

