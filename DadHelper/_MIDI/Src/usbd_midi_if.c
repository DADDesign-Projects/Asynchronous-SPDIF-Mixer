//==================================================================================
//==================================================================================
// File: usbd_midi_if.cpp
// Description: USB MIDI interface implementation for STM32
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "usbd_midi_if.h"

//**********************************************************************************
// External variables
//**********************************************************************************

extern USBD_HandleTypeDef hUsbMidiDeviceFS;  // USB device handle

//**********************************************************************************
// Private variables
//**********************************************************************************

static uint8_t UserRxBuffer[MIDI_DATA_FS_MAX_PACKET_SIZE];  // Receive buffer for MIDI data
static uint8_t UserTxBuffer[MIDI_DATA_FS_MAX_PACKET_SIZE];  // Transmit buffer for MIDI data

//**********************************************************************************
// Private function prototypes
//**********************************************************************************

static int8_t MIDI_Init_FS(void);
static int8_t MIDI_DeInit_FS(void);
static int8_t MIDI_Receive_FS(uint8_t *Buf, uint32_t Len);

// =============================================================================
// MIDI interface callbacks
// =============================================================================

USBD_MIDI_ItfTypeDef USBD_MIDI_fops =
{
    MIDI_Init_FS,
    MIDI_DeInit_FS,
    MIDI_Receive_FS
};

//**********************************************************************************
// Private methods implementation
//**********************************************************************************

// -----------------------------------------------------------------------------
// MIDI_Init_FS: Initialize MIDI interface
// -----------------------------------------------------------------------------

// Initializes the MIDI media low layer
// Returns: USBD_OK if all operations are OK else USBD_FAIL
static int8_t MIDI_Init_FS(void)
{
    // Set Application Buffers
    USBD_MIDI_SetRxBuffer(&hUsbMidiDeviceFS, UserRxBuffer);

    // Add your initialization code here

    return (USBD_OK);
}

// -----------------------------------------------------------------------------
// MIDI_DeInit_FS: Deinitialize MIDI interface
// -----------------------------------------------------------------------------

// DeInitializes the MIDI media low layer
// Returns: USBD_OK if all operations are OK else USBD_FAIL
static int8_t MIDI_DeInit_FS(void)
{
    // Add your deinitialization code here

    return (USBD_OK);
}

// -----------------------------------------------------------------------------
// MIDI_Receive_FS: Handle received MIDI data
// -----------------------------------------------------------------------------

// Data received over USB OUT endpoint
// Buf: Buffer of data received
// Len: Number of data received (in bytes)
// Returns: Result of the operation: USBD_OK if all operations are OK else USBD_FAIL

// External callback function wrappers for cMidi class
extern void OnNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
extern void OnNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
extern void OnControlChange(uint8_t channel, uint8_t control, uint8_t value);
extern void OnProgramChange(uint8_t channel, uint8_t program);

static int8_t MIDI_Receive_FS(uint8_t *Buf, uint32_t Len)
{
    // Process received MIDI packets (in blocks of 4 bytes)
    for (uint32_t i = 0; i < Len; i += 4)
    {
        //uint8_t cable = (Buf[i] >> 4) & 0x0F;  // Cable number (unused)
        uint8_t cin = Buf[i] & 0x0F;            // Code Index Number
        uint8_t status = Buf[i + 1];            // MIDI status byte
        uint8_t channel = status & 0x0F;        // MIDI channel
        uint8_t Data1 = Buf[i + 2];             // First data byte
        uint8_t Data2 = Buf[i + 3];             // Second data byte

        // Process according to message type
        switch (cin)
        {
            case MIDI_CIN_NOTE_OFF:
                OnNoteOff(channel, Data1, Data2);
                break;

            case MIDI_CIN_NOTE_ON:
                OnNoteOn(channel, Data1, Data2);
                break;

            case MIDI_CIN_CONTROL_CHANGE:
                OnControlChange(channel, Data1, Data2);
                break;

            case MIDI_CIN_PROGRAM_CHANGE:
                OnProgramChange(channel, Data1);
                break;

            case MIDI_CIN_PITCH_BEND:
                // Process Pitch Bend
                // uint16_t bend = data1 | (data2 << 7);
                break;

            default:
                // Other message types
                break;
        }
    }

    // Prepare for next packet reception
    USBD_MIDI_SetRxBuffer(&hUsbMidiDeviceFS, UserRxBuffer);
    USBD_MIDI_ReceivePacket(&hUsbMidiDeviceFS);

    return (USBD_OK);
}

//**********************************************************************************
// Public methods implementation
//**********************************************************************************

// -----------------------------------------------------------------------------
// MIDI_Transmit: Transmit MIDI data over USB
// -----------------------------------------------------------------------------

