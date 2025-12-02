//==================================================================================
//==================================================================================
// File: usbd_desc.c
// Description: USB Device Descriptors implementation for MIDI device
//
//==================================================================================
//==================================================================================

#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_conf.h"

//**********************************************************************************
// Constants and Definitions
//**********************************************************************************

#define USBD_VID                      0x0483                    // Vendor ID
#define USBD_LANGID_STRING            1036                      // Language ID
#define USBD_MANUFACTURER_STRING      "STMicroelectronics"      // Manufacturer name
#define USBD_PID_FS                   0x5740                    // Product ID for Full Speed
#define USBD_PRODUCT_STRING_FS        "SPDIF Mixer"             // Product name
#define USBD_CONFIGURATION_STRING_FS  "MIDI Config"             // Configuration string
#define USBD_INTERFACE_STRING_FS      "MIDI Interface"          // Interface string

//**********************************************************************************
// Function Prototypes
//**********************************************************************************

static void Get_SerialNum(void);                                // Get device serial number
static void IntToUnicode(uint32_t value, uint8_t * pbuf, uint8_t len);  // Convert integer to Unicode

uint8_t * USBD_Midi_FS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_Midi_FS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_Midi_FS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_Midi_FS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_Midi_FS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_Midi_FS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_Midi_FS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);

//**********************************************************************************
// Descriptor Structure
//**********************************************************************************

USBD_DescriptorsTypeDef FS_Midi_Desc =
{
    USBD_Midi_FS_DeviceDescriptor,      // Device descriptor callback
    USBD_Midi_FS_LangIDStrDescriptor,   // Language ID string descriptor callback
    USBD_Midi_FS_ManufacturerStrDescriptor,  // Manufacturer string descriptor callback
    USBD_Midi_FS_ProductStrDescriptor,  // Product string descriptor callback
    USBD_Midi_FS_SerialStrDescriptor,   // Serial string descriptor callback
    USBD_Midi_FS_ConfigStrDescriptor,   // Configuration string descriptor callback
    USBD_Midi_FS_InterfaceStrDescriptor // Interface string descriptor callback
};

// =============================================================================
// Device Descriptors
// =============================================================================

// -----------------------------------------------------------------------------
// Device Descriptor
// -----------------------------------------------------------------------------
__ALIGN_BEGIN uint8_t USBD_Midi_FS_DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END =
{
    0x12,                       /* bLength */
    USB_DESC_TYPE_DEVICE,       /* bDescriptorType */
    0x00, 0x02,                 /* bcdUSB */
    0x00,                       /* bDeviceClass */
    0x00,                       /* bDeviceSubClass */
    0x00,                       /* bDeviceProtocol */
    USB_MAX_EP0_SIZE,           /* bMaxPacketSize */
    LOBYTE(USBD_VID),           /* idVendor */
    HIBYTE(USBD_VID),           /* idVendor */
    LOBYTE(USBD_PID_FS),        /* idProduct */
    HIBYTE(USBD_PID_FS),        /* idProduct */
    0x00, 0x02,                 /* bcdDevice */
    USBD_IDX_MFC_STR,           /* Index of manufacturer string */
    USBD_IDX_PRODUCT_STR,       /* Index of product string */
    USBD_IDX_SERIAL_STR,        /* Index of serial number string */
    USBD_MAX_NUM_CONFIGURATION  /* bNumConfigurations */
};

// -----------------------------------------------------------------------------
// Language ID Descriptor
// -----------------------------------------------------------------------------
__ALIGN_BEGIN uint8_t USBD_Midi_LangIDDesc[USB_LEN_LANGID_STR_DESC] __ALIGN_END =
{
    USB_LEN_LANGID_STR_DESC,    /* bLength */
    USB_DESC_TYPE_STRING,       /* bDescriptorType */
    LOBYTE(USBD_LANGID_STRING), /* wLANGID */
    HIBYTE(USBD_LANGID_STRING)  /* wLANGID */
};

// -----------------------------------------------------------------------------
// String Descriptors
// -----------------------------------------------------------------------------
__ALIGN_BEGIN uint8_t USBD_Midi_StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END;  // String descriptor buffer

__ALIGN_BEGIN uint8_t USBD_Midi_StringSerial[USB_SIZ_STRING_SERIAL] __ALIGN_END =
{
    USB_SIZ_STRING_SERIAL,      /* bLength */
    USB_DESC_TYPE_STRING,       /* bDescriptorType */
};

// =============================================================================
// Descriptor Functions Implementation
// =============================================================================

