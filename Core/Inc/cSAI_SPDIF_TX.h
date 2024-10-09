//******************************************************************************
// cSAI_SPDIF_TX.h
//
// This file defines a class to handle SPDIF transmission using the SAI
// interface, inheriting from `cSAI_Handler` to manage callbacks more easily.
//******************************************************************************

#ifndef CSAISPDIFTX_H_
#define CSAISPDIFTX_H_

#include "main.h"
#include "cDeviceHandler.h"  // Base class for callback handling
#include "cMixer.h"
namespace Dad {
//***************************************************************************
// Class cSAI_SPDIF_TX
// Transmission of an SPDIF audio stream using SAI.
// This class inherits from `cSAI_Handler` for callback-based SAI handling.
//
DECLARE_DEVICE_HANDLE(SAI_HandleTypeDef, cSAIA1_Handler, SAIA1);
class cSAI_SPDIF_TX : public cSAIA1_Handler {
public:
    //---------------------------------------------------------------------
    // Constructor / Destructor
    cSAI_SPDIF_TX(){}
    virtual ~cSAI_SPDIF_TX(){}

    //---------------------------------------------------------------------
    // Initialization: Sets up the mixer and prepares the SAI for SPDIF transmission.
    //
    void Init(SAI_HandleTypeDef* phSAI, cMixer* pMixer) {
		m_pMixer = pMixer;                   // Store the mixer instance
		cSAIA1_Handler::Init(phSAI);         // Call base class initialization (register callbacks)
	}

    //---------------------------------------------------------------------
    // Starts the SPDIF transmission using DMA. It initiates the transmission
    // by calling `HAL_SAI_Transmit_DMA` with the double buffer.
    //
    inline void StartTransmit() {
        // Start the DMA transmission
        HAL_SAI_Transmit_DMA(m_phDevice, (uint8_t*)m_Buffer, TX_BUFFER_SIZE * 2);
    }

    //---------------------------------------------------------------------
    // Stops the SPDIF transmission by calling `HAL_SAI_Abort`.
    //
    inline void StopTransmit() {
           HAL_SAI_Abort(m_phDevice);
    }

protected:
    //---------------------------------------------------------------------
    // Datas

    int32_t              m_Buffer[TX_BUFFER_SIZE * 2]; // Buffer for SPDIF data

    uint64_t             m_CtCallBack=0;              // Callback counter

    cMixer*              m_pMixer = nullptr;         // Pointer to the mixer for audio data

    //---------------------------------------------------------------------
    // Overriding virtual methods from the base class to handle specific
    // transmission callbacks for SAI SPDIF.
    //
    virtual void onTransmitComplete_SAIA1() override {
    	m_pMixer->pullSamples(&m_Buffer[TX_BUFFER_SIZE]);
    	m_CtCallBack++;
    }

    virtual void onTransmitHalfComplete_SAIA1() override {
    	m_pMixer->pullSamples(m_Buffer);
    	m_CtCallBack++;
    }

    virtual void onErrorCallback_SAIA1() override {
        // Error handling logic (can be custom for SPDIF)
        while (1); // Infinite loop on error, can be replaced with more graceful error recovery
    }
};
} /* namespace Dad */
#endif /* CSAISPDIFTX_H_ */

