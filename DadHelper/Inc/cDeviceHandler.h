//******************************************************************************
// cDeviceHandler.h
//
// This file defines a class to handle device with callback mechanisms.
//
//******************************************************************************

#ifndef CDEVICE_HANDLER_H_
#define CDEVICE_HANDLER_H_

#define DECLARE_DEVICE_HANDLE(DeviceTypeDef, ClassName, CallbackName)\
/*****************************************************************************/\
/* This class provides a framework for handling device communication using the HAL library.*/\
/* It includes callback mechanisms to be implemented by derived classes.*/\
\
template<class DeviceTypeDef>\
class CallbackName##Handler {\
public:\
    /*---------------------------------------------------------------------*/\
    /* Constructor Destructor*/\
\
	CallbackName##Handler(){}\
    ~CallbackName##Handler(){}\
 \
	/*-----------------------------------------------------------------------*/\
    /* Init method*/\
    /* This method initializes the instance*/ \
    /* and registering various devices callbacks.*/\
\
    void Init(DeviceTypeDef* phDevice){\
    	m_phDevice = phDevice;\
    	m_pThis = this;\
\
    	/* Register the receive complete callback*/\
    	phDevice->RxCpltCallback = ReceiveCompleteCallback;\
\
    	/* Register the half receive complete callback*/\
    	phDevice->RxHalfCpltCallback = ReceiveHalfCompleteCallback;\
\
    	/* Register the transmit complete callback*/\
    	phDevice->TxCpltCallback = TransmitCompleteCallback;\
\
    	/* Register the half transmit complete callback*/\
    	phDevice->TxHalfCpltCallback = TransmitHalfCompleteCallback;\
\
    	/* Register the error callback*/\
    	phDevice->ErrorCallback = ErrorCallback;\
    }\
\
protected:\
    /*-----------------------------------------------------------------------*/\
    /* Member Variables*/\
\
	DeviceTypeDef*	m_phDevice;\
	static CallbackName##Handler<DeviceTypeDef>* m_pThis;\
\
    /*--------------------------------------------------------------------*/\
    /* Static callback functions called by HAL.*/\
    /* These functions are static because the HAL library doesn't know which instance*/\
    /* of cDevice_Handler to call. We manually redirect the callback to the correct instance.*/\
\
    static void ReceiveCompleteCallback(DeviceTypeDef* phDevice) {\
    	m_pThis->onReceiveComplete_##CallbackName();\
    }\
\
    static void ReceiveHalfCompleteCallback(DeviceTypeDef* phDevice) {\
    	m_pThis->onReceiveHalfComplete_##CallbackName();\
    }\
\
    static void TransmitCompleteCallback(DeviceTypeDef* phDevice) {\
    	m_pThis->onTransmitComplete_##CallbackName();\
    }\
\
    static void TransmitHalfCompleteCallback(DeviceTypeDef* phDevice) {\
    	m_pThis->onTransmitHalfComplete_##CallbackName();\
    }\
    static void ErrorCallback(DeviceTypeDef* phDevice) {\
    	m_pThis->onErrorCallback_##CallbackName();\
    }\
\
    /*---------------------------------------------------------------------*/\
    /* Virtual methods for derived classes to override.*/\
    /* These are meant to define custom behavior when a callback occurs.*/\
\
    virtual void onReceiveComplete_##CallbackName() {}\
\
    virtual void onReceiveHalfComplete_##CallbackName() {}\
\
    virtual void onTransmitComplete_##CallbackName() {}\
\
    virtual void onTransmitHalfComplete_##CallbackName() {}\
\
    virtual void onErrorCallback_##CallbackName() {\
        /* Default error handling: infinite loop. Can be overridden by derived classes.*/\
        while(1);\
    }\
};\
typedef Dad::CallbackName##Handler<DeviceTypeDef> ClassName;

#define INSTANTIATE_DEVICE_HANDLE2(DeviceTypeDef, CallbackName) template<> CallbackName##Handler<DeviceTypeDef>* CallbackName##Handler<DeviceTypeDef>::m_pThis = nullptr;

#define INSTANTIATE_DEVICE_HANDLE(ClassName) template<> ClassName* ClassName::m_pThis = nullptr;



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DECLARE_DEVICE_RXHANDLE(DeviceTypeDef, ClassName, CallbackName)\
/*****************************************************************************/\
/* This class provides a framework for handling device communication using the HAL library.*/\
/* It includes callback mechanisms to be implemented by derived classes.*/\
\
template<class DeviceTypeDef>\
class CallbackName##RxHandler {\
public:\
    /*---------------------------------------------------------------------*/\
    /* Constructor Destructor*/\
\
	CallbackName##RxHandler(){}\
    ~CallbackName##RxHandler(){}\
 \
	/*-----------------------------------------------------------------------*/\
    /* Init method*/\
    /* This method initializes the instance*/ \
    /* and registering various devices callbacks.*/\
\
    void Init(DeviceTypeDef* phDevice){\
    	m_phDevice = phDevice;\
    	m_pThis = this;\
\
    	/* Register the receive complete callback*/\
    	phDevice->RxCpltCallback = ReceiveCompleteCallback;\
\
    	/* Register the half receive complete callback*/\
    	phDevice->RxHalfCpltCallback = ReceiveHalfCompleteCallback;\
\
    	/* Register the error callback*/\
    	phDevice->ErrorCallback = ErrorCallback;\
    }\
\
protected:\
    /*-----------------------------------------------------------------------*/\
    /* Member Variables*/\
\
	DeviceTypeDef*	m_phDevice;\
	static CallbackName##RxHandler<DeviceTypeDef>* m_pThis;\
\
    /*--------------------------------------------------------------------*/\
    /* Static callback functions called by HAL.*/\
    /* These functions are static because the HAL library doesn't know which instance*/\
    /* of cDevice_Handler to call. We manually redirect the callback to the correct instance.*/\
\
    static void ReceiveCompleteCallback(DeviceTypeDef* phDevice) {\
    	m_pThis->onReceiveComplete_##CallbackName();\
    }\
\
    static void ReceiveHalfCompleteCallback(DeviceTypeDef* phDevice) {\
    	m_pThis->onReceiveHalfComplete_##CallbackName();\
    }\
\
	static void ErrorCallback(DeviceTypeDef* phDevice) {\
    	m_pThis->onErrorCallback_##CallbackName();\
    }\
\
    /*---------------------------------------------------------------------*/\
    /* Virtual methods for derived classes to override.*/\
    /* These are meant to define custom behavior when a callback occurs.*/\
\
    virtual void onReceiveComplete_##CallbackName() {}\
\
    virtual void onReceiveHalfComplete_##CallbackName() {}\
\
    virtual void onErrorCallback_##CallbackName() {\
        /* Default error handling: infinite loop. Can be overridden by derived classes.*/\
        while(1);\
    }\
};\
typedef CallbackName##RxHandler<DeviceTypeDef> ClassName;\

#define INSTANTIATE_DEVICE_RXHANDLE2(DeviceTypeDef, CallbackName) template<> Dad::CallbackName##RxHandler<DeviceTypeDef>* Dad::CallbackName##RxHandler<DeviceTypeDef>::m_pThis = nullptr;

#define INSTANTIATE_DEVICE_RXHANDLE(ClassName) template<> ClassName* ClassName::m_pThis = nullptr;


#endif /* CDEVICE_HANDLER_H_ */
