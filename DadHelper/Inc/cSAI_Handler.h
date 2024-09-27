//******************************************************************************
// cSAI_Handler.h
//
// This file defines a class to handle SAI (Serial Audio Interface) with
// callback mechanisms. It wraps the SAI handle and provides callback-based
// interrupt handling.
//******************************************************************************

#ifndef CSAI_HANDLER_H_
#define CSAI_HANDLER_H_

#include "main.h"

namespace Dad {

class cSAI_Handler;

//******************************************************************************
// Custom structure for SAI handle with an additional pointer to cSAI_Handler.
// This allows associating the SAI handle with the instance of cSAI_Handler
// for callback handling.
//
struct SAI_HandleTypeDefMod {
    SAI_HandleTypeDef 	hSAI;            // Standard SAI handle from HAL
    cSAI_Handler*		pSAIHandler;     // Pointer to the corresponding cSAI_Handler instance
};

//******************************************************************************
// Class cSAI_Handler
//
// This class handles the transmission/reception of SAI data and manages custom callbacks.
// Static callbacks registered in HAL are redirected to the instance-specific methods
// of this class.
//
class cSAI_Handler {
public:
    //---------------------------------------------------------------------
    // Constructor
    //
    cSAI_Handler(){}

    //------------------------------------------------------------------------
    // Virtual destructor
    //
    virtual ~cSAI_Handler(){}

    //------------------------------------------------------------------------
    // This method links the current instance of `cSAI_Handler` to its
    // associated HAL handle via `SPDIFRX_HandleTypeDefMod`. It also registers
    // the necessary callbacks for handling receive, transmit, and error interrupts.
    //
    void Init(SAI_HandleTypeDef* phSAI);

protected :
    //------------------------------------------------------------------------
    // Member Variables

    // Custom SAI handle extended with a pointer to the handler instance.
    SAI_HandleTypeDefMod m_hSAIMod;

    //---------------------------------------------------------------------
    // Static callback functions called by HAL. These functions retrieve
    // the `cSAI_Handler` instance using the `pSAIHandler` field and call
    // the corresponding instance method.
    //
    static void ReceiveCompleteCallback(SAI_HandleTypeDef* phSAI);
    static void ReceiveHalfCompleteCallback(SAI_HandleTypeDef* phSAI);
    static void TransmitCompleteCallback(SAI_HandleTypeDef* phSAI);
    static void TransmitHalfCompleteCallback(SAI_HandleTypeDef* phSAI);
    static void ErrorCallback(SAI_HandleTypeDef* phSAI);

    //---------------------------------------------------------------------
    // Virtual methods that are meant to be overridden in derived classes
    // to define the actual behavior on callbacks.
    //
    virtual void onReceiveComplete() {}

    virtual void onReceiveHalfComplete() {}

    virtual void onTransmitComplete() {}

    virtual void onTransmitHalfComplete() {}

    virtual void onErrorCallback() {
        // Default error handling: infinite loop. Can be overridden.
        while(1);
    }
};

} /* namespace Dad */

#endif /* CSAI_HANDLER_H_ */
