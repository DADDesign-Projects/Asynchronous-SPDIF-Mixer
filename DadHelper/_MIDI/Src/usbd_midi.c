//==================================================================================
//==================================================================================
// File: usbd_midi.c
// Description: USB MIDI Device Class implementation
//
//==================================================================================
//==================================================================================

#include "usbd_midi.h"
#include "usbd_ctlreq.h"

//**********************************************************************************
// Private function prototypes
//**********************************************************************************

// ---------------------------------------------------------------------------------
static uint8_t USBD_MIDI_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_MIDI_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_MIDI_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_MIDI_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_MIDI_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t *USBD_MIDI_GetFSCfgDesc(uint16_t *length);

//**********************************************************************************
// MIDI Class callbacks structure
//**********************************************************************************

// ============================================================================
USBD_ClassTypeDef USBD_MIDI =
{
    USBD_MIDI_Init,                    // Initialize MIDI interface
    USBD_MIDI_DeInit,                  // Deinitialize MIDI interface
    USBD_MIDI_Setup,                   // Handle setup requests
    NULL,                              // EP0_TxSent callback
    NULL,                              // EP0_RxReady callback
    USBD_MIDI_DataIn,                  // Data IN callback
    USBD_MIDI_DataOut,                 // Data OUT callback
    NULL,                              // SOF callback
    NULL,                              // IsoINIncomplete callback
    NULL,                              // IsoOutIncomplete callback
    USBD_MIDI_GetFSCfgDesc,            // Get FS configuration descriptor
    USBD_MIDI_GetFSCfgDesc,            // Get HS configuration descriptor
    USBD_MIDI_GetFSCfgDesc,            // Get other speed configuration descriptor
    NULL,                              // Get device qualifier descriptor
};

//**********************************************************************************
// USB MIDI Configuration Descriptor
//**********************************************************************************

#define USB_MIDI_CONFIG_DESC_SIZE 101U  // Total size of configuration descriptor

