#pragma once

#include <algorithm> // for transform()
#include <math.h> // for M_PI
#include "libfreenect.hpp"
#include "Driver/OniDriverAPI.h"
#include "PS1080.h"
#include "VideoStream.hpp"

#include <iostream>

namespace FreenectDriver {
  class DepthStream : public VideoStream {
  public:
    // from NUI library and converted to radians - please check
      static const float COLOR_DIAGONAL_FOV = 73.9 * (M_PI / 180);
      static const float COLOR_HORIZONTAL_FOV = 62.0 * (M_PI / 180);
      static const float COLOR_VERTICAL_FOV = 48.0 * (M_PI / 180);

      static const float DIAGONAL_FOV = 70 * (M_PI / 180);
      static const float HORIZONTAL_FOV = 58.5 * (M_PI / 180);
      static const float VERTICAL_FOV = 45.6 * (M_PI / 180);
    // from DepthKinectStream.cpp - please check
    static const int MAX_VALUE = 10000;
    static const unsigned long long GAIN_VAL = 42;
    static const unsigned long long CONST_SHIFT_VAL = 200; //<< should be 800  https://groups.google.com/forum/#!msg/openkinect/tc45lqPLU2Y/GMglTFh9ecAJ
    static const unsigned long long MAX_SHIFT_VAL = 2047;
    static const unsigned long long PARAM_COEFF_VAL = 4;
    static const unsigned long long SHIFT_SCALE_VAL = 10;
    static const unsigned long long ZERO_PLANE_DISTANCE_VAL = 120;
    static const double ZERO_PLANE_PIXEL_SIZE_VAL = 0.10520000010728836;
    static const double EMITTER_DCMOS_DISTANCE_VAL = 7.5;
      
  private:
    typedef std::map< OniVideoMode, std::pair<freenect_depth_format, freenect_resolution> > FreenectDepthModeMap;
    static const OniSensorType sensor_type = ONI_SENSOR_DEPTH;
    OniImageRegistrationMode image_registration_mode;

    static FreenectDepthModeMap getSupportedVideoModes();
    virtual OniStatus setVideoMode(OniVideoMode requested_mode);
    void populateFrame(void* data, OniFrame* frame) const;

  public:
    DepthStream(Freenect::FreenectDevice* pDevice);
    //~DepthStream() { }
    
    static OniSensorInfo getSensorInfo() {
      FreenectDepthModeMap supported_modes = getSupportedVideoModes();
      OniVideoMode* modes = new OniVideoMode[supported_modes.size()];
      std::transform(supported_modes.begin(), supported_modes.end(), modes, RetrieveKey());
      return { sensor_type, SIZE(modes), modes }; // sensorType, numSupportedVideoModes, pSupportedVideoModes
    }
    
    OniImageRegistrationMode getImageRegistrationMode() const { return image_registration_mode; }
    OniStatus setImageRegistrationMode(OniImageRegistrationMode mode) {
      if (!isImageRegistrationModeSupported(mode))
        return ONI_STATUS_NOT_SUPPORTED;
      image_registration_mode = mode;

      setVideoMode(video_mode);
      return ONI_STATUS_OK;
    }
    
    // from StreamBase
    OniBool isImageRegistrationModeSupported(OniImageRegistrationMode mode) { return (mode == ONI_IMAGE_REGISTRATION_OFF || mode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR); }

    OniStatus getProperty(int propertyId, void* data, int* pDataSize);
    OniStatus setProperty(int propertyId, const void* data, int dataSize) {
      switch (propertyId) {
        default:
          return VideoStream::setProperty(propertyId, data, dataSize);

        case ONI_STREAM_PROPERTY_MIRRORING:   // OniBool
          if (dataSize != sizeof(OniBool)) {
            printf("Unexpected size: %d != %lu\n", dataSize, sizeof(OniBool));
            return ONI_STATUS_ERROR;
          }
          mirroring = *(static_cast<const OniBool*>(data));
          return ONI_STATUS_OK;
      }
    }

    virtual void notifyAllProperties();

    virtual OniStatus SetCropping(OniCropping* cropping);
    virtual OniStatus GetCropping(OniCropping* cropping);

    OniStatus convertDepthToColorCoordinates(StreamBase* colorStream, int depthX, int depthY, OniDepthPixel depthZ, int* pColorX, int* pColorY)
    {
        std::cout << "convertDepthToColorCoordinates" << std::endl;
        /*
        // take video mode from the color stream
        XnOniMapStream* pColorStream = (XnOniMapStream*)colorStream;

        OniVideoMode videoMode;
        XnStatus retVal = pColorStream->GetVideoMode(&videoMode);
        if (retVal != XN_STATUS_OK)
        {
            XN_ASSERT(FALSE);
            return ONI_STATUS_ERROR;
        }

        // translate it to the internal property
        XnPixelRegistration pixelArgs;
        pixelArgs.nDepthX = depthX;
        pixelArgs.nDepthY = depthY;
        pixelArgs.nDepthValue = depthZ;
        pixelArgs.nImageXRes = videoMode.resolutionX;
        pixelArgs.nImageYRes = videoMode.resolutionY;
        int pixelArgsSize = sizeof(pixelArgs);

        if (ONI_STATUS_OK != getProperty(XN_STREAM_PROPERTY_PIXEL_REGISTRATION, &pixelArgs, &pixelArgsSize))
        {
            return ONI_STATUS_ERROR;
        }

        // take output
        *pColorX = pixelArgs.nImageX;
        *pColorY = pixelArgs.nImageY;
*/

        return ONI_STATUS_OK;
    }

  };


}
