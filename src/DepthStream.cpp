#include "DepthStream.hpp"
/*
#include "S2D.h"
#include "D2S.h"

#include "S2D_exp.h"
#include "D2S_exp.h"
*/
#include "S2D.h.h"
#include "D2S.h.h"
#include "DepthSensorCalibBlob.h"

using namespace FreenectDriver;


DepthStream::DepthStream(Freenect::FreenectDevice* pDevice) : VideoStream(pDevice) {
	video_mode = makeOniVideoMode(ONI_PIXEL_FORMAT_DEPTH_1_MM, 640, 480, 30);
    //image_registration_mode = ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR;
    image_registration_mode = ONI_IMAGE_REGISTRATION_OFF;
	setVideoMode(video_mode);
}

// Add video modes here as you implement them
// Note: if image_registration_mode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR,
// setVideoFormat() will try FREENECT_DEPTH_REGISTERED first then fall back on what is set here.
DepthStream::FreenectDepthModeMap DepthStream::getSupportedVideoModes() {
	FreenectDepthModeMap modes;
	//											pixelFormat, resolutionX, resolutionY, fps		freenect_video_format, freenect_resolution						
	modes[makeOniVideoMode(ONI_PIXEL_FORMAT_DEPTH_1_MM, 640, 480, 30)] = std::pair<freenect_depth_format, freenect_resolution>(FREENECT_DEPTH_MM, FREENECT_RESOLUTION_MEDIUM);
	
	
	return modes;
}

OniStatus DepthStream::setVideoMode(OniVideoMode requested_mode) {
	FreenectDepthModeMap supported_video_modes = getSupportedVideoModes();
	FreenectDepthModeMap::const_iterator matched_mode_iter = supported_video_modes.find(requested_mode);
	if (matched_mode_iter == supported_video_modes.end())
		return ONI_STATUS_NOT_SUPPORTED;      
	
	freenect_depth_format format = matched_mode_iter->second.first;
	freenect_resolution resolution = matched_mode_iter->second.second;
	if (image_registration_mode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR) // try forcing registration mode
		format = FREENECT_DEPTH_REGISTERED;
	
	try { device->setDepthFormat(format, resolution); }
	catch (std::runtime_error e) {
		printf("format-resolution combination not supported by libfreenect: %d-%d\n", format, resolution);
		if (image_registration_mode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR) {
			printf("could not use image registration format; disabling registration and falling back to format defined in getSupportedVideoModes()\n");
			image_registration_mode = ONI_IMAGE_REGISTRATION_OFF;
			return setVideoMode(requested_mode);
		}
		return ONI_STATUS_NOT_SUPPORTED;
	}
	video_mode = requested_mode;
	return ONI_STATUS_OK;
}

// Discard the depth value equal or greater than the max value.
inline unsigned short filterReliableDepthValue(unsigned short value)
{
    return value < DepthStream::MAX_VALUE ? value : 0;
}