// ============================================================================
// Full Speed Configuration Descriptor
// ============================================================================
__ALIGN_BEGIN static uint8_t USBD_MIDI_CfgFSDesc[USB_MIDI_CONFIG_DESC_SIZE] __ALIGN_END =
{
    // Configuration Descriptor
    0x09,                               // bLength: 9 bytes
    USB_DESC_TYPE_CONFIGURATION,        // bDescriptorType: Configuration
    LOBYTE(USB_MIDI_CONFIG_DESC_SIZE),  // wTotalLength: Low byte
    HIBYTE(USB_MIDI_CONFIG_DESC_SIZE),  // wTotalLength: High byte
    0x02,                               // bNumInterfaces: 2 interfaces
    0x01,                               // bConfigurationValue: Configuration 1
    0x00,                               // iConfiguration: No string
    0xC0,                               // bmAttributes: Self-powered
    0x32,                               // MaxPower: 100 mA

    // Standard AC Interface Descriptor (Audio Control)
    0x09,                               // bLength: 9 bytes
    USB_DESC_TYPE_INTERFACE,            // bDescriptorType: Interface
    0x00,                               // bInterfaceNumber: Interface 0
    0x00,                               // bAlternateSetting: Alternate setting 0
    0x00,                               // bNumEndpoints: No endpoints
    0x01,                               // bInterfaceClass: Audio
    0x01,                               // bInterfaceSubClass: Audio Control
    0x00,                               // bInterfaceProtocol: No protocol
    0x00,                               // iInterface: No string

    // Class-Specific AC Interface Descriptor
    0x09,                               // bLength: 9 bytes
    0x24,                               // bDescriptorType: CS_INTERFACE
    0x01,                               // bDescriptorSubtype: HEADER
    0x00, 0x01,                         // bcdADC: Audio Device Class 1.0
    0x09, 0x00,                         // wTotalLength: 9 bytes
    0x01,                               // bInCollection: 1 interface
    0x01,                               // baInterfaceNr(1): Interface 1

    // Standard MS Interface Descriptor (MIDI Streaming)
    0x09,                               // bLength: 9 bytes
    USB_DESC_TYPE_INTERFACE,            // bDescriptorType: Interface
    0x01,                               // bInterfaceNumber: Interface 1
    0x00,                               // bAlternateSetting: Alternate setting 0
    0x02,                               // bNumEndpoints: 2 endpoints
    0x01,                               // bInterfaceClass: Audio
    0x03,                               // bInterfaceSubClass: MIDI Streaming
    0x00,                               // bInterfaceProtocol: No protocol
    0x00,                               // iInterface: No string

    // Class-Specific MS Interface Descriptor
    0x07,                               // bLength: 7 bytes
    0x24,                               // bDescriptorType: CS_INTERFACE
    0x01,                               // bDescriptorSubtype: MS_HEADER
    0x00, 0x01,                         // bcdMSC: MIDI Streaming Class 1.0
    0x41, 0x00,                         // wTotalLength: 65 bytes

    // MIDI IN Jack Descriptor (Embedded)
    0x06,                               // bLength: 6 bytes
    0x24,                               // bDescriptorType: CS_INTERFACE
    0x02,                               // bDescriptorSubtype: MIDI_IN_JACK
    0x01,                               // bJackType: EMBEDDED
    0x01,                               // bJackID: Jack 1
    0x00,                               // iJack: No string

    // MIDI IN Jack Descriptor (External)
    0x06,                               // bLength: 6 bytes
    0x24,                               // bDescriptorType: CS_INTERFACE
    0x02,                               // bDescriptorSubtype: MIDI_IN_JACK
    0x02,                               // bJackType: EXTERNAL
    0x02,                               // bJackID: Jack 2
    0x00,                               // iJack: No string

    // MIDI OUT Jack Descriptor (Embedded)
    0x09,                               // bLength: 9 bytes
    0x24,                               // bDescriptorType: CS_INTERFACE
    0x03,                               // bDescriptorSubtype: MIDI_OUT_JACK
    0x01,                               // bJackType: EMBEDDED
    0x03,                               // bJackID: Jack 3
    0x01,                               // bNrInputPins: 1 input pin
    0x02,                               // baSourceID(1): Source Jack 2
    0x01,                               // baSourcePin(1): Source Pin 1
    0x00,                               // iJack: No string

    // MIDI OUT Jack Descriptor (External)
    0x09,                               // bLength: 9 bytes
    0x24,                               // bDescriptorType: CS_INTERFACE
    0x03,                               // bDescriptorSubtype: MIDI_OUT_JACK
    0x02,                               // bJackType: EXTERNAL
    0x04,                               // bJackID: Jack 4
    0x01,                               // bNrInputPins: 1 input pin
    0x01,                               // baSourceID(1): Source Jack 1
    0x01,                               // baSourcePin(1): Source Pin 1
    0x00,                               // iJack: No string

    // Standard Bulk OUT Endpoint Descriptor
    0x09,                               // bLength: 9 bytes
    USB_DESC_TYPE_ENDPOINT,             // bDescriptorType: Endpoint
    MIDI_OUT_EP,                        // bEndpointAddress: OUT endpoint
    0x02,                               // bmAttributes: Bulk transfer
    LOBYTE(MIDI_DATA_FS_MAX_PACKET_SIZE), // wMaxPacketSize: Low byte
    HIBYTE(MIDI_DATA_FS_MAX_PACKET_SIZE), // wMaxPacketSize: High byte
    0x00,                               // bInterval: Ignored for bulk
    0x00,                               // bRefresh: Unused
    0x00,                               // bSynchAddress: Unused

    // Class-Specific MS Bulk OUT Endpoint Descriptor
    0x05,                               // bLength: 5 bytes
    0x25,                               // bDescriptorType: CS_ENDPOINT
    0x01,                               // bDescriptorSubtype: MS_GENERAL
    0x01,                               // bNumEmbMIDIJack: 1 embedded jack
    0x01,                               // baAssocJackID(1): Associated Jack 1

    // Standard Bulk IN Endpoint Descriptor
    0x09,                               // bLength: 9 bytes
    USB_DESC_TYPE_ENDPOINT,             // bDescriptorType: Endpoint
    MIDI_IN_EP,                         // bEndpointAddress: IN endpoint
    0x02,                               // bmAttributes: Bulk transfer
    LOBYTE(MIDI_DATA_FS_MAX_PACKET_SIZE), // wMaxPacketSize: Low byte
    HIBYTE(MIDI_DATA_FS_MAX_PACKET_SIZE), // wMaxPacketSize: High byte
    0x00,                               // bInterval: Ignored for bulk
    0x00,                               // bRefresh: Unused
    0x00,                               // bSynchAddress: Unused

    // Class-Specific MS Bulk IN Endpoint Descriptor
    0x05,                               // bLength: 5 bytes
    0x25,                               // bDescriptorType: CS_ENDPOINT
    0x01,                               // bDescriptorSubtype: MS_GENERAL
    0x01,                               // bNumEmbMIDIJack: 1 embedded jack
    0x03,                               // baAssocJackID(1): Associated Jack 3
};

