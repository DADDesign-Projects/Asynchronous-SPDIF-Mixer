//==================================================================================
//==================================================================================
// File: usbd_midi.h
// Description: USB MIDI Device Class header definitions
//
// Copyright (c) [Year] [Company/Organization].
//==================================================================================
//==================================================================================

#ifndef __USBD_MIDI_H
#define __USBD_MIDI_H

#ifdef __cplusplus
extern "C" {
#endif

//**********************************************************************************
// Includes
//**********************************************************************************

#include "usbd_ioreq.h"  // USB device I/O requests

//**********************************************************************************
// Constants and Macros
//**********************************************************************************

#define MIDI_IN_EP                    0x81U  // MIDI IN endpoint address
#define MIDI_OUT_EP                   0x01U  // MIDI OUT endpoint address
#define MIDI_DATA_FS_MAX_PACKET_SIZE  64U    // Maximum packet size for Full Speed

//**********************************************************************************
// Type Definitions
//**********************************************************************************

// ---------------------------------------------------------------------------------
// USBD_MIDI_HandleTypeDef
// Description: MIDI device handle structure
// ---------------------------------------------------------------------------------
typedef struct
{
    uint8_t buffer[MIDI_DATA_FS_MAX_PACKET_SIZE];  // Data buffer for MIDI messages
    uint32_t rxLength;                             // Received data length
    uint8_t  txState;                              // Transmission state
    uint8_t  rxState;                              // Reception state
} USBD_MIDI_HandleTypeDef;

// ---------------------------------------------------------------------------------
// USBD_MIDI_ItfTypeDef
// Description: MIDI interface callback functions structure
// ---------------------------------------------------------------------------------
typedef struct
{
    int8_t (*Init)(void);                          // Initialize MIDI interface
    int8_t (*DeInit)(void);                        // Deinitialize MIDI interface
    int8_t (*Receive)(uint8_t *Buf, uint32_t Len); // Receive MIDI data callback
} USBD_MIDI_ItfTypeDef;

//**********************************************************************************
// External Declarations
//**********************************************************************************

extern USBD_ClassTypeDef USBD_MIDI;  // MIDI class structure

//**********************************************************************************
// Public Function Prototypes
//**********************************************************************************

// ---------------------------------------------------------------------------------
uint8_t USBD_MIDI_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_MIDI_ItfTypeDef *fops);
// Description: Register MIDI interface callbacks

// ---------------------------------------------------------------------------------
uint8_t USBD_MIDI_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff);
// Description: Set reception buffer

// ---------------------------------------------------------------------------------
uint8_t USBD_MIDI_ReceivePacket(USBD_HandleTypeDef *pdev);
// Description: Prepare to receive next packet

// ---------------------------------------------------------------------------------
uint8_t USBD_MIDI_TransmitPacket(USBD_HandleTypeDef *pdev);
// Description: Transmit packet on IN endpoint

// ---------------------------------------------------------------------------------
uint8_t USBD_MIDI_SetTxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff, uint32_t length);
// Description: Set transmission buffer

#ifdef __cplusplus
}
#endif

#endif /* __USBD_MIDI_H */

//***End of file**************************************************************