OniStatus DepthStream::getProperty(int propertyId, void* data, int* pDataSize)
{

    switch (propertyId)
    {
    default:
    {

        OniStatus ret = VideoStream::getProperty(propertyId, data, pDataSize);
        if(ret == ONI_STATUS_OK)
            return ONI_STATUS_OK;

        if(pDataSize != NULL)
            std::cout << "--> default:" << propertyId << "  size:" << *pDataSize << std::endl;
        else
            std::cout << "--> default:" << propertyId << "  size: null" << std::endl;
        return ONI_STATUS_NOT_SUPPORTED;
    }
    case ONI_STREAM_PROPERTY_HORIZONTAL_FOV:        // float (radians)
        if (*pDataSize != sizeof(float)) {
            printf("Unexpected size: %d != %lu\n", *pDataSize, sizeof(float));
            return ONI_STATUS_ERROR;
        }

        //*(static_cast<float*>(data)) = COLOR_HORIZONTAL_FOV;
        *(static_cast<float*>(data)) = HORIZONTAL_FOV;
        std::cout << "--> ONI_STREAM_PROPERTY_HORIZONTAL_FOV" << std::endl;
        return ONI_STATUS_OK;

    case ONI_STREAM_PROPERTY_VERTICAL_FOV:          // float (radians)
        if (*pDataSize != sizeof(float)) {
            printf("Unexpected size: %d != %lu\n", *pDataSize, sizeof(float));
            return ONI_STATUS_ERROR;
        }
        //*(static_cast<float*>(data)) = COLOR_VERTICAL_FOV;
        *(static_cast<float*>(data)) = VERTICAL_FOV;
        std::cout << "--> ONI_STREAM_PROPERTY_VERTICAL_FOV" << std::endl;
        return ONI_STATUS_OK;

    case ONI_STREAM_PROPERTY_MAX_VALUE:             // int
        if (*pDataSize != sizeof(int)) {
            printf("Unexpected size: %d != %lu\n", *pDataSize, sizeof(int));
            return ONI_STATUS_ERROR;
        }
        *(static_cast<int*>(data)) = MAX_VALUE;
        std::cout << "--> ONI_STREAM_PROPERTY_MAX_VALUE" << std::endl;
        return ONI_STATUS_OK;

    case ONI_STREAM_PROPERTY_MIN_VALUE:             // int
        if (*pDataSize != sizeof(int)) {
            printf("Unexpected size: %d != %lu\n", *pDataSize, sizeof(int));
            return ONI_STATUS_ERROR;
        }
        *(static_cast<int*>(data)) = 0;
        std::cout << "--> ONI_STREAM_PROPERTY_MIN_VALUE" << std::endl;
        return ONI_STATUS_OK;

    case XN_STREAM_PROPERTY_PIXEL_REGISTRATION:     // XnPixelRegistration (get only)
    case XN_STREAM_PROPERTY_WHITE_BALANCE_ENABLED:  // unsigned long long
        //case XN_STREAM_PROPERTY_HOLE_FILTER:            // unsigned long long
        //case XN_STREAM_PROPERTY_REGISTRATION_TYPE:      // XnProcessingType
    case XN_STREAM_PROPERTY_AGC_BIN:                // XnDepthAGCBin*
        //case XN_STREAM_PROPERTY_PIXEL_SIZE_FACTOR:      // unsigned long long
        //case XN_STREAM_PROPERTY_DCMOS_RCMOS_DISTANCE:   // double
        std::cout << "--> ccccccccccccccccccccccc" << std::endl;
        return ONI_STATUS_NOT_SUPPORTED;

    case XN_STREAM_PROPERTY_CLOSE_RANGE:            // unsigned long long
        if (*pDataSize != sizeof(unsigned long long)) {
            printf("Unexpected size: %d != %lu\n", *pDataSize, sizeof(int));
            return ONI_STATUS_ERROR;
        }      *(static_cast<unsigned long long*>(data)) = 0;
        std::cout << "--> XN_STREAM_PROPERTY_CLOSE_RANGE" << std::endl;
        return ONI_STATUS_OK;

    case XN_STREAM_PROPERTY_GAIN:                   // unsigned long long
        if (*pDataSize != sizeof(unsigned long long)) {
            printf("Unexpected size: %d != %lu\n", *pDataSize, sizeof(unsigned long long));
            return ONI_STATUS_ERROR;
        }
        *(static_cast<unsigned long long*>(data)) = GAIN_VAL;
        std::cout << "--> XN_STREAM_PROPERTY_GAIN" << std::endl;
        return ONI_STATUS_OK;

    case XN_STREAM_PROPERTY_CONST_SHIFT:            // unsigned long long
        if (*pDataSize != sizeof(unsigned long long)) {
            printf("Unexpected size: %d != %lu\n", *pDataSize, sizeof(unsigned long long));
            return ONI_STATUS_ERROR;
        }
        *(static_cast<unsigned long long*>(data)) = CONST_SHIFT_VAL;
        std::cout << "--> XN_STREAM_PROPERTY_CONST_SHIFT" << std::endl;
        return ONI_STATUS_OK;

    case XN_STREAM_PROPERTY_MAX_SHIFT:              // unsigned long long
        if (*pDataSize != sizeof(unsigned long long)) {
            printf("Unexpected size: %d != %lu\n", *pDataSize, sizeof(unsigned long long));
            return ONI_STATUS_ERROR;
        }
        *(static_cast<unsigned long long*>(data)) = MAX_SHIFT_VAL;
        std::cout << "--> XN_STREAM_PROPERTY_MAX_SHIFT" << std::endl;
        return ONI_STATUS_OK;

    case XN_STREAM_PROPERTY_PARAM_COEFF:            // unsigned long long
        if (*pDataSize != sizeof(unsigned long long)) {
            printf("Unexpected size: %d != %lu\n", *pDataSize, sizeof(unsigned long long));
            return ONI_STATUS_ERROR;
        }
        *(static_cast<unsigned long long*>(data)) = PARAM_COEFF_VAL;
        std::cout << "--> XN_STREAM_PROPERTY_PARAM_COEFF" << std::endl;
        return ONI_STATUS_OK;

    case XN_STREAM_PROPERTY_SHIFT_SCALE:            // unsigned long long
        if (*pDataSize != sizeof(unsigned long long)) {
            printf("Unexpected size: %d != %lu\n", *pDataSize, sizeof(unsigned long long));
            return ONI_STATUS_ERROR;
        }
        *(static_cast<unsigned long long*>(data)) = SHIFT_SCALE_VAL;
        std::cout << "--> XN_STREAM_PROPERTY_SHIFT_SCALE" << std::endl;
        return ONI_STATUS_OK;

    case XN_STREAM_PROPERTY_ZERO_PLANE_DISTANCE:    // unsigned long long
        if (*pDataSize != sizeof(unsigned long long)) {
            printf("Unexpected size: %d != %lu\n", *pDataSize, sizeof(unsigned long long));
            return ONI_STATUS_ERROR;
        }
        *(static_cast<unsigned long long*>(data)) = ZERO_PLANE_DISTANCE_VAL;
        std::cout << "--> XN_STREAM_PROPERTY_ZERO_PLANE_DISTANCE" << std::endl;
        return ONI_STATUS_OK;

    case XN_STREAM_PROPERTY_ZERO_PLANE_PIXEL_SIZE:  // double
        if (*pDataSize != sizeof(double)) {
            printf("Unexpected size: %d != %lu\n", *pDataSize, sizeof(double));
            return ONI_STATUS_ERROR;
        }
        *(static_cast<double*>(data)) = ZERO_PLANE_PIXEL_SIZE_VAL;
        std::cout << "--> XN_STREAM_PROPERTY_ZERO_PLANE_PIXEL_SIZE" << std::endl;
        return ONI_STATUS_OK;

    case XN_STREAM_PROPERTY_EMITTER_DCMOS_DISTANCE: // double
        if (*pDataSize != sizeof(double)) {
            printf("Unexpected size: %d != %lu\n", *pDataSize, sizeof(double));
            return ONI_STATUS_ERROR;
        }
        *(static_cast<double*>(data)) = EMITTER_DCMOS_DISTANCE_VAL;
        std::cout << "--> XN_STREAM_PROPERTY_EMITTER_DCMOS_DISTANCE" << std::endl;
        return ONI_STATUS_OK;

    case XN_STREAM_PROPERTY_S2D_TABLE:              // OniDepthPixel[]
        /*
      *pDataSize = sizeof(S2D);
      //std::copy(&S2D[0], &S2D[*pDataSize / sizeof(uint16_t)], static_cast<OniDepthPixel*>(data));
      memcpy(data,S2D,*pDataSize);
*/
        /* // works with skel,no hands
      *pDataSize = S2D_Size;
      memcpy(data,S2D_Blob,S2D_Size);
      */
        /*
      *pDataSize = sizeof(S2D_exp);
      memcpy(data,S2D_exp,*pDataSize);
*/
        *pDataSize = sizeof(S2D_);
        memcpy(data,S2D_,sizeof(S2D_));
        std::cout << "--> XN_STREAM_PROPERTY_S2D_TABLE" << std::endl;

        return ONI_STATUS_OK;

    case XN_STREAM_PROPERTY_D2S_TABLE:              // unsigned short[]
        /*
      *pDataSize = sizeof(D2S);
      //std::copy(&D2S[0], &D2S[*pDataSize / sizeof(uint8_t)], static_cast<unsigned short*>(data));
      memcpy(data,D2S,*pDataSize);
*/
        /* // works with skel,no hands
      *pDataSize = D2S_Size;
      memcpy(data,D2S_Blob,D2S_Size);
*/
        /*
      *pDataSize = sizeof(D2S_exp);
      memcpy(data,D2S_exp,*pDataSize);
*/
        *pDataSize = sizeof(D2S_);
        memcpy(data,D2S_,sizeof(D2S_));
        std::cout << "--> XN_STREAM_PROPERTY_D2S_TABLE" << std::endl;

        return ONI_STATUS_OK;
    case XN_STREAM_PROPERTY_DEPTH_SENSOR_CALIBRATION_INFO:              // unsigned short[]
        *pDataSize = sizeof(DepthSensorCalibBlob);
        memcpy(data,DepthSensorCalibBlob,sizeof(DepthSensorCalibBlob));
        std::cout << "--> XN_STREAM_PROPERTY_DEPTH_SENSOR_CALIBRATION_INFO" << std::endl;

        return ONI_STATUS_OK;


    case XN_STREAM_PROPERTY_HOLE_FILTER:
        *pDataSize = sizeof(unsigned long long);
        *(static_cast<unsigned long long*>(data)) = 1;
        std::cout << "--> XN_STREAM_PROPERTY_HOLE_FILTER" << std::endl;
        return ONI_STATUS_OK;
    case XN_STREAM_PROPERTY_REGISTRATION_TYPE:
        *pDataSize = sizeof(unsigned long long);
        *(static_cast<unsigned long long*>(data)) = 0;
        std::cout << "--> XN_STREAM_PROPERTY_REGISTRATION_TYPE" << std::endl;
        return ONI_STATUS_OK;
    case XN_STREAM_PROPERTY_PIXEL_SIZE_FACTOR:
        *pDataSize = sizeof(unsigned long long);
        *(static_cast<unsigned long long*>(data)) = 1;
        std::cout << "--> XN_STREAM_PROPERTY_PIXEL_SIZE_FACTOR" << std::endl;
        return ONI_STATUS_OK;
    case XN_STREAM_PROPERTY_DCMOS_RCMOS_DISTANCE:
        *pDataSize = sizeof(double);
        *(static_cast<double*>(data)) = 2.63;
        std::cout << "--> XN_STREAM_PROPERTY_DCMOS_RCMOS_DISTANCE" << std::endl;
        return ONI_STATUS_OK;
    case XN_STREAM_PROPERTY_GMC_MODE:
        *pDataSize = sizeof(int);
        *(static_cast<int*>(data)) = 1;
        std::cout << "--> XN_STREAM_PROPERTY_GMC_MODE" << std::endl;
        return ONI_STATUS_OK;
    case XN_STREAM_PROPERTY_GMC_DEBUG:
        *pDataSize = sizeof(int);
        *(static_cast<int*>(data)) = 0;
        std::cout << "--> XN_STREAM_PROPERTY_GMC_DEBUG" << std::endl;
        return ONI_STATUS_OK;
    case XN_STREAM_PROPERTY_WAVELENGTH_CORRECTION:
        *pDataSize = sizeof(int);
        *(static_cast<int*>(data)) = 0;
        std::cout << "--> XN_STREAM_PROPERTY_WAVELENGTH_CORRECTION" << std::endl;
        return ONI_STATUS_OK;
    case XN_STREAM_PROPERTY_WAVELENGTH_CORRECTION_DEBUG:
        *pDataSize = sizeof(int);
        *(static_cast<int*>(data)) = 0;
        std::cout << "--> XN_STREAM_PROPERTY_WAVELENGTH_CORRECTION_DEBUG" << std::endl;
        return ONI_STATUS_OK;


    }
}


