//******************************************************************************
// cSAI_DIR9001_RX.h
//
// This file defines a class to handle DIR9001 reception using the SAI
// interface. It inherits from `cDeviceHandler` to manage callbacks for audio
// data processing more effectively.
//******************************************************************************

#ifndef CSAIWM8805RX_H_
#define CSAIWM8805RX_H_

#include "main.h"
#include "cDeviceHandler.h"  // Base class for callback handling
#include "cMixer.h"

namespace Dad {

//***************************************************************************
// Class cSAI_DIR9001_RX
//
// This class handles the DIR9001 interface in reception mode using the SAI
// (Serial Audio Interface) of the STM32. It provides initialization,
// starting and stopping of reception, and callback handling for received data.
DECLARE_DEVICE_HANDLE(SAI_HandleTypeDef, cSAIA2_Handler, SAIA2)
class cSAI_DIR9001_RX1 : public cSAIA2_Handler{
public:
    //---------------------------------------------------------------------
	// Constructor / Destructor
	cSAI_DIR9001_RX1() {}
	virtual ~cSAI_DIR9001_RX1() {}

    //---------------------------------------------------------------------
	// Initializes the class and the SAI interface.
	// It sets up the SAI parameters and registers the necessary callbacks.
	void Init(SAI_HandleTypeDef* phSAI, cMixer* pMixer, int32_t* pBuffer) {
		m_pMixer = pMixer;
		m_pBuffer = pBuffer;

		cSAIA2_Handler::Init(phSAI);  // Call base class initialization (register callbacks)

		// DIR9001 RESET
		HAL_GPIO_WritePin(RESET1_GPIO_Port, RESET1_Pin, GPIO_PIN_RESET);
		HAL_Delay(300);
		HAL_GPIO_WritePin(RESET1_GPIO_Port, RESET1_Pin, GPIO_PIN_SET);
	}

    //---------------------------------------------------------------------
	// Starts receiving data using DMA.
	// It initiates the DMA transfer to fill the reception buffer.
	inline void StartReceive() {
		memset(m_pBuffer, 0xAA, 400);
		HAL_SAI_Receive_DMA(m_phDevice, (uint8_t *)m_pBuffer, RX_BUFFER_SIZE*2);//, 1000);
	}

    //---------------------------------------------------------------------
	// Stops the reception of data.
	// It aborts the ongoing DMA transfer, halting any data reception.
	inline void StopReceive() {
		HAL_SAI_Abort(m_phDevice);
	}

protected:
    //---------------------------------------------------------------------
    // Datas

    int32_t* m_pBuffer;						 // Buffer for SPDIF data

    uint64_t m_CtCallBack = 0;               // Callback counter

    cMixer* m_pMixer = nullptr;              // Pointer to the mixer for audio data

    //---------------------------------------------------------------------
    // Overriding virtual methods from the base class to handle specific
    // reception callbacks for SAI.
    //
    virtual void onReceiveComplete_SAIA2() override {
    	GPIO_PinState NO_AUDIO = HAL_GPIO_ReadPin(NO_AUDIO1_GPIO_Port, NO_AUDIO1_Pin);
    	GPIO_PinState TRANS_ERR = HAL_GPIO_ReadPin(ERROR1_GPIO_Port, ERROR1_Pin);
    	if((NO_AUDIO | TRANS_ERR) == 0){
    		m_pMixer->pushSamples1(&m_pBuffer[RX_BUFFER_SIZE]);
    	}
    	m_CtCallBack++;
    }

    virtual void onReceiveHalfComplete_SAIA2() override {
    	GPIO_PinState NO_AUDIO = HAL_GPIO_ReadPin(NO_AUDIO1_GPIO_Port, NO_AUDIO1_Pin);
    	GPIO_PinState TRANS_ERR = HAL_GPIO_ReadPin(ERROR1_GPIO_Port, ERROR1_Pin);
    	if((NO_AUDIO | TRANS_ERR) == 0){
    		m_pMixer->pushSamples1(m_pBuffer);
    	}
    	m_CtCallBack++;
    }

