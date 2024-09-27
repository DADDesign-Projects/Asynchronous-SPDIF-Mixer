//******************************************************************************
// cSAI_Handler.cpp
//
// This file defines a class to handle SAI (Serial Audio Interface) with
// callback mechanisms. It wraps the SAI handle and provides callback-based
// interrupt handling.
//******************************************************************************

#include "cSAI_Handler.h"

namespace Dad {

//------------------------------------------------------------------------
// This method links the current instance of `cSAI_Handler` to its
// associated HAL handle via `SPDIFRX_HandleTypeDefMod`. It also registers
// the necessary callbacks for handling receive, transmit, and error interrupts.
//
void cSAI_Handler::Init(SAI_HandleTypeDef* phSAI) {
	m_hSAIMod.pSAIHandler = this;
	m_hSAIMod.hSAI = *phSAI;
	m_hSAIMod.hSAI.hdmarx->Parent=&m_hSAIMod.hSAI;
	m_hSAIMod.hSAI.hdmatx->Parent=&m_hSAIMod.hSAI;

	// Register the receive complete callback
	HAL_SAI_RegisterCallback(&(m_hSAIMod.hSAI), HAL_SAI_RX_COMPLETE_CB_ID, cSAI_Handler::ReceiveCompleteCallback);

	// Register the half receive complete callback
	HAL_SAI_RegisterCallback(&(m_hSAIMod.hSAI), HAL_SAI_RX_HALFCOMPLETE_CB_ID, cSAI_Handler::ReceiveHalfCompleteCallback);

	// Register the transmit complete callback
	HAL_SAI_RegisterCallback(&(m_hSAIMod.hSAI), HAL_SAI_TX_COMPLETE_CB_ID, cSAI_Handler::TransmitCompleteCallback);

	// Register the half transmit complete callback
	HAL_SAI_RegisterCallback(&(m_hSAIMod.hSAI), HAL_SAI_TX_HALFCOMPLETE_CB_ID, cSAI_Handler::TransmitHalfCompleteCallback);

	// Register the error callback
	HAL_SAI_RegisterCallback(&(m_hSAIMod.hSAI), HAL_SAI_ERROR_CB_ID, cSAI_Handler::ErrorCallback);
}


//---------------------------------------------------------------------
// Static callback functions called by HAL. These functions retrieve
// the `cSAI_Handler` instance using the `pSAIHandler` field and call
// the corresponding instance method.
//
void cSAI_Handler::ReceiveCompleteCallback(SAI_HandleTypeDef* phSAI) {
	// Cast the SAI_HandleTypeDef to SAI_HandleTypeDefMod to access pSAIHandler
	SAI_HandleTypeDefMod* phSAIMod = (SAI_HandleTypeDefMod*)(phSAI);

	// Ensure pSAIHandler is not null before dereferencing
	if (phSAIMod->pSAIHandler) {
		phSAIMod->pSAIHandler->onReceiveComplete();
	}
}

void cSAI_Handler::ReceiveHalfCompleteCallback(SAI_HandleTypeDef* phSAI) {
	SAI_HandleTypeDefMod* phSAIMod = (SAI_HandleTypeDefMod*)(phSAI);

	if (phSAIMod->pSAIHandler) {
		phSAIMod->pSAIHandler->onReceiveHalfComplete();
	}
}

void cSAI_Handler::TransmitCompleteCallback(SAI_HandleTypeDef* phSAI) {
	SAI_HandleTypeDefMod* phSAIMod = (SAI_HandleTypeDefMod*)(phSAI);
	if (phSAIMod->pSAIHandler) {
		phSAIMod->pSAIHandler->onTransmitComplete();
	}
}

void cSAI_Handler::TransmitHalfCompleteCallback(SAI_HandleTypeDef* phSAI) {
	SAI_HandleTypeDefMod* phSAIMod = (SAI_HandleTypeDefMod*)(phSAI);
	if (phSAIMod->pSAIHandler) {
		phSAIMod->pSAIHandler->onTransmitHalfComplete();
	}
}

void cSAI_Handler::ErrorCallback(SAI_HandleTypeDef* phSAI) {
	SAI_HandleTypeDefMod* phSAIMod = (SAI_HandleTypeDefMod*)(phSAI);
	if (phSAIMod->pSAIHandler) {
		phSAIMod->pSAIHandler->onErrorCallback();
	}
}


} /* namespace Dad */

