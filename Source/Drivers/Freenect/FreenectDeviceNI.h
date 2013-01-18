#ifndef _FREENECT_DEVICE_NI_H_
#define _FREENECT_DEVICE_NI_H_

#include "libfreenect.hpp"
#include "Driver/OniDriverAPI.h"
#include "PS1080.h"
#include "FreenectDepthStream.h"
#include "FreenectColorStream.h"


using namespace oni::driver;

class FreenectDeviceNI : public DeviceBase, public Freenect::FreenectDevice
{
protected:
	FreenectDepthStream* depth_stream;
	FreenectColorStream* color_stream;

public:
	FreenectDeviceNI(freenect_context *_ctx, int _index);
	~FreenectDeviceNI();

	// from DeviceBase
	OniStatus getSensorInfoList(OniSensorInfo** pSensors, int* numSensors);
	virtual OniBool isImageRegistrationModeSupported(OniImageRegistrationMode mode) { return depth_stream->isImageRegistrationModeSupported(mode); }
	StreamBase* createStream(OniSensorType sensorType);
	void destroyStream(StreamBase* pStream);
	// property and command handlers are empty skeletons by default
	// only add here if the property is generic to all children
	// otherwise, implement in child and call these in default case
	OniBool isPropertySupported(int propertyId) { return (getProperty(propertyId, NULL, NULL) != ONI_STATUS_NOT_SUPPORTED); }
	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize)
	{
		switch (propertyId)
		{
			default:
			case ONI_DEVICE_PROPERTY_FIRMWARE_VERSION:				// string
			case ONI_DEVICE_PROPERTY_DRIVER_VERSION:					// OniVersion
			case ONI_DEVICE_PROPERTY_HARDWARE_VERSION:				// int
			case ONI_DEVICE_PROPERTY_SERIAL_NUMBER:						// string
			case ONI_DEVICE_PROPERTY_ERROR_STATE:							// ?
			// files
			case ONI_DEVICE_PROPERTY_PLAYBACK_SPEED:					// float
			case ONI_DEVICE_PROPERTY_PLAYBACK_REPEAT_ENABLED:	// OniBool
			// xn
			case XN_MODULE_PROPERTY_USB_INTERFACE:						// XnSensorUsbInterface
			case XN_MODULE_PROPERTY_MIRROR:										// bool
			case XN_MODULE_PROPERTY_RESET_SENSOR_ON_STARTUP:	// unsigned long long
			case XN_MODULE_PROPERTY_LEAN_INIT:								// unsigned long long
			case XN_MODULE_PROPERTY_SERIAL_NUMBER:						// unsigned long long
			case XN_MODULE_PROPERTY_VERSION:									// XnVersions
				return ONI_STATUS_NOT_SUPPORTED;
				
			case ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION:			// OniImageRegistrationMode
				if (*pDataSize != sizeof(OniImageRegistrationMode))
				{
					printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(OniImageRegistrationMode));
					return ONI_STATUS_ERROR;
				}
				*(static_cast<OniImageRegistrationMode*>(data)) = depth_stream->getImageRegistrationMode();
				return ONI_STATUS_OK;
		}
	}	
	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize)
	{
		switch (propertyId)
		{
			default:
			case ONI_DEVICE_PROPERTY_FIRMWARE_VERSION:				// By implementation
			case ONI_DEVICE_PROPERTY_DRIVER_VERSION:					// OniVersion
			case ONI_DEVICE_PROPERTY_HARDWARE_VERSION:				// int
			case ONI_DEVICE_PROPERTY_SERIAL_NUMBER:						// string
			case ONI_DEVICE_PROPERTY_ERROR_STATE:							// ?
			// files
			case ONI_DEVICE_PROPERTY_PLAYBACK_SPEED:					// float
			case ONI_DEVICE_PROPERTY_PLAYBACK_REPEAT_ENABLED:	// OniBool
			// xn
			case XN_MODULE_PROPERTY_USB_INTERFACE:						// XnSensorUsbInterface
			case XN_MODULE_PROPERTY_MIRROR:										// bool
			case XN_MODULE_PROPERTY_RESET_SENSOR_ON_STARTUP:	// unsigned long long
			case XN_MODULE_PROPERTY_LEAN_INIT:								// unsigned long long
			case XN_MODULE_PROPERTY_SERIAL_NUMBER:						// unsigned long long
			case XN_MODULE_PROPERTY_VERSION:									// XnVersions
			// xn commands
			case XN_MODULE_PROPERTY_FIRMWARE_PARAM:						// XnInnerParam
			case XN_MODULE_PROPERTY_RESET:										// unsigned long long
			case XN_MODULE_PROPERTY_IMAGE_CONTROL:						// XnControlProcessingData
			case XN_MODULE_PROPERTY_DEPTH_CONTROL:						// XnControlProcessingData
			case XN_MODULE_PROPERTY_AHB:											// XnAHBData
			case XN_MODULE_PROPERTY_LED_STATE:								// XnLedState
				return ONI_STATUS_NOT_SUPPORTED;

			case ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION:			// OniImageRegistrationMode
				if (dataSize != sizeof(OniImageRegistrationMode))
				{
					printf("Unexpected size: %d != %d\n", dataSize, sizeof(OniImageRegistrationMode));
					return ONI_STATUS_ERROR;
				}
				return depth_stream->setImageRegistrationMode(*(static_cast<const OniImageRegistrationMode*>(data)));
		}
	}
	OniBool isCommandSupported(int propertyId) { return (invoke(propertyId, NULL, NULL) != ONI_STATUS_NOT_SUPPORTED); }
	virtual OniStatus invoke(int commandId, const void* data, int dataSize)
	{
		switch (commandId)
		{
			default:
			case ONI_DEVICE_COMMAND_SEEK:	// OniSeek
				return ONI_STATUS_NOT_SUPPORTED;
		}
	}

	// from Freenect::FreenectDevice
	// Do not call these directly, even in child
	void DepthCallback(void *depth, uint32_t timestamp)
	{
		depth_stream->acquireFrame(depth, timestamp);
	}
	void VideoCallback(void *image, uint32_t timestamp)
	{
		color_stream->acquireFrame(image, timestamp);
	}
	
	
	/* todo : from DeviceBase
	virtual OniStatus tryManualTrigger() {return ONI_STATUS_OK;}
	*/
};


#endif //_FREENECT_DEVICE_NI_H_