void DepthStream::populateFrame(void* data, OniFrame* frame) const {	
	frame->sensorType = sensor_type;
	frame->stride = video_mode.resolutionX*sizeof(uint16_t);
	frame->cropOriginX = frame->cropOriginY = 0;
	frame->croppingEnabled = FALSE;	
	
	// copy stream buffer from freenect
	uint16_t* data_ptr = static_cast<uint16_t*>(data);
	uint16_t* frame_data = static_cast<uint16_t*>(frame->data);
	if (mirroring)
	{
		for (unsigned int i = 0; i < frame->dataSize / 2; i++)
		{
			// find corresponding mirrored pixel
			unsigned int row = i / video_mode.resolutionX;
			unsigned int col = video_mode.resolutionX - (i % video_mode.resolutionX);
			unsigned int target = (row * video_mode.resolutionX) + col;
			// copy it to this pixel
            frame_data[i] = filterReliableDepthValue(data_ptr[target]);
		}
	}
	else
    {
        for (unsigned int i = 0; i < frame->dataSize / 2; i++)
            frame_data[i] = filterReliableDepthValue(data_ptr[i]);

        //mempcpy(frame_data,data_ptr,frame->dataSize);
        //std::copy(data_ptr, data_ptr+frame->dataSize / 2, frame_data);
    }
}


