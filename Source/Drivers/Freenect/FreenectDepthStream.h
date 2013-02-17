#ifndef _FREENECT_DEPTH_STREAM_H_
#define _FREENECT_DEPTH_STREAM_H_

#include <math.h> // for M_PI
#include "FreenectVideoStream.h"
#include "Driver/OniDriverAPI.h"
#include "PS1080.h"
#include "../Kinect/S2D.h.h"
#include "../Kinect/D2S.h.h"
#include "libfreenect.hpp"


class FreenectDepthStream : public FreenectVideoStream
{
public:
	// from NUI library and converted to radians - please check
	static const float DIAGONAL_FOV = 70 * (M_PI / 180);
	static const float HORIZONTAL_FOV = 58.5 * (M_PI / 180);
	static const float VERTICAL_FOV = 45.6 * (M_PI / 180);
	// from DepthKinectStream.cpp - please check
	static const int MAX_VALUE = 10000;
	static const unsigned long long GAIN_VAL = 42;
	static const unsigned long long CONST_SHIFT_VAL = 200;
	static const unsigned long long MAX_SHIFT_VAL = 2047;
	static const unsigned long long PARAM_COEFF_VAL = 4;
	static const unsigned long long SHIFT_SCALE_VAL = 10;
	static const unsigned long long ZERO_PLANE_DISTANCE_VAL = 120;
	static const double ZERO_PLANE_PIXEL_SIZE_VAL = 0.10520000010728836;
	static const double EMITTER_DCMOS_DISTANCE_VAL = 7.5;

protected:
	typedef std::map< OniVideoMode, std::pair<freenect_depth_format, freenect_resolution> > FreenectDepthModeMap;
	OniImageRegistrationMode image_registration_mode;
	
private:
	static const OniSensorType sensor_type = ONI_SENSOR_DEPTH;
	static FreenectDepthModeMap getSupportedVideoModes();
	virtual void populateFrame(void* data, OniDriverFrame* pFrame) const;
	OniStatus setVideoMode(OniVideoMode requested_mode)
	{
		FreenectDepthModeMap supported_video_modes = getSupportedVideoModes();
		FreenectDepthModeMap::const_iterator matched_mode_iter = supported_video_modes.find(requested_mode);
		if (matched_mode_iter == supported_video_modes.end())
			return ONI_STATUS_NOT_SUPPORTED;			
		
		freenect_depth_format format = matched_mode_iter->second.first;
		freenect_resolution resolution = matched_mode_iter->second.second;
		if (image_registration_mode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR) // try forcing registration mode
			format = FREENECT_DEPTH_REGISTERED;
		
		try { device->setDepthFormat(format, resolution); }
		catch (std::runtime_error e)
		{
			printf("format-resolution combination not supported by libfreenect: %d-%d\n", format, resolution);
			if (image_registration_mode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR)
			{
				printf("could not use image registration format; disabling registration and falling back to format defined in getSupportedVideoModes()\n");
				image_registration_mode = ONI_IMAGE_REGISTRATION_OFF;
				return setVideoMode(requested_mode);
			}
			return ONI_STATUS_NOT_SUPPORTED;
		}
		video_mode = requested_mode;
		return ONI_STATUS_OK;
	}

public:
	FreenectDepthStream(Freenect::FreenectDevice* pDevice);
	~FreenectDepthStream() { }
	
	static OniSensorInfo getSensorInfo()
	{
		FreenectDepthModeMap supported_modes = getSupportedVideoModes();
		OniVideoMode* modes = new OniVideoMode[supported_modes.size()];
		std::transform(supported_modes.begin(), supported_modes.end(), modes, RetrieveKey());
		return { sensor_type, SIZE(modes), modes }; // sensorType, numSupportedVideoModes, pSupportedVideoModes
	}
	virtual OniBool isImageRegistrationModeSupported(OniImageRegistrationMode mode)
	{
		return (mode == ONI_IMAGE_REGISTRATION_OFF || mode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR);
	}
	OniImageRegistrationMode getImageRegistrationMode() const { return image_registration_mode; }
	virtual OniStatus setImageRegistrationMode(OniImageRegistrationMode mode)
	{
		if (!isImageRegistrationModeSupported(mode))
			return ONI_STATUS_NOT_SUPPORTED;
		image_registration_mode = mode;
		return ONI_STATUS_OK;
	}
	
