//******************************************************************************
// cSPDIFRX_Handler.h
//
// This file defines a class to handle SPDIFRX reception using the HAL interface.
// The class extends the standard SPDIFRX handle by associating it with a handler
// object, allowing the management of reception, transmission, and error callbacks.
//
//******************************************************************************

#ifndef CSPDIFRX_HANDLER_H_
#define CSPDIFRX_HANDLER_H_

#include "main.h"

namespace Dad {
class cSPDIFRX_Handler;

//******************************************************************************
// This structure extends the standard SPDIFRX handle from HAL with an additional
// pointer to the `cSPDIFRX_Handler` instance. This allows associating the SPDIFRX
// handle with a specific handler instance for callback handling.
struct SPDIFRX_HandleTypeDefMod {
    SPDIFRX_HandleTypeDef 	hSPDIFRX;            // Standard SPDIFRX handle from HAL
    cSPDIFRX_Handler*		pSPDIFRXHandler;     // Pointer to the corresponding cSPDIFRX_Handler instance
};

//******************************************************************************
// Class cSPDIFRX_Handler
//
// This class handles the reception of SPDIFRX data and manages custom callbacks.
// Static callbacks registered in HAL are redirected to the instance-specific methods
// of this class.
//
class cSPDIFRX_Handler {
public:
    //------------------------------------------------------------------------
    // Constructor
    //
    cSPDIFRX_Handler(){}

    //------------------------------------------------------------------------
    // Virtual Destructor
    ///
    virtual ~cSPDIFRX_Handler(){}

    //------------------------------------------------------------------------
    // This method links the current instance of `cSPDIFRX_Handler` to its
    // associated HAL handle via `SPDIFRX_HandleTypeDefMod`. It also registers
    // the necessary callbacks for handling receive and error interrupts.
    //
    void Init(SPDIFRX_HandleTypeDef* phSPDIFRX);
protected:
    //------------------------------------------------------------------------
    // Member Variables

    // Custom SPDIFRX handle extended with a pointer to the handler instance.
    //
    SPDIFRX_HandleTypeDefMod m_hSPDIFRXMod;

    //------------------------------------------------------------------------
    // Static Callback Methods
    //
    // These are static methods registered with HAL. They fetch the specific
    // handler instance using the `pSPDIFRXHandler` field from the extended
    // `SPDIFRX_HandleTypeDefMod` structure and call the corresponding instance
    // methods.
    //
    static void ReceiveRxCompleteCallback(SPDIFRX_HandleTypeDef* phSPDIFRX);
    static void ReceiveRxHalfCompleteCallback(SPDIFRX_HandleTypeDef* phSPDIFRX);
    static void ReceiveCxCompleteCallback(SPDIFRX_HandleTypeDef* phSPDIFRX);
    static void ReceiveCxHalfCompleteCallback(SPDIFRX_HandleTypeDef* phSPDIFRX);
    static void ErrorCallback(SPDIFRX_HandleTypeDef* phSPDIFRX);

    //------------------------------------------------------------------------
    // Instance Methods for Handling Callbacks
    //
    // These are virtual methods that are meant to be overridden in derived classes.
    // They define the behavior on reception, transmission, and error callbacks.
    //
    virtual void onReceiveRxComplete() {}
    virtual void onReceiveRxHalfComplete() {}
    virtual void onReceiveCxComplete() {}
    virtual void onReceiveCxHalfComplete() {}
    virtual void onErrorCallback() {
        // Default error handling: infinite loop. Can be overridden for specific error handling.
        while(1);
    }
};

} /* namespace Dad */

#endif /* CSPDIFRX_HANDLER_H_ */
