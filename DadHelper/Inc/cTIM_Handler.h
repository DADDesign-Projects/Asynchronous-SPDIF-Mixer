//******************************************************************************
// cTIM_Handler.h
//
// This file defines a class to handle TIM (timer) interrupts using the HAL
// interface. The class extends the standard TIM handle by associating it with
// a handler object, allowing the management of update, period elapsed,
// and error callbacks.
//
//******************************************************************************

#ifndef CTIM_HANDLER_H_
#define CTIM_HANDLER_H_

#include "main.h"

namespace Dad {
class cTIM_Handler;

//******************************************************************************
// Structure TIM_HandleTypeDefMod
//
// This structure extends the standard TIM handle from HAL with an additional
// pointer to the `cTIM_Handler` instance. This allows associating the TIM handle
// with a specific handler instance for callback handling.
//
//******************************************************************************
struct TIM_HandleTypeDefMod {
    TIM_HandleTypeDef 	hTIM;             // Standard TIM handle from HAL
    cTIM_Handler*		pTIMHandler;      // Pointer to the corresponding cTIM_Handler instance
};

//******************************************************************************
// Class cTIM_Handler
//
// This class handles timer interrupts (TIM) and manages custom callbacks.
// Static callbacks registered in HAL are redirected to the instance-specific
// methods of this class.
//
//******************************************************************************
class cTIM_Handler {
public:
    //------------------------------------------------------------------------
    // Constructor
    //
    // Initializes an instance of cTIM_Handler with no parameters.
    //
    cTIM_Handler(){}

    //------------------------------------------------------------------------
    // Virtual Destructor
    //
    // Virtual destructor to allow clean inheritance and destruction.
    //
    virtual ~cTIM_Handler(){}

    //------------------------------------------------------------------------
    // Init Method
    //
    // This method links the current instance of `cTIM_Handler` to its associated
    // HAL handle via `TIM_HandleTypeDefMod`. It also registers the necessary
    // callbacks for handling update, period elapsed, and error interrupts.
    //
    void Init(TIM_HandleTypeDef* phTIM);

protected:
    //------------------------------------------------------------------------
    // Member Variables
    //
    // m_hTIMMod: Custom TIM handle extended with a pointer to the handler instance.
    //
    TIM_HandleTypeDefMod m_hTIMMod;

    //------------------------------------------------------------------------
    // Static Callback Methods
    //
    // These are static methods registered with HAL. They fetch the specific
    // handler instance using the `pTIMHandler` field from the extended
    // `TIM_HandleTypeDefMod` structure and call the corresponding instance
    // methods.
    //
    static void PeriodElapsedCallback(TIM_HandleTypeDef* phTIM);
    static void ErrorCallback(TIM_HandleTypeDef* phTIM);

    //------------------------------------------------------------------------
    // Instance Methods for Handling Callbacks
    //
    // These are virtual methods that are meant to be overridden in derived classes.
    // They define the behavior on period elapsed and error callbacks.
    //
    virtual void onPeriodElapsed() {
        // Default behavior for period elapsed callback
    }

    virtual void onErrorCallback() {
        // Default error handling: infinite loop. Can be overridden for specific error handling.
        while(1);
    }
};

} /* namespace Dad */

#endif /* CTIM_HANDLER_H_ */
