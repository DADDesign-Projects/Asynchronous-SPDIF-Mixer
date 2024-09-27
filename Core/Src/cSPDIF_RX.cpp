//***************************************************************************
// cSPDIFRX.cpp
//
// This file defines the implementation of the `cSPDIF_RX` class, which manages
// the reception of S/PDIF audio streams.
//
//***************************************************************************

#include <cSPDIF_RX.h>
#include <string.h>

namespace Dad {
extern "C" {
//***************************************************************************
// Global variable for the timer used in interrupt management.
// This variable is linked to the interrupt handler in `stm32h7xx_it.c` to manage
// timer-related operations, specifically for TIM6 in this implementation.
TIM_HandleTypeDef* __phTIM6;
}

//***************************************************************************
// Class cSPDIF_RX
// Reception of S/PDIF stream
//

//---------------------------------------------------------------------------
// Initialization of the class
// This method initializes the S/PDIF receiver and timer used for managing
// the reception and synchronization of S/PDIF data.
//
// pMixer - Mixer instance
// Freq_SPDiff_Clk - Input clock frequency for the S/PDIF peripheral
//
void cSPDIF_RX::Init(SPDIFRX_HandleTypeDef* phSPDIFRX, TIM_HandleTypeDef* phTIM, cMixer* pMixer, uint32_t Freq_SPDiff_Clk) {
    // Store the clock frequency for S/PDIF
    m_Freq_SPDiff_Clk = Freq_SPDiff_Clk;

    // Store the mixer instance
    m_pMixer = pMixer;

    // Link global timer handler to TIM6 (used for periodic interrupts)
    __phTIM6 = &m_hTIMMod.hTIM;

    // Initialize internal variables
    m_CtCallBack = 0;           		// Reset the callback counter
    m_EtatSPDif = eEtatSPDif::inactive; // Initial state: inactive
    m_SPDiff_SampleRate = 0;    		// Reset the sample rate

    // Initialize S/PDIF callbacks
    cSPDIFRX_Handler::Init(phSPDIFRX);

    // Initialize TIM callbacks and state machine
    cTIM_Handler::Init(phTIM);

    // Start the timer interrupt (periodic callbacks every 100ms)
    HAL_TIM_Base_Start_IT(&m_hTIMMod.hTIM);
}

//---------------------------------------------------------------------------
// Timer callback (100 ms)
// Called every time the timer period elapses. This handles the state machine
// for S/PDIF synchronization. The state machine has three states:
//
// - stop     : Stop recever
// - inactive : Nothing
// - init     : Reinitialization of the S/PDIF receiver
// - synchro  : Waiting for synchronization with the incoming S/PDIF stream
// - run      : Monitoring the state of the S/PDIF synchronization for errors
//
void cSPDIF_RX::onPeriodElapsed() {
    switch (m_EtatSPDif) {
    // stop state
    case eEtatSPDif::stop:
    	HAL_DMA_Abort_IT(m_hSPDIFRXMod.hSPDIFRX.hdmaDrRx);      	// Abort DMA reception
        __HAL_SPDIFRX_IDLE(&m_hSPDIFRXMod.hSPDIFRX);              	// Set SPDIFRX to idle state
        m_EtatSPDif = eEtatSPDif::inactive;  						// Move to the inactive state
        break;

    // stop state
    case eEtatSPDif::inactive:
        break;														// Nothing

    // Initialise state: Reset the S/PDIF receiver and prepare for synchronization
    case eEtatSPDif::init:
    	HAL_DMA_Abort_IT(m_hSPDIFRXMod.hSPDIFRX.hdmaDrRx);      	// Abort DMA reception
        __HAL_SPDIFRX_IDLE(&m_hSPDIFRXMod.hSPDIFRX);              	// Set SPDIFRX to idle state
        m_hSPDIFRXMod.hSPDIFRX.State = HAL_SPDIFRX_STATE_READY; 	// Reset SPDIFRX state
        __HAL_SPDIFRX_SYNC(&m_hSPDIFRXMod.hSPDIFRX);              	// Initiate synchronization
        m_EtatSPDif = eEtatSPDif::synchro;  						// Move to the synchro state
        break;

    // synchro state: Wait for synchronization with the incoming S/PDIF signal
    case eEtatSPDif::synchro: // synchro state: Wait for synchronization with the incoming S/PDIF signal
        if (__HAL_SPDIFRX_GET_FLAG(&m_hSPDIFRXMod.hSPDIFRX, SPDIFRX_FLAG_SYNCD)) {
            // If synchronization is detected, start DMA reception
            HAL_SPDIFRX_ReceiveDataFlow_DMA(&m_hSPDIFRXMod.hSPDIFRX, (uint32_t*)m_Buffer, RX_BUFFER_SIZE * 2);

            // Calculate the sample rate of the incoming S/PDIF stream
            CalcSampleRate();

            // Move to the run state to monitor the stream
            m_EtatSPDif = eEtatSPDif::run;
        } else {
            // If no synchronization is detected, reset to stop state
            m_EtatSPDif = eEtatSPDif::init;
        }
        break;

    // run state: Monitor the S/PDIF synchronization and handle errors
    case eEtatSPDif::run:
        uint32_t Err = (m_hSPDIFRXMod.hSPDIFRX.Instance->SR) & (SPDIFRX_FLAG_TERR | SPDIFRX_FLAG_FERR | SPDIFRX_FLAG_SERR);
        if (Err != 0) {
            // If any error flags are set, reset to stop state
            m_EtatSPDif = eEtatSPDif::init;
        }
        break;
    }
}

//***************************************************************************
// Calculate Sample Rate
// This method calculates the sample rate of the incoming S/PDIF stream using
// the clock frequency and comparing it to the received data frame rate.
//
// The sample rate is classified into one of the standard audio rates (e.g.,
// 32kHz, 44.1kHz, 48kHz, 96kHz, 192kHz).
//
void cSPDIF_RX::CalcSampleRate() {
    // Calculate the sample rate based on the received S/PDIF clock data
    uint32_t SampleRate = (m_Freq_SPDiff_Clk * 5) / ((((m_hSPDIFRXMod.hSPDIFRX.Instance->SR) >> 16) & 0xEFFF) * 64);

    // Determine the closest standard sample rate
    if (SampleRate > 190000) {
        m_SPDiff_SampleRate = 192000;
    } else if (SampleRate > 90000) {
        m_SPDiff_SampleRate = 96000;
    } else if (SampleRate > 46000) {
        m_SPDiff_SampleRate = 48000;
    } else if (SampleRate > 40000) {
        m_SPDiff_SampleRate = 44100;
    } else if (SampleRate > 3000) {
        m_SPDiff_SampleRate = 32000;
    }
}

} // namespace Dad