void DepthStream::notifyAllProperties()
{
    std::cout << "notifyAllProperties" << std::endl;

    double nDouble;
    int size = sizeof(nDouble);
    getProperty(XN_STREAM_PROPERTY_ZERO_PLANE_PIXEL_SIZE, &nDouble, &size);
    raisePropertyChanged(XN_STREAM_PROPERTY_ZERO_PLANE_PIXEL_SIZE, &nDouble, size);

    getProperty(XN_STREAM_PROPERTY_EMITTER_DCMOS_DISTANCE, &nDouble, &size);
    raisePropertyChanged(XN_STREAM_PROPERTY_EMITTER_DCMOS_DISTANCE, &nDouble, size);

    int nInt;
    size = sizeof(nInt);
    getProperty(XN_STREAM_PROPERTY_GAIN, &nInt, &size);
    raisePropertyChanged(XN_STREAM_PROPERTY_GAIN, &nInt, size);

    getProperty(XN_STREAM_PROPERTY_CONST_SHIFT, &nInt, &size);
    raisePropertyChanged(XN_STREAM_PROPERTY_CONST_SHIFT, &nInt, size);

    getProperty(XN_STREAM_PROPERTY_MAX_SHIFT, &nInt, &size);
    raisePropertyChanged(XN_STREAM_PROPERTY_MAX_SHIFT, &nInt, size);

    getProperty(XN_STREAM_PROPERTY_SHIFT_SCALE, &nInt, &size);
    raisePropertyChanged(XN_STREAM_PROPERTY_SHIFT_SCALE, &nInt, size);

    getProperty(XN_STREAM_PROPERTY_ZERO_PLANE_DISTANCE, &nInt, &size);
    raisePropertyChanged(XN_STREAM_PROPERTY_ZERO_PLANE_DISTANCE, &nInt, size);

    getProperty(ONI_STREAM_PROPERTY_MAX_VALUE, &nInt, &size);
    raisePropertyChanged(ONI_STREAM_PROPERTY_MAX_VALUE, &nInt, size);

    unsigned short nBuff[10001];
    size = sizeof(S2D_);
    getProperty(XN_STREAM_PROPERTY_S2D_TABLE, nBuff, &size);
    raisePropertyChanged(XN_STREAM_PROPERTY_S2D_TABLE, nBuff, size);

    size = sizeof(D2S_);
    getProperty(XN_STREAM_PROPERTY_D2S_TABLE, nBuff, &size);
    raisePropertyChanged(XN_STREAM_PROPERTY_D2S_TABLE, nBuff, size);

    oni::driver::StreamBase::notifyAllProperties();
}