	// from StreamBase
	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize)
	{
		switch (propertyId)
		{
			default:
				return FreenectVideoStream::getProperty(propertyId, data, pDataSize);

			case ONI_STREAM_PROPERTY_HORIZONTAL_FOV:        // float (radians)
				if (*pDataSize != sizeof(float))
				{
					printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(float));
					return ONI_STATUS_ERROR;
				}
				*(static_cast<float*>(data)) = HORIZONTAL_FOV;
				return ONI_STATUS_OK;
			case ONI_STREAM_PROPERTY_VERTICAL_FOV:          // float (radians)
				if (*pDataSize != sizeof(float))
				{
					printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(float));
					return ONI_STATUS_ERROR;
				}
				*(static_cast<float*>(data)) = VERTICAL_FOV;
				return ONI_STATUS_OK;
			case ONI_STREAM_PROPERTY_MAX_VALUE:             // int
				if (*pDataSize != sizeof(int))
				{
					printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(int));
					return ONI_STATUS_ERROR;
				}
				*(static_cast<int*>(data)) = MAX_VALUE;
				return ONI_STATUS_OK;

			case XN_STREAM_PROPERTY_PIXEL_REGISTRATION:			// XnPixelRegistration (get only)
			case XN_STREAM_PROPERTY_WHITE_BALANCE_ENABLED:	// unsigned long long
			case XN_STREAM_PROPERTY_HOLE_FILTER:						// unsigned long long
			case XN_STREAM_PROPERTY_REGISTRATION_TYPE:			// XnProcessingType
			case XN_STREAM_PROPERTY_AGC_BIN:								// XnDepthAGCBin*
			case XN_STREAM_PROPERTY_PIXEL_SIZE_FACTOR:			// unsigned long long
			case XN_STREAM_PROPERTY_DCMOS_RCMOS_DISTANCE:		// double
			case XN_STREAM_PROPERTY_CLOSE_RANGE:						// unsigned long long
				return ONI_STATUS_NOT_SUPPORTED;
			
			case XN_STREAM_PROPERTY_GAIN:										// unsigned long long
				if (*pDataSize != sizeof(unsigned long long))
				{
					printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(unsigned long long));
					return ONI_STATUS_ERROR;
				}
				*(static_cast<unsigned long long*>(data)) = GAIN_VAL;
				return ONI_STATUS_OK;
			case XN_STREAM_PROPERTY_CONST_SHIFT:						// unsigned long long
				if (*pDataSize != sizeof(unsigned long long))
				{
					printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(unsigned long long));
					return ONI_STATUS_ERROR;
				}
				*(static_cast<unsigned long long*>(data)) = CONST_SHIFT_VAL;
				return ONI_STATUS_OK;
			case XN_STREAM_PROPERTY_MAX_SHIFT:							// unsigned long long
				if (*pDataSize != sizeof(unsigned long long))
				{
					printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(unsigned long long));
					return ONI_STATUS_ERROR;
				}
				*(static_cast<unsigned long long*>(data)) = MAX_SHIFT_VAL;
				return ONI_STATUS_OK;
			case XN_STREAM_PROPERTY_PARAM_COEFF:						// unsigned long long
				if (*pDataSize != sizeof(unsigned long long))
				{
					printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(unsigned long long));
					return ONI_STATUS_ERROR;
				}
				*(static_cast<unsigned long long*>(data)) = PARAM_COEFF_VAL;
				return ONI_STATUS_OK;
			case XN_STREAM_PROPERTY_SHIFT_SCALE:						// unsigned long long
				if (*pDataSize != sizeof(unsigned long long))
				{
					printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(unsigned long long));
					return ONI_STATUS_ERROR;
				}
				*(static_cast<unsigned long long*>(data)) = SHIFT_SCALE_VAL;
				return ONI_STATUS_OK;
			case XN_STREAM_PROPERTY_ZERO_PLANE_DISTANCE:		// unsigned long long
				if (*pDataSize != sizeof(unsigned long long))
				{
					printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(unsigned long long));
					return ONI_STATUS_ERROR;
				}
				*(static_cast<unsigned long long*>(data)) = ZERO_PLANE_DISTANCE_VAL;
				return ONI_STATUS_OK;
			case XN_STREAM_PROPERTY_ZERO_PLANE_PIXEL_SIZE:	// double
				if (*pDataSize != sizeof(double))
				{
					printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(double));
					return ONI_STATUS_ERROR;
				}
				*(static_cast<double*>(data)) = ZERO_PLANE_PIXEL_SIZE_VAL;
				return ONI_STATUS_OK;
			case XN_STREAM_PROPERTY_EMITTER_DCMOS_DISTANCE:	// double
				if (*pDataSize != sizeof(double))
				{
					printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(double));
					return ONI_STATUS_ERROR;
				}
				*(static_cast<double*>(data)) = EMITTER_DCMOS_DISTANCE_VAL;
				return ONI_STATUS_OK;
			case XN_STREAM_PROPERTY_S2D_TABLE:							// OniDepthPixel[]
				*pDataSize = sizeof(S2D);
				//std::copy(S2D, S2D+sizeof(S2D), static_cast<OniDepthPixel*>(data));
				xnOSMemCopy(data, S2D, sizeof(S2D));
				return ONI_STATUS_OK;
			case XN_STREAM_PROPERTY_D2S_TABLE:							// unsigned short[]
				*pDataSize = sizeof(D2S);
				//std::copy(D2S, D2S+sizeof(D2S), static_cast<unsigned short*>(data));
				xnOSMemCopy(data, D2S, sizeof(D2S));
				return ONI_STATUS_OK;
		}
	}
	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize)
	{
		switch (propertyId)
		{
			default:
				return FreenectVideoStream::setProperty(propertyId, data, dataSize);
			case ONI_STREAM_PROPERTY_MIRRORING:		// OniBool
				if (dataSize != sizeof(OniBool))
				{
					printf("Unexpected size: %d != %d\n", dataSize, sizeof(OniBool));
					return ONI_STATUS_ERROR;
				}
				mirroring = *(static_cast<const OniBool*>(data));
				return ONI_STATUS_OK;
		}
	}
};


#endif // _FREENECT_DEPTH_STREAM_H_
