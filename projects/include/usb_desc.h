
/******************************************************************************
 * USB Standard descriptors
 *****************************************************************************/
#ifndef USB_DESC_H
#define USB_DESC_H

/* Descriptor types */
#define USB_DESC_DEV     0x01
#define USB_DESC_CFG     0x02
#define USB_DESC_STR     0x03
#define USB_DESC_INTF    0x04
#define USB_DESC_EP      0x05

/* Endpoints */
#define USB_EP01_OUT   0x01
#define USB_EP01_IN    0x81
#define USB_EP02_OUT   0x02
#define USB_EP02_IN    0x82
#define USB_EP03_OUT   0x03
#define USB_EP03_IN    0x83
#define USB_EP04_OUT   0x04
#define USB_EP04_IN    0x84

/* Configuration attributes */
#define USB_CONF_DEFAULT    (0x01 << 7)         //default value
#define USB_CONF_SELF       (0x01 << 6)         //self-powered
#define USB_CONF_RWKU       (0x01 << 5)         //remote wake-up 

/* Endpoint transfer type */
#define USB_TRNT_CTRL       0x00                //control transfer
#define USB_TRNT_ISO        0x01                //isochronous transfer
#define USB_TRNT_BULK       0x02                //bulk transfer
#define USB_TRNT_INT        0x03                //interrupt transfer

/* Isochronous endpoint synchronization type */
#define USB_ISOS_NS         (0x00 << 2)         //no synchronization
#define USB_ISOS_AS         (0x01 << 2)         //asynchronous
#define USB_ISOS_AD         (0x02 << 2)         //adaptive
#define USB_ISOS_SY         (0x03 << 2)         //synchronous

/* Isochronous endpoint Usage Type */
#define USB_ISOU_DE         (0x00 << 4)         //data endpoint
#define USB_ISOU_FE         (0x01 << 4)         //feedback endpoint
#define USB_ISOU_IE         (0x02 << 4)         //implicit feedback data endpoint



/* Device descriptor */
typedef struct _USB_DEV_DSC
{
    uint8_t  bLength;
    uint8_t  bDscType;
    uint16_t bcdUSB;
    uint8_t  bDevCls;
    uint8_t  bDevSubCls;
    uint8_t  bDevProtocol;
    uint8_t  bMaxPktSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t  iMFR;
    uint8_t  iProduct;
    uint8_t  iSerialNum;
    uint8_t  bNumCfg;
} USB_DEV_DSC;

/* Configuration descriptor */
typedef struct _USB_CFG_DSC
{
    uint8_t bLength;
    uint8_t bDscType;
    uint16_t wTotalLength;
    uint8_t bNumIntf;
    uint8_t bCfgValue;
    uint8_t iCfg;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
} USB_CFG_DSC;

/* Interface descriptor */
typedef struct _USB_INTF_DSC
{
    uint8_t bLength;
    uint8_t bDscType;
    uint8_t bIntfNum;
    uint8_t bAltSetting;
    uint8_t bNumEPs;
    uint8_t bIntfCls;
    uint8_t bIntfSubCls;
    uint8_t bIntfProtocol;
    uint8_t iIntf;
} USB_INTF_DSC;

/* Endpoint descriptor */
typedef struct _USB_EP_DSC
{
    uint8_t bLength;
    uint8_t bDscType;
    uint8_t bEPAdr;
    uint8_t bmAttributes;
    uint16_t wMaxPktSize;
    uint8_t bInterval;
} USB_EP_DSC;

#endif /* USB_DESC_H */
