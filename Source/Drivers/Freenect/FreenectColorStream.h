#ifndef _FREENECT_COLOR_STREAM_H_
#define _FREENECT_COLOR_STREAM_H_

#include <math.h> // for M_PI
#include "FreenectVideoStream.h"
#include "Driver/OniDriverAPI.h"
#include "libfreenect.hpp"


class FreenectColorStream : public FreenectVideoStream
{
public:
	// from NUI library & converted to radians - please check
	static const float DIAGONAL_FOV = 73.9 * (M_PI / 180);
	static const float HORIZONTAL_FOV = 62 * (M_PI / 180);
	static const float VERTICAL_FOV = 48.6 * (M_PI / 180);

private:
	static const OniSensorType sensor_type = ONI_SENSOR_COLOR;
	static FreenectVideoModeMap getSupportedVideoModes();
	virtual void populateFrame(void* data, OniDriverFrame* pFrame) const;
	OniStatus setVideoMode(OniVideoMode requested_mode)
	{
		FreenectVideoModeMap supported_video_modes = getSupportedVideoModes();
		FreenectVideoModeMap::const_iterator matched_mode_iter = supported_video_modes.find(requested_mode);
		if (matched_mode_iter == supported_video_modes.end())
			return ONI_STATUS_NOT_SUPPORTED;			
		
		freenect_video_format format = matched_mode_iter->second.first;
		freenect_resolution resolution = matched_mode_iter->second.second;
		
		try { device->setVideoFormat(format, resolution); }
		catch (std::runtime_error e)
		{
			printf("format-resolution combination not supported by libfreenect: %d-%d\n", format, resolution);
			return ONI_STATUS_NOT_SUPPORTED;
		}
		video_mode = requested_mode;
		return ONI_STATUS_OK;
	}

public:
	FreenectColorStream(Freenect::FreenectDevice* pDevice);
	~FreenectColorStream() { }
	
	static OniSensorInfo getSensorInfo()
	{
		FreenectVideoModeMap supported_modes = getSupportedVideoModes();
		OniVideoMode* modes = new OniVideoMode[supported_modes.size()];
		std::transform(supported_modes.begin(), supported_modes.end(), modes, RetrieveKey());
		return { sensor_type, SIZE(modes), modes }; // sensorType, numSupportedVideoModes, pSupportedVideoModes
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


#endif // _FREENECT_COLOR_STREAM_H_