    virtual void onErrorCallback_SAIA2() override {
        // Error handling logic (can be custom for SPDIF)
        while (1);  // Infinite loop on error, can be replaced with more graceful error recovery
    }
};
//***************************************************************************
// Class cSAI_DIR9001_RX2
//
// This class handles the DIR9001 interface in reception mode using the SAI
// (Serial Audio Interface) of the STM32. It provides initialization,
// starting and stopping of reception, and callback handling for received data.
DECLARE_DEVICE_HANDLE(SAI_HandleTypeDef, cSAIA3_Handler, SAIA3)
class cSAI_DIR9001_RX2 : public cSAIA3_Handler{
public:
    //---------------------------------------------------------------------
	// Constructor / Destructor
	cSAI_DIR9001_RX2() {}
	virtual ~cSAI_DIR9001_RX2() {}

    //---------------------------------------------------------------------
	// Initializes the class and the SAI interface.
	// It sets up the SAI parameters and registers the necessary callbacks.
	void Init(SAI_HandleTypeDef* phSAI, cMixer* pMixer, int32_t* pBuffer) {
		m_pMixer = pMixer;
		m_pBuffer = pBuffer;

		cSAIA3_Handler::Init(phSAI);  // Call base class initialization (register callbacks)

		// DIR9001 RESET
		HAL_GPIO_WritePin(RESET2_GPIO_Port, RESET2_Pin, GPIO_PIN_RESET);
		HAL_Delay(300);
		HAL_GPIO_WritePin(RESET2_GPIO_Port, RESET2_Pin, GPIO_PIN_SET);
	}

    //---------------------------------------------------------------------
	// Starts receiving data using DMA.
	// It initiates the DMA transfer to fill the reception buffer.
	inline void StartReceive() {
		memset(m_pBuffer, 0xAA, 400);
		HAL_SAI_Receive_DMA(m_phDevice, (uint8_t *)m_pBuffer, RX_BUFFER_SIZE*2);//, 1000);
	}

    //---------------------------------------------------------------------
	// Stops the reception of data.
	// It aborts the ongoing DMA transfer, halting any data reception.
	inline void StopReceive() {
		HAL_SAI_Abort(m_phDevice);
	}

protected:
    //---------------------------------------------------------------------
    // Datas

    int32_t* m_pBuffer;						 // Buffer for SPDIF data

    uint64_t m_CtCallBack = 0;               // Callback counter

    cMixer* m_pMixer = nullptr;              // Pointer to the mixer for audio data

    //---------------------------------------------------------------------
    // Overriding virtual methods from the base class to handle specific
    // reception callbacks for SAI.
    //
    virtual void onReceiveComplete_SAIA3() override {
    	GPIO_PinState NO_AUDIO = HAL_GPIO_ReadPin(NO_AUDIO2_GPIO_Port, NO_AUDIO2_Pin);
    	GPIO_PinState TRANS_ERR = HAL_GPIO_ReadPin(ERROR2_GPIO_Port, ERROR2_Pin);
    	if((NO_AUDIO | TRANS_ERR) == 0){
    		m_pMixer->pushSamples3(&m_pBuffer[RX_BUFFER_SIZE]);
    	}
    	m_CtCallBack++;
    }

    virtual void onReceiveHalfComplete_SAIA3() override {
    	GPIO_PinState NO_AUDIO = HAL_GPIO_ReadPin(NO_AUDIO2_GPIO_Port, NO_AUDIO2_Pin);
    	GPIO_PinState TRANS_ERR = HAL_GPIO_ReadPin(ERROR2_GPIO_Port, ERROR2_Pin);
    	if((NO_AUDIO | TRANS_ERR) == 0){
    		m_pMixer->pushSamples3(m_pBuffer);
    	}
    	m_CtCallBack++;
    }

    virtual void onErrorCallback_SAIA3() override {
        // Error handling logic (can be custom for SPDIF)
        while (1);  // Infinite loop on error, can be replaced with more graceful error recovery
    }
};

} /* namespace Dad */

#endif /* CSAIWM8805RX_H_ */
