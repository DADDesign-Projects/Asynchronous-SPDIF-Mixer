//******************************************************************************
// cTIM_Handler.cpp
//
// This file defines a class to handle TIM (timer) interrupts using the HAL
// interface. The class extends the standard TIM handle by associating it with
// a handler object, allowing the management of update, period elapsed,
// and error callbacks.
//
//******************************************************************************


#include "cTIM_Handler.h"

namespace Dad {
	//******************************************************************************
	// Class cTIM_Handler
	//
	// This class handles timer interrupts (TIM) and manages custom callbacks.
	// Static callbacks registered in HAL are redirected to the instance-specific
	// methods of this class.
	//
	//******************************************************************************

    //------------------------------------------------------------------------
    // Init Method
    //
    // This method links the current instance of `cTIM_Handler` to its associated
    // HAL handle via `TIM_HandleTypeDefMod`. It also registers the necessary
    // callbacks for handling update, period elapsed, and error interrupts.
    //
    void cTIM_Handler::Init(TIM_HandleTypeDef* phTIM) {
        // Link the current handler instance to the TIM handle
        m_hTIMMod.pTIMHandler = this;
        m_hTIMMod.hTIM = *phTIM;

        // Register the update callback (e.g., when counter overflows)
        HAL_TIM_RegisterCallback(&(m_hTIMMod.hTIM), HAL_TIM_PERIOD_ELAPSED_CB_ID, cTIM_Handler::PeriodElapsedCallback);

        // Register the error callback
        HAL_TIM_RegisterCallback(&(m_hTIMMod.hTIM), HAL_TIM_ERROR_CB_ID, cTIM_Handler::ErrorCallback);
    }

    //------------------------------------------------------------------------
    // Static Callback Methods
    //
    // These are static methods registered with HAL. They fetch the specific
    // handler instance using the `pTIMHandler` field from the extended
    // `TIM_HandleTypeDefMod` structure and call the corresponding instance
    // methods.
    //
    void cTIM_Handler::PeriodElapsedCallback(TIM_HandleTypeDef* phTIM) {
        TIM_HandleTypeDefMod* phTIMMod = (TIM_HandleTypeDefMod*)(phTIM);
        if (phTIMMod->pTIMHandler) {
            phTIMMod->pTIMHandler->onPeriodElapsed();
        }
    }

    void cTIM_Handler::ErrorCallback(TIM_HandleTypeDef* phTIM) {
        TIM_HandleTypeDefMod* phTIMMod = (TIM_HandleTypeDefMod*)(phTIM);
        if (phTIMMod->pTIMHandler) {
            phTIMMod->pTIMHandler->onErrorCallback();
        }
    }

} /* namespace Dad */
