// Copyright 2018 SICK AG. All rights reserved.
#pragma once

#ifdef __linux__
#   define SICK_CALIB_EXPORT_TO_DLL

#else
#if defined(DLL_EXPORTS) // inside DLL
#   define SICK_CALIB_EXPORT_TO_DLL   __declspec(dllexport)
#else // outside DLL
#   define SICK_CALIB_EXPORT_TO_DLL   __declspec(dllimport)
#endif  

#endif // __linux__



#include <string>
#include <memory>
#include "CalibrationAPI.h"


namespace ranger3calibration {

	struct CalibrationParameters;

	/*! \brief Class for calibration output data
	*
	*	Contains the output data from the calibration filter. 
	*	Memory is managed by the object and is deleted when the object 
	*	is deleted or runs out of scope.
	*/
	class CalibrationData {
	public:

		/*! \brief Default constructor
		*
		*	If using the default constructor the resize function should be called after to allocated memory.
		*/
		CalibrationData() { width = 0; height = 0; } //Default constructor
		/*! \brief Constructor
		*
		*	Allocates memory to fit data of size width and height by calling resize.
		*/
		CalibrationData(int64_t width, int64_t height) {
			resize(width, height);
		}

		/*! \brief Copy-constructor
		*
		*	Performs a deep-copy of internal members rangeX and rangeR. 
		*/
		CalibrationData(const CalibrationData& other) {
			resize(other.width, other.height);
			for (int i = 0; i < width*height; i++) {
				this->rangeX[i] = other.rangeX[i];
				this->rangeR[i] = other.rangeR[i];
			}
		}

		/*! \brief assignment-operator
		*
		*	Performs a deep-copy of internal members rangeX and rangeR.
		*/
		CalibrationData& operator=(CalibrationData other) {
			resize(other.width, other.height);
			for (int i = 0; i < width*height; i++) {
				this->rangeX[i] = other.rangeX[i];
				this->rangeR[i] = other.rangeR[i];
			}
			return *this;
		}

		/*! \brief Resize's the buffer capacity
		*
		*	Allocates memory to fit data of size width and height and allocates memory. Previously held memory is released.
		*/
		void resize(int64_t aWidth, int64_t aHeight) {
			width = aWidth;
			height = aHeight;

			rangeX = std::make_unique<float[]>(width*height);
			rangeR = std::make_unique<float[]>(width*height);
		}

		int64_t width;
		int64_t height;

		std::unique_ptr<float[]> rangeX;/*!< The data for the rangeX. Each point in the array represents that points x-coordinate. */
		std::unique_ptr<float[]> rangeR;/*!< The data for the rangeR. Each point in the image represents that points range-value. This is usually the z-coordinate or height in mm. */
	};

	/*! \brief Filter for applying a calibration file to data
	*
	*	A filter which given the CalibrationParameters and the SensorTraits and input data, converts 
	*   the range value expressed in pixels to range values expressed in mm's. 
	*
	*	As output RangeR and RangeX will be emitted. 
	*/
	EXPORT_TO_DLL class CalibrationFilter {
	public:
		CalibrationFilter() = delete;
		/*! \brief Constructor for the filter
		*
		*	Construct the filter by providing the CalibrationParameters p, and the SensorTraits s.
		*   WorldRangeTraits will be calculated upon initialization. 
		*	\param[in] missingDataValue - Change missing data (0) to this value. (e.g. 0 or NaN).
		*/
		CalibrationFilter(CalibrationParameters p, SensorTraits s, float _missingDataValue = 0.0f) :
			_mMissingDataValue(_missingDataValue), sensorTraits(s), calibrationParameters(p)
		{
			calculateWorldTraits();
		}

		/*! \brief Sets the CalibrationParameters
		*
		*	Note. One need to run CalculateWorldRangeTraits manually after setting new CalibrationParameters
		*	\param[in] p - The calibration parameters for this filter.
		*/
		void setCalibrationParameters(CalibrationParameters p);
		
		/*! \brief Sets the SensorTraits
		*
		*	Note. One need to run CalculateWorldRangeTraits manually after setting new SensorTraits
		*/
		void setSensorTraits(SensorTraits s);

		/*! \brief Calculates the WorldRangeTraits
		*/
		EXPORT_TO_DLL void calculateWorldTraits();

		/*! \brief Performs calibration on the inData
		*	
		*	Apply the calibration on inData and output the result in data.
		*	The CalibrationData should be initialized prior to executing this function, 
		*   the convenience function generateOutputContainer is provided for this purpose. 
		*
		*	It's possible to run the calibration with SSE-optimization. This has better 
		*	better performance on certain hardware. SSE-optimization will be disabled internally 
		*	if the buffer width is not a multiple of 8. 
		*
		*	\param[in] inData - Pointer to the range data on 16 bit format. (not 12 bit). 
		*	\param[in,out] calibData - Contains the resulting calibrated data after calibration. Must be initialized beforehand.
		*
		*/
		EXPORT_TO_DLL void applyCalibration(const uint8_t* inData, CalibrationData& outputData, bool SSEEnabled = false);

		/*! \brief Get the missing data value for the calibration filter.
		*
		*/
		EXPORT_TO_DLL float getMissingDataValue();
		/*! \brief Set the missing data value for the calibration filter.
		*
		*/
		void setMissingDataValue(float newVal);

		/*! \brief Returns a output containter for calibration data
		*
		*	Convenience function for creating the output container needed for applyCalibration.
		*/
		EXPORT_TO_DLL CalibrationData generateOutputContainer();

		/*! \brief Calibrates a single point
		 *
		 *	Calibrates a single point. Note, convenience function, slightly slower than calibratePointFast
		 */
		EXPORT_TO_DLL void calibratePoint(
			const unsigned short& col,
			const unsigned short & range,
			float & xOut,
			float & rangeOut);

		SensorTraits sensorTraits;
		WorldTraits worldTraits;

	private:

		void applyCalibrationVersion1(const uint8_t* inData, CalibrationData& outputData, bool SSEEnabled = false);
		void applyCalibrationVersion2(const uint8_t* inData, CalibrationData& outputData, bool SSEEnabled = false);
		void calculateWorldTraitsV1();
		void calculateWorldTraitsV2();

		CalibrationParameters calibrationParameters;
		float _mMissingDataValue = 0;
	
		/*! \brief Calibrates a single point
		*
		*	Calibrates a single point.
		*/
		inline void calibratePointFast(
			const int& col,
			const float& scaleX,
			const float& xOffset,
			const float& scaleZ,
			const float& zOffset,
			const unsigned short& data,
			float* xOut,
			float* rOut);
	};
}