//**********************************************************************************
// Public Methods
//**********************************************************************************

// -----------------------------------------------------------------------------
// Device Descriptor
// -----------------------------------------------------------------------------
uint8_t * USBD_Midi_FS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);              // Speed parameter not used
    *length = sizeof(USBD_Midi_FS_DeviceDesc);
    return USBD_Midi_FS_DeviceDesc;
}

// -----------------------------------------------------------------------------
// Language ID String Descriptor
// -----------------------------------------------------------------------------
uint8_t * USBD_Midi_FS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);              // Speed parameter not used
    *length = sizeof(USBD_Midi_LangIDDesc);
    return USBD_Midi_LangIDDesc;
}

// -----------------------------------------------------------------------------
// Product String Descriptor
// -----------------------------------------------------------------------------
uint8_t * USBD_Midi_FS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);              // Speed parameter not used
    // Convert product string to descriptor format
    USBD_GetString((uint8_t *)USBD_PRODUCT_STRING_FS, USBD_Midi_StrDesc, length);
    return USBD_Midi_StrDesc;
}

// -----------------------------------------------------------------------------
// Manufacturer String Descriptor
// -----------------------------------------------------------------------------
uint8_t * USBD_Midi_FS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);              // Speed parameter not used
    // Convert manufacturer string to descriptor format
    USBD_GetString((uint8_t *)USBD_MANUFACTURER_STRING, USBD_Midi_StrDesc, length);
    return USBD_Midi_StrDesc;
}

// -----------------------------------------------------------------------------
// Serial String Descriptor
// -----------------------------------------------------------------------------
uint8_t * USBD_Midi_FS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);              // Speed parameter not used
    *length = USB_SIZ_STRING_SERIAL;
    // Generate unique serial number from device ID
    Get_SerialNum();
    return (uint8_t *) USBD_Midi_StringSerial;
}

// -----------------------------------------------------------------------------
// Configuration String Descriptor
// -----------------------------------------------------------------------------
uint8_t * USBD_Midi_FS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);              // Speed parameter not used
    // Convert configuration string to descriptor format
    USBD_GetString((uint8_t *)USBD_CONFIGURATION_STRING_FS, USBD_Midi_StrDesc, length);
    return USBD_Midi_StrDesc;
}

// -----------------------------------------------------------------------------
// Interface String Descriptor
// -----------------------------------------------------------------------------
uint8_t * USBD_Midi_FS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);              // Speed parameter not used
    // Convert interface string to descriptor format
    USBD_GetString((uint8_t *)USBD_INTERFACE_STRING_FS, USBD_Midi_StrDesc, length);
    return USBD_Midi_StrDesc;
}

//**********************************************************************************
// Private Methods
//**********************************************************************************

// -----------------------------------------------------------------------------
// Get Serial Number
// -----------------------------------------------------------------------------
static void Get_SerialNum(void)
{
    uint32_t deviceserial0, deviceserial1, deviceserial2;  // Device unique IDs

    // Read device unique IDs from hardware addresses
    deviceserial0 = *(uint32_t *) DEVICE_ID1;  // First device ID
    deviceserial1 = *(uint32_t *) DEVICE_ID2;  // Second device ID
    deviceserial2 = *(uint32_t *) DEVICE_ID3;  // Third device ID

    // Combine device IDs to create unique serial number
    deviceserial0 += deviceserial2;

    // Convert device IDs to Unicode string format
    if (deviceserial0 != 0)
    {
        // Convert first part of serial number (8 characters)
        IntToUnicode(deviceserial0, &USBD_Midi_StringSerial[2], 8);
        // Convert second part of serial number (4 characters)
        IntToUnicode(deviceserial1, &USBD_Midi_StringSerial[18], 4);
    }
}

// -----------------------------------------------------------------------------
// Integer to Unicode Conversion
// -----------------------------------------------------------------------------
static void IntToUnicode(uint32_t value, uint8_t * pbuf, uint8_t len)
{
    uint8_t idx = 0;  // Loop index

    // Process each nibble of the value
    for (idx = 0; idx < len; idx++)
    {
        // Convert nibble to hexadecimal character
        if (((value >> 28)) < 0xA)
        {
            pbuf[2 * idx] = (value >> 28) + '0';  // Number 0-9
        }
        else
        {
            pbuf[2 * idx] = (value >> 28) + 'A' - 10;  // Letter A-F
        }

        value = value << 4;     // Shift to next nibble
        pbuf[2 * idx + 1] = 0; // Null terminator for Unicode
    }
}

//***End of file**************************************************************
