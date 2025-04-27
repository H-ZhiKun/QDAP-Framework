// Copyright 2018 SICK AG. All rights reserved.
#include "../include/RectificationFilter.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <cstring>
using namespace ranger3calibration;

int RectificationFilter::applyRectification(
	const uint8_t* reflectance, 
	const CalibrationData& calibData, 
	RectificationData& rectData, 
	const RectificationMethod method) 
{
	if (rectData.range == nullptr) {
		throw std::runtime_error("RectificationData uninitialized.");
	}
	else if (reflectance != nullptr && rectData.reflectanceByte == nullptr) {
		throw std::runtime_error("Passing reflectance to Rectification filter with output not containing reflectance is invalid.");
	}
	//Get pointers to output data
	float* range = rectData.range.get();
	//float* reflectance_out = rectData.reflectance.get();
	uint8_t* reflectance_out = rectData.reflectanceByte.get();
	float* weight = rectData.weight.get();

	//Get pointers to input data
	const float* rangeR = calibData.rangeR.get();
	const float* rangeX = calibData.rangeX.get();

	//We need to clear the data before the rectification. This is important as multiple values can end up in the same container.
	if (method == RectificationMethod::Mean) {
		std::fill(range, range + rectificationWidth * height, 0.0f);
	}
	else {
		std::fill(range, range + rectificationWidth * height, _mMissingDataValue);
	}
	if (reflectance != nullptr) { std::fill(reflectance_out, reflectance_out + rectificationWidth * height, 0); }
	std::fill(weight, weight + rectificationWidth * height, 0.0f);


	// Accumulate values for all valid points and their weights. 
//#pragma omp parallel for
	for (int j = 0; j < height; j++) {
		float* range_it = range + rectificationWidth * j;
		//float* refl_it = reflectance_out + rectificationWidth * j;
		uint8_t* refl_it = reflectance_out + rectificationWidth * j;
		float* weight_it = weight + rectificationWidth * j;


		for (int i = 0; i < width; i++) {
			const float* rangeRIn = rangeR + i + j*width;
			const float* rangeXIn = rangeX + i + j*width;
			const unsigned char* reflIn = reflectance == nullptr ? nullptr : reflectance + i + j*width;


			if (*rangeRIn != _mMissingDataValue)
			{
				if (xTrans->inRange(*rangeXIn))
				{

					float xf = xTrans->apply(*rangeXIn);
					unsigned long xi = (unsigned long)xf;


					if (method == RectificationMethod::Top || method == RectificationMethod::Bottom) {
						if (xi > (unsigned long)rectificationWidth) {
							//Should not happen.
							throw std::runtime_error("X out of range after range check. ");
						}
						else if (xi == rectificationWidth) {
							xi = rectificationWidth - 1;
						}
						bool topComp = (range_it[xi] < static_cast<float>(*rangeRIn) && method == RectificationMethod::Top);
						bool bottomComp = (range_it[xi] > static_cast<float>(*rangeRIn) && method == RectificationMethod::Bottom);

#ifdef __linux__
						if (isnanf(range_it[xi]) || range_it[xi] == _mMissingDataValue || topComp || bottomComp) { // if no value yet or 'better' value
							range_it[xi] = static_cast<float>(*rangeRIn);
							if (reflectance != nullptr) { refl_it[xi] = (*reflIn); }
						}
#else
						if (isnan(range_it[xi]) || range_it[xi] == _mMissingDataValue || topComp || bottomComp) { // if no value yet or 'better' value
							range_it[xi] = static_cast<float>(*rangeRIn);
							if (reflectance != nullptr) { refl_it[xi] = (*reflIn); }
						}
#endif
					}
					else if (method == RectificationMethod::Mean) {
						if (xi > (unsigned long)rectificationWidth)
						{
							throw std::runtime_error("X out of range after range check. ");
						}
						else if (xi + 1 >= (unsigned long)rectificationWidth)
						{
							float w = (float)rectificationWidth - xf;
							weight_it[rectificationWidth - 1] += w;
							range_it[rectificationWidth - 1] += w * static_cast<float>(*rangeRIn);
							if (reflectance != nullptr) { refl_it[rectificationWidth - 1] += static_cast<uint8_t>(w * (*reflIn)); }
						}
						else
						{
							float w = xf - (float)xi;
							float wi = 1.0f - w;
							weight_it[xi] += wi;
							weight_it[xi + 1] += w;
							range_it[xi] += wi * static_cast<float>(*rangeRIn);
							range_it[xi + 1] += w * static_cast<float>(*rangeRIn);
							if (reflectance != nullptr) {
								refl_it[xi] += static_cast<uint8_t>(wi * (*reflIn));
								refl_it[xi + 1] += static_cast<uint8_t>(w * (*reflIn));
							}
						}
					}
				}
				else
				{
					xOutOfRange++;
				}
			}
		}
		// Apply weights to obtain final value, only do this for RectificationMethod::Mean
		if (method == RectificationMethod::Mean) {
			for (int k = 0; k < rectificationWidth; k++) {

				if (*weight_it > 0)
				{
					*range_it /= *weight_it;
					if (reflectance != nullptr) {
						*refl_it = static_cast<uint8_t>(static_cast<float>(*refl_it) / (*weight_it));
					}
				}
				else {
					*range_it = _mMissingDataValue;
				}

				++weight_it;
				++range_it;
				if (reflectance != nullptr) {
					++refl_it;
				}
			}
		}
	}
	return 0;
}

int RectificationFilter::applyRectification(const CalibrationData& calibData, RectificationData& rectData, const RectificationMethod method)
{
	return applyRectification(nullptr, calibData, rectData, method);
}

RectificationData ranger3calibration::RectificationFilter::generateOutputContainer(bool generateRectification)
{
	checkIfContainerIsValid();

	return RectificationData(rectificationWidth, height, generateRectification);
}

void ranger3calibration::RectificationFilter::checkIfContainerIsValid()
{
	if (width == 0 || height == 0) {
		throw std::runtime_error("Rectification filter needs to be initialized with a height and width larger than zero before running generateOutputContainer");
	}
}

float ranger3calibration::RectificationFilter::getMissingDataValue()
{
	return _mMissingDataValue;
}

void ranger3calibration::RectificationFilter::setMissingDataValue(float newVal)
{
	_mMissingDataValue = newVal;
}

int RectificationFilter::setBounds(const float xMin, const float xMax)
{
	xTrans = new XTransformation(xMin, xMax, rectificationWidth);

	return 0;
}

int64_t ranger3calibration::RectificationData::getSize()
{
	return width*height;
}

void ranger3calibration::RectificationData::copyRange(float * outRange)
{
	memcpy(outRange, range.get(), sizeof(float) * getSize());
}

void ranger3calibration::RectificationData::copyReflectance(uint8_t * outReflectance)
{
	memcpy(outReflectance, reflectanceByte.get(), sizeof(float) * getSize());
}
