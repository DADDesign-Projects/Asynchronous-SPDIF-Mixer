//***************************************************************************
// cSPDIF_RX.h
//
// This file defines the `cSPDIF_RX` class, responsible for managing the
// reception of audio data from an S/PDIF (Sony/Philips Digital Interface) stream.
//
// The class inherits from two handlers:
// - `cSPDIFRX_Handler` to handle interrupts related to audio data reception
//   via DMA.
// - `cTIM_Handler` to manage periodic timer interrupts, used to monitor
//   synchronization and ensure the proper handling of the audio stream.
//
// The class allows for initialization, processing DMA callbacks for complete
// and half-complete data reception, and calculating the sample rate of the
// incoming audio stream. It also provides utility methods to access the current
// sample rate and synchronization state.
//
//***************************************************************************

#ifndef CSPDIFRX_H_
#define CSPDIFRX_H_

#include "cSPDIFRX_Handler.h"
#include "cTIM_Handler.h"
#include "main.h"
#include "cMixer.h"

namespace Dad {

//***************************************************************************
// Enum eEtatSPDif
//
// Defines the synchronization state of the S/PDIF input.
// - stop     : Stop recever
// - inactive : Nothing
// - init     : Reinitialization of the S/PDIF receiver
// - synchro  : Waiting for synchronization with the incoming S/PDIF stream
// - run      : Monitoring the state of the S/PDIF synchronization for errors
enum class eEtatSPDif {
    stop,
	inactive,
	init,
    synchro,
    run
};

//***************************************************************************
// Class cSPDIF_RX
//
// This class manages the reception of an S/PDIF stream by inheriting from
// `cSPDIFRX_Handler` for handling S/PDIF DMA-related callbacks, and from
// `cTIM_Handler` for managing timer interrupts.
//
class cSPDIF_RX : public cSPDIFRX_Handler, cTIM_Handler {
public:


    //---------------------------------------------------------------------
	// Constructor
    cSPDIF_RX(){}

    //---------------------------------------------------------------------
    // Destructor
    virtual ~cSPDIF_RX(){}

    //---------------------------------------------------------------------
    // Method Init
    //
    // Initializes the S/PDIF reception system based on the provided clock
    // frequency (Freq_SPDiff_Clk).
    //
    // @param Freq_SPDiff_Clk: Clock frequency for calculate samplerate
    //
    void Init(SPDIFRX_HandleTypeDef* phSPDIFRX, TIM_HandleTypeDef* phTIM, cMixer* pMixer, uint32_t Freq_SPDiff_Clk);

    //---------------------------------------------------------------------
    // DMA Callback - onReceiveComplete
    //
    // Called when the complete buffer is received via DMA.
    //
    virtual void onReceiveRxComplete() override {
       m_pMixer->pushSamples2(&m_Buffer[RX_BUFFER_SIZE]);
        m_CtCallBack++;  // Increment callback counter
    }

    //---------------------------------------------------------------------
    // DMA Callback - onReceiveHalfComplete
    //
    // Called when half of the buffer is filled via DMA.
    //
    virtual void onReceiveRxHalfComplete() override {
    	m_pMixer->pushSamples2(m_Buffer);
        m_CtCallBack++;  // Increment callback counter
    }

    //---------------------------------------------------------------------
    // Timer Callback - onPeriodElapsed
    //
    // Called when the timer period elapses (e.g., every 100 ms). This method
    // is used to perform periodic checks, such as verifying synchronization
    // or adjusting the system as needed.
    virtual void onPeriodElapsed() override;

    //---------------------------------------------------------------------
    // Utility Methods
    //

    //---------------------------------------------------------------------
	// Starts receiving data
	//
	inline void StartReceive() {
		m_EtatSPDif = eEtatSPDif::init;
	}

    //---------------------------------------------------------------------
	// Stops the reception of data.
	//
	inline void StopReceive() {
		m_EtatSPDif = eEtatSPDif::stop;
	}

    // Returns the current sample rate of the S/PDIF stream.
    inline uint32_t getSampleRate() { return m_SPDiff_SampleRate; }

    // Returns the current synchronization state of the S/PDIF stream.
    inline eEtatSPDif getEtat() { return m_EtatSPDif; }

protected:
    //---------------------------------------------------------------------
    // Method CalcSampleRate
    //
    // Calculates the sample rate of the incoming S/PDIF stream based on
    // synchronization information.
    //
    void CalcSampleRate();

    //---------------------------------------------------------------------
    // Member Variables
    cMixer*		m_pMixer;
	eEtatSPDif	m_EtatSPDif;            		// Synchronization state of the S/PDIF

	uint32_t    m_Freq_SPDiff_Clk;      		// Clock frequency for calculate S/PDIF samplerate
	uint32_t    m_SPDiff_SampleRate;    		// Sample rate of the received stream

	int32_t     m_Buffer[RX_BUFFER_SIZE * 2]; 	// Double buffer for S/PDIF reception

	uint64_t    m_CtCallBack;           		// Counter to track the number of callbacks

};

} /* namespace Dad */

#endif /* CSPDIFRX_H_ */
