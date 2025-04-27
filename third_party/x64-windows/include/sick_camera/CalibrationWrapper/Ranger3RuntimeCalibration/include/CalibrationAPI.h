#pragma once

#ifdef __linux__
#   define EXPORT_TO_DLL

#else
#if defined(DLL_EXPORTS) // inside DLL
#   define EXPORT_TO_DLL   __declspec(dllexport)
#else // outside DLL
#   define EXPORT_TO_DLL   __declspec(dllimport)
#endif  

#endif // __linux__

#include <cstdint>


#ifdef __cplusplus
namespace ranger3calibration {
	extern "C" {
#endif
		/*! \brief Struct for Calibration Parameters
		 *
		 *	Can be created and then initialized by using the loadCalibrationParameters function.
		 */
		typedef struct CalibrationParameters
		{
			//Parameters shared between version1 and version2 calibration
			double H[9];
			double opticalCenter[2];
			double lensDistortion[4];


			//Only for version2 calibration
			double scheimpflugRoll;
			double scheimpflugTilt;

			double coverage;

			int version = -1;
			bool SSE = false;
			float missingData = 0.0f;
		} CalibrationParameters;

		/*! \brief Class for Sensor Traits
		 *
		 *	Specifies and passes around information concerning the sensor and the AOI used during laser-peak extraction
		 */
		typedef struct SensorTraits {
			int width; //Width of input buffer
			int height; // Height of input buffer
			int aoiHeight;
			float scaleZ = 0.0625f;
			float scaleX = 1.0f;
			float xOffset = 0.0f;
			float zOffset = 0.0f;
		} SensorTraits;

		/*! \brief Struct for WorldRangeTraits
		*
		*	The values of the struct construct a 2-dimensional boundingbox within which all valid values after calibration should lie.
		*	In addition can the pixel x-resolution be described as (upperBoundX - lowerBoundX)/bufferWidth after rectification
		*/
		typedef struct WorldTraits
		{
			float lowerBoundX;
			float upperBoundX;
			float lowerBoundR;
			float upperBoundR;
		} WorldTraits;

		/*! \brief Enum for various rectification methods
		*
		*	Top		- Selects for each output pixel the highest range value
		*	Bottom	- Selects for each output pixel the lowest range value
		*	Mean	- Selects for each output pixel the weighted mean value of all range-values for that pixel.
		*/
		typedef enum RectificationMethod {
			Top,
			Bottom,
			Mean
		} RectificationMethod;

		/*! \brief Function for loading a calibration xml file
		*
		* A calibration file created with the Easy Ranger Calibration GUI can be loaded to create a CalibrationParameters object.
		*/
		EXPORT_TO_DLL void loadCalibrationParameters(
			const char* pathToXML, 
			CalibrationParameters* outParameters);

		/*! \brief Function for calibrating an unsigned char buffer containing range buffer
		*
		* Input - rawRangeBuffer, an uncalibrated range buffer of size width*height of type unsigned char
		* Output - rectifiedRangeBuffer, a rectified and calibrated range buffer of size width*height if type float
		* Output - worldTraits, specifying upper and lower bounds for range and x-axis
		* Input - SensorTraits, information regarding cameras sensor configuration this can be accessed from the camera configuration
		* Input - parameters, information regarding the calibration, this can be loaded from a calibration.xml
		* Input - rectificationMethod, what rectification method will be used Top, Bottom and Mean are available.
		* 
		*/
		EXPORT_TO_DLL void rectifyAndCalibrate(
			unsigned char* rawRangeBuffer,
			float* rectifiedRangeBuffer,
			WorldTraits* worldTraits,
			SensorTraits* sensorTraits,
			CalibrationParameters* parameters,
			RectificationMethod rectificationMethod);

#ifdef __cplusplus
	};
};
#endif

