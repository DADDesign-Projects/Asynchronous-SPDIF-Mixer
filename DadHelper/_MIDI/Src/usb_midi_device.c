//==================================================================================
//==================================================================================
// File: usb_midi_device.cpp
// Description: USB MIDI device initialization and management
//
//==================================================================================
//==================================================================================

#include "usb_midi_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_midi.h"
#include "usbd_midi_if.h"

//**********************************************************************************
// Global Variables
//**********************************************************************************

USBD_HandleTypeDef hUsbMidiDeviceFS;  // USB device handle for MIDI functionality
extern USBD_DescriptorsTypeDef FS_Midi_Desc;  // External declaration of MIDI descriptors

//**********************************************************************************
// Public Methods
//**********************************************************************************

// =============================================================================
// Function: MX_USB_MIDI_DEVICE_Init
// Description: Initializes USB MIDI device with all required components
// =============================================================================
void MX_USB_MIDI_DEVICE_Init(void)
{
    // -----------------------------------------------------------------------------
    // Step 1: Initialize USB device core with MIDI descriptors
    // -----------------------------------------------------------------------------
    if (USBD_Init(&hUsbMidiDeviceFS, &FS_Midi_Desc, DEVICE_FS) != USBD_OK)
    {
        Error_Handler();
    }

    // -----------------------------------------------------------------------------
    // Step 2: Register MIDI class with USB device
    // -----------------------------------------------------------------------------
    if (USBD_RegisterClass(&hUsbMidiDeviceFS, &USBD_MIDI) != USBD_OK)
    {
        Error_Handler();
    }

    // -----------------------------------------------------------------------------
    // Step 3: Register MIDI interface operations
    // -----------------------------------------------------------------------------
    if (USBD_MIDI_RegisterInterface(&hUsbMidiDeviceFS, &USBD_MIDI_fops) != USBD_OK)
    {
        Error_Handler();
    }

    // -----------------------------------------------------------------------------
    // Step 4: Start USB device functionality
    // -----------------------------------------------------------------------------
    if (USBD_Start(&hUsbMidiDeviceFS) != USBD_OK)
    {
        Error_Handler();
    }
}

//***End of file**************************************************************
