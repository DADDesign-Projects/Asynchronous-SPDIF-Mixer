//==================================================================================
//==================================================================================
// File: usbd_midi_if.h
// Description: USB MIDI interface header for STM32
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#ifndef __USBD_MIDI_IF_H
#define __USBD_MIDI_IF_H

//**********************************************************************************
// Includes and extern definitions
//**********************************************************************************

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_midi.h"

//**********************************************************************************
// Exported variables
//**********************************************************************************

extern USBD_MIDI_ItfTypeDef USBD_MIDI_fops;  // MIDI interface callbacks

//**********************************************************************************
// Exported functions
//**********************************************************************************

uint8_t MIDI_Transmit(uint8_t *buffer, uint16_t length);  // Transmit MIDI data over USB

// =============================================================================
// MIDI USB Code Index Numbers (CIN) definitions
// =============================================================================

#define MIDI_CIN_MISC               0x00  // Miscellaneous function codes
#define MIDI_CIN_CABLE_EVENT        0x01  // Cable events
#define MIDI_CIN_2BYTE_SYS_COMMON   0x02  // System common messages (2 bytes)
#define MIDI_CIN_3BYTE_SYS_COMMON   0x03  // System common messages (3 bytes)
#define MIDI_CIN_SYSEX_START        0x04  // System exclusive start or continue
#define MIDI_CIN_SYSEX_END_1BYTE    0x05  // System exclusive end with 1 byte
#define MIDI_CIN_SYSEX_END_2BYTE    0x06  // System exclusive end with 2 bytes
#define MIDI_CIN_SYSEX_END_3BYTE    0x07  // System exclusive end with 3 bytes
#define MIDI_CIN_NOTE_OFF           0x08  // Note Off message
#define MIDI_CIN_NOTE_ON            0x09  // Note On message
#define MIDI_CIN_POLY_KEYPRESS      0x0A  // Polyphonic Key Pressure
#define MIDI_CIN_CONTROL_CHANGE     0x0B  // Control Change message
#define MIDI_CIN_PROGRAM_CHANGE     0x0C  // Program Change message
#define MIDI_CIN_CHANNEL_PRESSURE   0x0D  // Channel Pressure message
#define MIDI_CIN_PITCH_BEND         0x0E  // Pitch Bend Change message
#define MIDI_CIN_SINGLE_BYTE        0x0F  // Single byte message

//**********************************************************************************
// Helper functions for sending MIDI messages
//**********************************************************************************

// -----------------------------------------------------------------------------
// Note messages
// -----------------------------------------------------------------------------

void MIDI_SendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);       // Send Note On message
void MIDI_SendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);      // Send Note Off message

// -----------------------------------------------------------------------------
// Control and program messages
// -----------------------------------------------------------------------------

void MIDI_SendControlChange(uint8_t channel, uint8_t controller, uint8_t value);  // Send Control Change message
void MIDI_SendProgramChange(uint8_t channel, uint8_t program);                    // Send Program Change message

// -----------------------------------------------------------------------------
// Pitch and expression messages
// -----------------------------------------------------------------------------

void MIDI_SendPitchBend(uint8_t channel, uint16_t value);  // Send Pitch Bend message

#ifdef __cplusplus
}
#endif

#endif /* __USBD_MIDI_IF_H */

//***End of file**************************************************************