//**********************************************************************************
// Private variables
//**********************************************************************************

static USBD_MIDI_ItfTypeDef *pMidiItf = NULL;  // MIDI interface callbacks

//**********************************************************************************
// Private methods implementation
//**********************************************************************************

// ============================================================================
// USBD_MIDI_Init
// Description: Initialize the MIDI interface
// ============================================================================
static uint8_t USBD_MIDI_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    USBD_MIDI_HandleTypeDef *hmidi;

    // Allocate memory for MIDI handle structure
    pdev->pClassData = USBD_malloc(sizeof(USBD_MIDI_HandleTypeDef));

    if (pdev->pClassData == NULL)
    {
        return (uint8_t)USBD_EMEM;  // Memory allocation failed
    }

    hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassData;

    // Open OUT endpoint for receiving MIDI data
    (void)USBD_LL_OpenEP(pdev, MIDI_OUT_EP, USBD_EP_TYPE_BULK, MIDI_DATA_FS_MAX_PACKET_SIZE);
    pdev->ep_out[MIDI_OUT_EP & 0xFU].is_used = 1U;

    // Open IN endpoint for transmitting MIDI data
    (void)USBD_LL_OpenEP(pdev, MIDI_IN_EP, USBD_EP_TYPE_BULK, MIDI_DATA_FS_MAX_PACKET_SIZE);
    pdev->ep_in[MIDI_IN_EP & 0xFU].is_used = 1U;

    // Initialize physical interface components if callbacks are registered
    if (pMidiItf != NULL)
    {
        pMidiItf->Init();
    }

    // Initialize state variables
    hmidi->txState = 0U;  // Transmission state: ready
    hmidi->rxState = 0U;  // Reception state: ready

    // Prepare OUT endpoint to receive the first packet
    (void)USBD_LL_PrepareReceive(pdev, MIDI_OUT_EP, hmidi->buffer, MIDI_DATA_FS_MAX_PACKET_SIZE);

    return (uint8_t)USBD_OK;
}

// ============================================================================
// USBD_MIDI_DeInit
// Description: DeInitialize the MIDI layer
// ============================================================================
static uint8_t USBD_MIDI_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    // Close OUT endpoint
    (void)USBD_LL_CloseEP(pdev, MIDI_OUT_EP);
    pdev->ep_out[MIDI_OUT_EP & 0xFU].is_used = 0U;

    // Close IN endpoint
    (void)USBD_LL_CloseEP(pdev, MIDI_IN_EP);
    pdev->ep_in[MIDI_IN_EP & 0xFU].is_used = 0U;

    // Deinitialize physical interface and free resources
    if (pdev->pClassData != NULL)
    {
        if (pMidiItf != NULL)
        {
            pMidiItf->DeInit();  // Call application deinitialization
        }

        (void)USBD_free(pdev->pClassData);  // Free class data memory
        pdev->pClassData = NULL;
    }

    return (uint8_t)USBD_OK;
}

// ============================================================================
// USBD_MIDI_Setup
// Description: Handle the MIDI specific requests
// ============================================================================
static uint8_t USBD_MIDI_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    USBD_StatusTypeDef ret = USBD_OK;

    // Process request based on type
    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
        case USB_REQ_TYPE_CLASS:
            // No specific class requests for MIDI
            break;

        case USB_REQ_TYPE_STANDARD:
            // Handle standard requests
            switch (req->bRequest)
            {
                case USB_REQ_GET_INTERFACE:
                    // Return current interface setting
                    if (pdev->dev_state == USBD_STATE_CONFIGURED)
                    {
                        (void)USBD_CtlSendData(pdev, (uint8_t *)&ret, 1U);
                    }
                    else
                    {
                        USBD_CtlError(pdev, req);
                        ret = USBD_FAIL;
                    }
                    break;

                case USB_REQ_SET_INTERFACE:
                    // Set interface (no action needed for MIDI)
                    if (pdev->dev_state == USBD_STATE_CONFIGURED)
                    {
                        // Interface setting accepted, no action required
                    }
                    else
                    {
                        USBD_CtlError(pdev, req);
                        ret = USBD_FAIL;
                    }
                    break;

                default:
                    // Unsupported standard request
                    USBD_CtlError(pdev, req);
                    ret = USBD_FAIL;
                    break;
            }
            break;

        default:
            // Unsupported request type
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
            break;
    }

    return (uint8_t)ret;
}

