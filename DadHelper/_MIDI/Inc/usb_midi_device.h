//==================================================================================
//==================================================================================
// File: usb_device.h
// Description: USB device header file with MIDI device declarations
//
//==================================================================================
//==================================================================================

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_DEVICE__H__
#define __USB_DEVICE__H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "usbd_def.h"

//**********************************************************************************
// Global Variables
//**********************************************************************************

extern USBD_HandleTypeDef hUsbMidiDeviceFS;  // Global USB MIDI device handle

//**********************************************************************************
// Public Function Declarations
//**********************************************************************************

/** USB Device initialization function. */
void MX_USB_MIDI_DEVICE_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __USB_DEVICE__H__ */

//***End of file**************************************************************