OniStatus DepthStream::SetCropping(OniCropping* cropping)
{
    std::cout << "SetCropping" << std::endl;
    return ONI_STATUS_OK;
}

OniStatus DepthStream::GetCropping(OniCropping* cropping)
{
    std::cout << "GetCropping" << std::endl;
    return ONI_STATUS_OK;

}

/* depth video modes reference

FREENECT_DEPTH_11BIT        = 0, //< 11 bit depth information in one uint16_t/pixel
FREENECT_DEPTH_10BIT        = 1, //< 10 bit depth information in one uint16_t/pixel
FREENECT_DEPTH_11BIT_PACKED = 2, //< 11 bit packed depth information
FREENECT_DEPTH_10BIT_PACKED = 3, //< 10 bit packed depth information
FREENECT_DEPTH_REGISTERED   = 4, //< processed depth data in mm, aligned to 640x480 RGB
FREENECT_DEPTH_MM           = 5, //< depth to each pixel in mm, but left unaligned to RGB image
FREENECT_DEPTH_DUMMY        = 2147483647, //< Dummy value to force enum to be 32 bits wide

ONI_PIXEL_FORMAT_DEPTH_1_MM = 100,
ONI_PIXEL_FORMAT_DEPTH_100_UM = 101,
ONI_PIXEL_FORMAT_SHIFT_9_2 = 102,
ONI_PIXEL_FORMAT_SHIFT_9_3 = 103,
*/