// ============================================================================
// USBD_MIDI_DataIn
// Description: Data sent on non-control IN endpoint
// ============================================================================
static uint8_t USBD_MIDI_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassData;

    if (hmidi != NULL)
    {
        hmidi->txState = 0U;  // Transmission complete, ready for next packet
        return (uint8_t)USBD_OK;
    }

    return (uint8_t)USBD_FAIL;
}

// ============================================================================
// USBD_MIDI_DataOut
// Description: Data received on non-control Out endpoint
// ============================================================================
static uint8_t USBD_MIDI_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassData;

    if (hmidi != NULL)
    {
        // Get the received data length
        hmidi->rxLength = USBD_LL_GetRxDataSize(pdev, epnum);

        // Process received MIDI data through application callback
        // This allows next USB traffic to be NAKed until processing completes
        if (pMidiItf != NULL)
        {
            pMidiItf->Receive(hmidi->buffer, hmidi->rxLength);
        }

        return (uint8_t)USBD_OK;
    }

    return (uint8_t)USBD_FAIL;
}

// ============================================================================
// USBD_MIDI_GetFSCfgDesc
// Description: Return configuration descriptor
// ============================================================================
static uint8_t *USBD_MIDI_GetFSCfgDesc(uint16_t *length)
{
    *length = (uint16_t)sizeof(USBD_MIDI_CfgFSDesc);  // Set descriptor length
    return USBD_MIDI_CfgFSDesc;                       // Return descriptor pointer
}

//**********************************************************************************
// Public methods implementation
//**********************************************************************************

// ============================================================================
// USBD_MIDI_RegisterInterface
// Description: Register MIDI interface callbacks
// ============================================================================
uint8_t USBD_MIDI_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_MIDI_ItfTypeDef *fops)
{
    if (fops == NULL)
    {
        return (uint8_t)USBD_FAIL;  // Invalid callback structure
    }

    pMidiItf = fops;  // Store application callbacks

    return (uint8_t)USBD_OK;
}

// ============================================================================
// USBD_MIDI_SetTxBuffer
// Description: Set transmission buffer (placeholder implementation)
// ============================================================================
uint8_t USBD_MIDI_SetTxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff, uint32_t length)
{
    USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassData;

    if (hmidi == NULL)
    {
        return (uint8_t)USBD_FAIL;  // MIDI handle not initialized
    }

    hmidi->txState = 1U;  // Set transmission state to busy

    return (uint8_t)USBD_OK;
}

// ============================================================================
// USBD_MIDI_SetRxBuffer
// Description: Set reception buffer (placeholder implementation)
// ============================================================================
uint8_t USBD_MIDI_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff)
{
    USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassData;

    if (hmidi == NULL)
    {
        return (uint8_t)USBD_FAIL;  // MIDI handle not initialized
    }

    // Note: Buffer is already managed in hmidi structure
    // This function serves as a placeholder for future enhancements

    return (uint8_t)USBD_OK;
}

// ============================================================================
// USBD_MIDI_TransmitPacket
// Description: Transmit packet on IN endpoint (placeholder implementation)
// ============================================================================
uint8_t USBD_MIDI_TransmitPacket(USBD_HandleTypeDef *pdev)
{
    USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassData;
    USBD_StatusTypeDef ret = USBD_BUSY;

    if (hmidi != NULL)
    {
        if (hmidi->txState == 0U)
        {
            // Transmission logic would go here
            // Data should be in buffer set by SetTxBuffer
            ret = USBD_OK;
        }
    }

    return (uint8_t)ret;
}

// ============================================================================
// USBD_MIDI_ReceivePacket
// Description: Prepare Out endpoint to receive next packet
// ============================================================================
uint8_t USBD_MIDI_ReceivePacket(USBD_HandleTypeDef *pdev)
{
    USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassData;

    if (hmidi == NULL)
    {
        return (uint8_t)USBD_FAIL;  // MIDI handle not initialized
    }

    // Prepare OUT endpoint for next reception
    (void)USBD_LL_PrepareReceive(pdev, MIDI_OUT_EP, hmidi->buffer, MIDI_DATA_FS_MAX_PACKET_SIZE);

    return (uint8_t)USBD_OK;
}

//***End of file**************************************************************