// Transmit MIDI data over USB
// buffer: Buffer of data to transmit
// length: Number of data to transmit (must be multiple of 4)
// Returns: USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
uint8_t MIDI_Transmit(uint8_t *buffer, uint16_t length)
{
    uint8_t result = USBD_OK;
    USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)hUsbMidiDeviceFS.pClassData;

    // Check if MIDI handle is valid
    if (hmidi == NULL)
    {
        return USBD_FAIL;
    }

    // Check if transmitter is busy
    if (hmidi->txState != 0)
    {
        return USBD_BUSY;
    }

    // Copy data to transmit buffer
    for (uint16_t i = 0; i < length; i++)
    {
        UserTxBuffer[i] = buffer[i];
    }

    // Send data over USB
    USBD_MIDI_SetTxBuffer(&hUsbMidiDeviceFS, UserTxBuffer, length);
    result = USBD_LL_Transmit(&hUsbMidiDeviceFS, MIDI_IN_EP, UserTxBuffer, length);

    return result;
}

// -----------------------------------------------------------------------------
// MIDI_SendNoteOn: Send MIDI Note On message
// -----------------------------------------------------------------------------

// Send MIDI Note On message
// channel: MIDI channel (0-15)
// note: Note number (0-127)
// velocity: Velocity (0-127)
void MIDI_SendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
{
    uint8_t packet[4];

    packet[0] = (0 << 4) | MIDI_CIN_NOTE_ON;   // Cable 0 + CIN
    packet[1] = 0x90 | (channel & 0x0F);       // Note On + channel
    packet[2] = note & 0x7F;                   // Note number
    packet[3] = velocity & 0x7F;               // Velocity

    MIDI_Transmit(packet, 4);
}

// -----------------------------------------------------------------------------
// MIDI_SendNoteOff: Send MIDI Note Off message
// -----------------------------------------------------------------------------

// Send MIDI Note Off message
// channel: MIDI channel (0-15)
// note: Note number (0-127)
// velocity: Release velocity (0-127)
void MIDI_SendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity)
{
    uint8_t packet[4];

    packet[0] = (0 << 4) | MIDI_CIN_NOTE_OFF;  // Cable 0 + CIN
    packet[1] = 0x80 | (channel & 0x0F);       // Note Off + channel
    packet[2] = note & 0x7F;                   // Note number
    packet[3] = velocity & 0x7F;               // Velocity

    MIDI_Transmit(packet, 4);
}

// -----------------------------------------------------------------------------
// MIDI_SendControlChange: Send MIDI Control Change message
// -----------------------------------------------------------------------------

// Send MIDI Control Change message
// channel: MIDI channel (0-15)
// controller: Controller number (0-127)
// value: Controller value (0-127)
void MIDI_SendControlChange(uint8_t channel, uint8_t controller, uint8_t value)
{
    uint8_t packet[4];

    packet[0] = (0 << 4) | MIDI_CIN_CONTROL_CHANGE;  // Cable 0 + CIN
    packet[1] = 0xB0 | (channel & 0x0F);             // Control Change + channel
    packet[2] = controller & 0x7F;                   // Controller number
    packet[3] = value & 0x7F;                        // Controller value

    MIDI_Transmit(packet, 4);
}

// -----------------------------------------------------------------------------
// MIDI_SendProgramChange: Send MIDI Program Change message
// -----------------------------------------------------------------------------

// Send MIDI Program Change message
// channel: MIDI channel (0-15)
// program: Program number (0-127)
void MIDI_SendProgramChange(uint8_t channel, uint8_t program)
{
    uint8_t packet[4];

    packet[0] = (0 << 4) | MIDI_CIN_PROGRAM_CHANGE;  // Cable 0 + CIN
    packet[1] = 0xC0 | (channel & 0x0F);             // Program Change + channel
    packet[2] = program & 0x7F;                      // Program number
    packet[3] = 0x00;                                // Unused

    MIDI_Transmit(packet, 4);
}

// -----------------------------------------------------------------------------
// MIDI_SendPitchBend: Send MIDI Pitch Bend message
// -----------------------------------------------------------------------------

// Send MIDI Pitch Bend message
// channel: MIDI channel (0-15)
// value: Pitch bend value (0-16383, 8192 = center)
void MIDI_SendPitchBend(uint8_t channel, uint16_t value)
{
    uint8_t packet[4];

    packet[0] = (0 << 4) | MIDI_CIN_PITCH_BEND;  // Cable 0 + CIN
    packet[1] = 0xE0 | (channel & 0x0F);         // Pitch Bend + channel
    packet[2] = value & 0x7F;                    // LSB
    packet[3] = (value >> 7) & 0x7F;             // MSB

    MIDI_Transmit(packet, 4);
}

//***End of file**************************************************************
