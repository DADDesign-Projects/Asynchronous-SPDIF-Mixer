//******************************************************************************
// cSPDIFRX_Handler.cpp
//
// This file defines a class to handle SPDIFRX reception using the HAL interface.
// The class extends the standard SPDIFRX handle by associating it with a handler
// object, allowing the management of reception, transmission, and error callbacks.
//
//******************************************************************************

#include "cSPDIFRX_Handler.h"

namespace Dad {

	//******************************************************************************
	// Class cSPDIFRX_Handler
	//
	// This class handles the reception of SPDIFRX data and manages custom callbacks.
	// Static callbacks registered in HAL are redirected to the instance-specific methods
	// of this class.
	//

    //------------------------------------------------------------------------
    // This method links the current instance of `cSPDIFRX_Handler` to its
    // associated HAL handle via `SPDIFRX_HandleTypeDefMod`. It also registers
    // the necessary callbacks for handling receive and error interrupts.
    //
    void cSPDIFRX_Handler ::Init(SPDIFRX_HandleTypeDef* phSPDIFRX) {
        // Link the current handler instance to the SPDIFRX handle
        m_hSPDIFRXMod.pSPDIFRXHandler = this;
        m_hSPDIFRXMod.hSPDIFRX = *phSPDIFRX;
        m_hSPDIFRXMod.hSPDIFRX.hdmaDrRx->Parent=&m_hSPDIFRXMod.hSPDIFRX;
        m_hSPDIFRXMod.hSPDIFRX.hdmaCsRx->Parent=&m_hSPDIFRXMod.hSPDIFRX;


        // Register the receive complete data callback
        HAL_SPDIFRX_RegisterCallback(&(m_hSPDIFRXMod.hSPDIFRX), HAL_SPDIFRX_RX_CPLT_CB_ID, cSPDIFRX_Handler::ReceiveRxCompleteCallback);

        // Register the half receive data complete callback
        HAL_SPDIFRX_RegisterCallback(&(m_hSPDIFRXMod.hSPDIFRX), HAL_SPDIFRX_RX_HALF_CB_ID, cSPDIFRX_Handler::ReceiveRxHalfCompleteCallback);

        // Register the receive control complete callback
        HAL_SPDIFRX_RegisterCallback(&(m_hSPDIFRXMod.hSPDIFRX), HAL_SPDIFRX_CX_CPLT_CB_ID, cSPDIFRX_Handler::ReceiveCxCompleteCallback);

        // Register the half receive control complete callback
        HAL_SPDIFRX_RegisterCallback(&(m_hSPDIFRXMod.hSPDIFRX), HAL_SPDIFRX_CX_HALF_CB_ID, cSPDIFRX_Handler::ReceiveCxHalfCompleteCallback);

        // Register the error callback
        HAL_SPDIFRX_RegisterCallback(&(m_hSPDIFRXMod.hSPDIFRX), HAL_SPDIFRX_ERROR_CB_ID, cSPDIFRX_Handler::ErrorCallback);
    }

    //------------------------------------------------------------------------
    // Static Callback Methods
    //
    // These are static methods registered with HAL. They fetch the specific
    // handler instance using the `pSPDIFRXHandler` field from the extended
    // `SPDIFRX_HandleTypeDefMod` structure and call the corresponding instance
    // methods.
    //
    void cSPDIFRX_Handler::ReceiveRxCompleteCallback(SPDIFRX_HandleTypeDef* phSPDIFRX) {
        SPDIFRX_HandleTypeDefMod* phSPDIFRXMod = (SPDIFRX_HandleTypeDefMod*)(phSPDIFRX);
        if (phSPDIFRXMod->pSPDIFRXHandler) {
            phSPDIFRXMod->pSPDIFRXHandler->onReceiveRxComplete();
        }
    }

    void cSPDIFRX_Handler::ReceiveRxHalfCompleteCallback(SPDIFRX_HandleTypeDef* phSPDIFRX) {
        SPDIFRX_HandleTypeDefMod* phSPDIFRXMod = (SPDIFRX_HandleTypeDefMod*)(phSPDIFRX);
    	if (phSPDIFRXMod->pSPDIFRXHandler) {
    		phSPDIFRXMod->pSPDIFRXHandler->onReceiveRxHalfComplete();
        }
    }
    void cSPDIFRX_Handler::ReceiveCxCompleteCallback(SPDIFRX_HandleTypeDef* phSPDIFRX) {
        SPDIFRX_HandleTypeDefMod* phSPDIFRXMod = (SPDIFRX_HandleTypeDefMod*)(phSPDIFRX);
        if (phSPDIFRXMod->pSPDIFRXHandler) {
            phSPDIFRXMod->pSPDIFRXHandler->onReceiveCxComplete();
        }
    }
    void cSPDIFRX_Handler::ReceiveCxHalfCompleteCallback(SPDIFRX_HandleTypeDef* phSPDIFRX) {
        SPDIFRX_HandleTypeDefMod* phSPDIFRXMod = (SPDIFRX_HandleTypeDefMod*)(phSPDIFRX);
    	if (phSPDIFRXMod->pSPDIFRXHandler) {
    		phSPDIFRXMod->pSPDIFRXHandler->onReceiveCxHalfComplete();
        }
    }
    void cSPDIFRX_Handler::ErrorCallback(SPDIFRX_HandleTypeDef* phSPDIFRX) {
        SPDIFRX_HandleTypeDefMod* phSPDIFRXMod = (SPDIFRX_HandleTypeDefMod*)(phSPDIFRX);
        if (phSPDIFRXMod->pSPDIFRXHandler) {
        	phSPDIFRXMod->pSPDIFRXHandler->onErrorCallback();
        }
    }

} /* namespace Dad */

