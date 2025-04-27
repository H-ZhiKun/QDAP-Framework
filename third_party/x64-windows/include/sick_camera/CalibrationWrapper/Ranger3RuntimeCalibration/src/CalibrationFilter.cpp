// Copyright 2018 SICK AG. All rights reserved.
#include "../include/CalibrationFilter.h"
#include <algorithm>
#include <iostream>

#ifndef __linux__
#include <xmmintrin.h>
#include <immintrin.h>
#include <emmintrin.h>
#endif

#include <cassert>
#include <iterator>
#include <cmath>

#include "../include/Point2D.h"
#include "../include/Transformations.h"
#include "../../tinyxml2/tinyxml2.h"

using namespace tinyxml2;
using namespace ranger3calibration;


void ranger3calibration::CalibrationFilter::setCalibrationParameters(CalibrationParameters p)
{
	calibrationParameters = p;
}

void ranger3calibration::CalibrationFilter::setSensorTraits(SensorTraits s) {
	sensorTraits = s;
};

void ranger3calibration::CalibrationFilter::applyCalibration(const uint8_t * inData, CalibrationData & outputData, bool SSEEnabled)
{
	if (calibrationParameters.version == 1) {
		applyCalibrationVersion1(inData, outputData, SSEEnabled);
	}
	else if (calibrationParameters.version == 2) {
		applyCalibrationVersion2(inData, outputData, SSEEnabled);
	}
	else {
		assert(false && "Unknown calibration type");
	}

}

void ranger3calibration::CalibrationFilter::applyCalibrationVersion2(const uint8_t * inData, CalibrationData & outputData, bool SSEEnabled)
{

	const int width = sensorTraits.width;
	const int height = sensorTraits.height;
	const float scaleX = sensorTraits.scaleX;
	const float scaleZ = sensorTraits.scaleZ;
	const float xOffset = sensorTraits.xOffset;
	const float zOffset = sensorTraits.zOffset;

	//Setup v2 layout of data.
	const double lens[2 + 4]{ 
		calibrationParameters.opticalCenter[0],
		calibrationParameters.opticalCenter[1],
		calibrationParameters.lensDistortion[0],
		calibrationParameters.lensDistortion[1],
		calibrationParameters.lensDistortion[2],
		calibrationParameters.lensDistortion[3] 
	};
	const double scheimpflugTilt{ calibrationParameters.scheimpflugTilt };
	const double scheimpflugRoll{ calibrationParameters.scheimpflugRoll };
	double* Ht = calibrationParameters.H;
	const double H[9]{ Ht[0],Ht[1], Ht[2], Ht[3], Ht[4], Ht[5], Ht[6], Ht[7], Ht[8] };


	const uint16_t* wordData = (uint16_t*)inData;
	if (outputData.width != width || outputData.height != height) {
		throw std::runtime_error("Buffer size and output size do not match");
	}

	float* wXPtr = outputData.rangeX.get();
	float* wRangePtr = outputData.rangeR.get();
	Point2D p{}; //Pre-construct the point
	Point2D resPoint{};

#ifdef __linux__
	SSEEnabled = false;
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            if (*wordData == 0) {
                *wXPtr = _mMissingDataValue;
                *wRangePtr = _mMissingDataValue;
            }
            else {
                p.x = i * scaleX + xOffset;
                p.y = *wordData*scaleZ + zOffset;
                resPoint = sensorToWorld(p, lens, scheimpflugRoll, scheimpflugTilt, H);
                *wXPtr = (float)resPoint.x;
                *wRangePtr = (float)resPoint.y;
            }
            wordData++;
            wRangePtr++;
            wXPtr++;
        }
    }
#else
	if (SSEEnabled) {
		const Vector4d H_vec4[9] = { H[0],H[1], H[2], H[3], H[4] , H[5] , H[6] ,H[7],H[8] };
		const Vector4d lens_vec4[6] = { lens[0],lens[1], lens[2], lens[3], lens[4], lens[5] };
		const Vector4d sch_sx{ scheimpflugRoll };
		const Vector4d sch_sy{ scheimpflugTilt };

		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width / 8; i++) {
				__m128i data = _mm_loadu_si128((__m128i*)&wordData[i * 8 + j * width]); //Contains 8 x 16 bit integers
																						//Convert to float
				__m128i dataiLo = _mm_unpacklo_epi16(data, _mm_set1_epi16(0));
				__m128i dataiHi = _mm_unpackhi_epi16(data, _mm_set1_epi16(0));
				__m256d dataLo = _mm256_cvtepi32_pd(dataiLo);
				__m256d dataHi = _mm256_cvtepi32_pd(dataiHi);

				__m256d dataArr[2] = { dataLo, dataHi };

				for (int k = 0; k < 2; k++) {
					Vector4d px{ (double)(i * 8 + k * 4),(double)(i * 8 + 1 + k * 4),(double)(i * 8 + 2 + k * 4),(double)(i * 8 + 3 + k * 4) }; //x_i x_i+1 x_i+2 x_i+3
					px = px * Vector4d{ scaleX };
					px = px + Vector4d{ xOffset };
					Vector4d py{ dataArr[k] };
					Vector4d outputMask = (py == Vector4d{ 0 }); //Create mask so that missing data can be inserted later

					py = py * Vector4d{ scaleZ };
					py = py + Vector4d{ zOffset };

					sensorToWorld(px, py, lens_vec4, sch_sx, sch_sy, H_vec4);

					px = px.select(Vector4d{ _mMissingDataValue }, outputMask); //Set output to missingdata where input was zero
					py = py.select(Vector4d{ _mMissingDataValue }, outputMask); //Set output to missingdata where input was zero

					__m128 temp = _mm256_cvtpd_ps(px);
					_mm_storeu_ps(wXPtr, temp);

					temp = _mm256_cvtpd_ps(py);
					_mm_storeu_ps(wRangePtr, temp);

					wRangePtr += 4;
					wXPtr += 4;
				}
			}
		}
	}
	else {
		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width; i++) {
				if (*wordData == 0) {
					*wXPtr = _mMissingDataValue;
					*wRangePtr = _mMissingDataValue;
				}
				else {
					p.x = i * scaleX + xOffset;
					p.y = *wordData*scaleZ + zOffset;
					resPoint = sensorToWorld(p, lens, scheimpflugRoll, scheimpflugTilt, H);
					*wXPtr = (float)resPoint.x;
					*wRangePtr = (float)resPoint.y;
				}
				wordData++;
				wRangePtr++;
				wXPtr++;
			}
		}
	}
#endif // __linux__

}

void CalibrationFilter::applyCalibrationVersion1(const uint8_t *inData, CalibrationData& outputData, bool SSE)
{
	const uint16_t* wordData = (uint16_t*)inData;

	const int width = sensorTraits.width;
	const int height = sensorTraits.height;
	const float scaleX = sensorTraits.scaleX;
	const float scaleZ = sensorTraits.scaleZ;
	const float xOffset = sensorTraits.xOffset;
	const float zOffset = sensorTraits.zOffset;

	//Setup float versions of the data. Precision loss?
	float lensDistortion[4];
	float H[9];
	float opticalCenter[2];

#ifdef __linux__
	std::transform(
		calibrationParameters.opticalCenter + 0,
		calibrationParameters.opticalCenter + 2,
		opticalCenter,
		[](float a) { return static_cast<float>(a);
	});
	std::transform(
		calibrationParameters.lensDistortion + 0,
		calibrationParameters.lensDistortion + 4,
		lensDistortion + 0,
		[](float a) { return static_cast<float>(a);
	});
	std::transform(
		calibrationParameters.H + 0,
		calibrationParameters.H + 9,
		H,
		[](float a) { return static_cast<float>(a);
	});

#else

	std::transform(
		calibrationParameters.opticalCenter + 0,
		calibrationParameters.opticalCenter + 2,
		stdext::checked_array_iterator<float*>(opticalCenter + 0, 2),
		[](float a) { return static_cast<float>(a);
	});
	std::transform(
		calibrationParameters.lensDistortion + 0,
		calibrationParameters.lensDistortion + 4,
		stdext::checked_array_iterator<float*>(lensDistortion + 0, 4),
		[](float a) { return static_cast<float>(a);
	});
	std::transform(
		calibrationParameters.H + 0,
		calibrationParameters.H + 9,
		stdext::checked_array_iterator<float*>(H + 0, 9),
		[](float a) { return static_cast<float>(a);
	});

#endif // __linux__


	if (outputData.width != width || outputData.height != height) {
		throw std::runtime_error("Buffer size and output size do not match");
	}

#ifndef __linux__
	if (SSE && width % 8 == 0) {

		const float* pLensDist = lensDistortion;
		//Constants
		__m128 scaleX_s = __m128{scaleX, scaleX, scaleX, scaleX};
		__m128 scaleZ_s = __m128{scaleZ, scaleZ, scaleZ, scaleZ};
		__m128 offsetX_s = __m128{xOffset, xOffset, xOffset, xOffset};
		__m128 offsetZ_s = __m128{zOffset, zOffset, zOffset, zOffset};
		__m128 pOC0 = __m128{opticalCenter[0], opticalCenter[0], opticalCenter[0], opticalCenter[0]};
		__m128 pOC1 = __m128{opticalCenter[1], opticalCenter[1], opticalCenter[1], opticalCenter[1]};
		__m128 pLensDist0 = __m128{ pLensDist[0], pLensDist[0], pLensDist[0], pLensDist[0] };
		__m128 pLensDist1 = __m128{ pLensDist[1], pLensDist[1], pLensDist[1], pLensDist[1] };
		__m128 pLensDist2 = __m128{ pLensDist[2], pLensDist[2], pLensDist[2], pLensDist[2] };
		__m128 pLensDist3 = __m128{ pLensDist[3], pLensDist[3], pLensDist[3], pLensDist[3] };
		__m128 nan = __m128{_mMissingDataValue, _mMissingDataValue, _mMissingDataValue, _mMissingDataValue};
		__m128 H0 = __m128{H[0], H[0], H[0], H[0]};
		__m128 H1 = __m128{H[1], H[1], H[1], H[1]};
		__m128 H2 = __m128{H[2], H[2], H[2], H[2]};
		__m128 H3 = __m128{H[3], H[3], H[3], H[3]};
		__m128 H4 = __m128{H[4], H[4], H[4], H[4]};
		__m128 H5 = __m128{H[5], H[5], H[5], H[5]};
		__m128 H6 = __m128{H[6], H[6], H[6], H[6]};
		__m128 H7 = __m128{H[7], H[7], H[7], H[7]};
		__m128 H8 = __m128{H[8], H[8], H[8], H[8]};

		//Variables
		__m128 xUD = __m128{0, 0, 0, 0};
		__m128 yUD = __m128{0, 0, 0, 0};
		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width / 8; i++) { // We read 8 values at a time.
				__m128i data = _mm_loadu_si128((__m128i*)&wordData[i * 8 + j * width]); //Contains 8 x 16 bit integers

				//Convert to float
				__m128i dataiLo = _mm_unpacklo_epi16(data, _mm_set1_epi16(0));
				__m128i dataiHi = _mm_unpackhi_epi16(data, _mm_set1_epi16(0));
				__m128 datafLo = _mm_cvtepi32_ps(dataiLo);
				__m128 datafHi = _mm_cvtepi32_ps(dataiHi);

				__m128 maskLo = _mm_cmpneq_ps(datafLo, __m128{0, 0, 0, 0});
				__m128 maskHi = _mm_cmpneq_ps(datafHi, __m128{0, 0, 0, 0});

				__m128 loopData[2] = { datafLo,datafHi };
				__m128 mask[2] = { maskLo,maskHi };

				//Each dataf(Lo|Hi) contains 4 floats, but we read in 8 values so we need to loop over both.  
				for (int k = 0; k < 2; k++) {
					__m128 x = _mm_add_ps(_mm_mul_ps(
						__m128{
							(float)i * 8 + 0 + 4 * k,
							(float)i * 8 + 1 + 4 * k,
							(float)i * 8 + 2 + 4 * k,
							(float)i * 8 + 3 + 4 * k},
							scaleX_s),
						offsetX_s
					);
					__m128 y = _mm_add_ps(
						_mm_mul_ps(
							loopData[k],
							scaleZ_s),
						offsetZ_s);

					__m128 x3 = _mm_sub_ps(x, pOC0);
					__m128 y3 = _mm_sub_ps(y, pOC1);

					__m128 x2 = _mm_mul_ps(x3, x3);// x3 < y3 < 13312. pOC1 > 0 => y3 < 13312 < 2^14
					__m128 y2 = _mm_mul_ps(y3, y3);// y3 < 13312. pOC1 > 0 => y3 < 13312 < 2^14
					__m128 r2 = _mm_add_ps(x2, y2);// r2 < 2^14 + 2 ^14 <= 2^15
					__m128 r4 = _mm_mul_ps(r2, r2);// r4 < 2^15 * 2^15 < 2^32


					//xUD = (float)((x + x3 * (pLensDist[0] * r2 + pLensDist[1] * r2*r2) + 2 * pLensDist[2] * x3*y3 + pLensDist[3] * (r2 + 2 * x2)));
					__m128 temp1 = _mm_add_ps(_mm_mul_ps(pLensDist0, r2), _mm_mul_ps(pLensDist1, r4));//(pLensDist[0] * r2 + pLensDist[1] * r2*r2)
					temp1 = _mm_mul_ps(x3, temp1);
					temp1 = _mm_add_ps(x, temp1);
					__m128 temp2 = _mm_mul_ps(pLensDist3, _mm_add_ps(r2, _mm_mul_ps(__m128{2, 2, 2, 2}, x2))); //  pLensDist[3] * (r2 + 2 * x2)
					__m128 temp3 = _mm_mul_ps(_mm_mul_ps(__m128{2, 2, 2, 2}, pLensDist2), _mm_mul_ps(x3, y3)); // 2 * pLensDist[2] * x3*y3
					xUD = _mm_add_ps(temp1, _mm_add_ps(temp2, temp3));

					//yUD = (float)((y + y3 * (pLensDist[0] * r2 + pLensDist[1] * r2*r2) + 2 * pLensDist[3] * x3*y3 + pLensDist[2] * (r2 + 2 * y2)));

					temp1 = _mm_add_ps(_mm_mul_ps(pLensDist0, r2), _mm_mul_ps(pLensDist1, r4));//(pLensDist[0] * r2 + pLensDist[1] * r2*r2)
					temp1 = _mm_mul_ps(y3, temp1);
					temp1 = _mm_add_ps(y, temp1);
					temp2 = _mm_mul_ps(pLensDist2, _mm_add_ps(r2, _mm_mul_ps(__m128{2, 2, 2, 2}, y2))); //  pLensDist[2] * (r2 + 2 * x2)
					temp3 = _mm_mul_ps(_mm_mul_ps(__m128{2, 2, 2, 2}, pLensDist3), _mm_mul_ps(x3, y3)); // 2 * pLensDist[3] * x3*y3
					yUD = _mm_add_ps(temp1, _mm_add_ps(temp2, temp3));

					//float alpha = H[6] * xUD + H[7] * yUD + H[8];
					__m128 alpha = _mm_add_ps(_mm_add_ps(_mm_mul_ps(H6, xUD), _mm_mul_ps(H7, yUD)), H8);

					//x_out[i + j * width] = (H[0] * xUD + H[1] * yUD + H[2]) / alpha;
					//range[i + j * width] = (H[3] * xUD + H[4] * yUD + H[5]) / alpha;

					__m128 x_out_local = _mm_div_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(H0, xUD), _mm_mul_ps(H1, yUD)), H2), alpha);
					__m128 range_out_local = _mm_div_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(H3, xUD), _mm_mul_ps(H4, yUD)), H5), alpha);

					//Set missing data to missing data value (0 for x, and missingDataValue for range).
					x_out_local = _mm_and_ps(mask[k], x_out_local);
					range_out_local = _mm_add_ps(_mm_andnot_ps(mask[k], nan), _mm_and_ps(mask[k], range_out_local));

					_mm_storeu_ps(&outputData.rangeX.get()[i * 8 + j * width + 4 * k], x_out_local);
					_mm_storeu_ps(&outputData.rangeR.get()[i * 8 + j * width + 4 * k], range_out_local);

				}
			}
		}
	}
	else 
#endif
		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width; i++) {
				const unsigned short data = wordData[i + j * width];
				float* xOut = &outputData.rangeX.get()[i + j * width];
				float* rOut = &outputData.rangeR.get()[i + j * width];

				if (data != 0) {
					calibratePointFast(i, scaleX, xOffset, scaleZ, zOffset, data, xOut, rOut);
				}
				else {
					*xOut = _mMissingDataValue;
					*rOut = _mMissingDataValue;
				}
			}
		}
	
}

float ranger3calibration::CalibrationFilter::getMissingDataValue()
{
	return _mMissingDataValue;
}

void ranger3calibration::CalibrationFilter::setMissingDataValue(float newVal)
{
	_mMissingDataValue = newVal;
}

CalibrationData ranger3calibration::CalibrationFilter::generateOutputContainer()
{
	if (sensorTraits.width == 0 || sensorTraits.height == 0) {
		throw std::runtime_error("Calibration filter needs to be initialized with a height and width larger than zero before running generateOutputContainer");
	}

	return CalibrationData(sensorTraits.width, sensorTraits.height);
}

void CalibrationFilter::calculateWorldTraits() {
	if (calibrationParameters.version == 1) {
		calculateWorldTraitsV1();
	}
	else if (calibrationParameters.version == 2) {
		calculateWorldTraitsV2();
	}
	else {
		assert(false && "Unknown Calibration version");
	}
}

//These are the same values that are written in GenIRanger in the WriteSensorRangeTraits
void CalibrationFilter::calculateWorldTraitsV1() {

	float x0 = (float)0;
	float x1 = (float)sensorTraits.width - 1.f;
	float x2 = (float)0;
	float x3 = (float)sensorTraits.width - 1.f;

	float  z0 = (float)1;
	float  z1 = (float)1;

#ifdef __linux__
	float  z2 = (float)sensorTraits.aoiHeight * 1.f / std::abs(sensorTraits.scaleZ) - 1.f;
	float  z3 = (float)sensorTraits.aoiHeight * 1.f / std::abs(sensorTraits.scaleZ) - 1.f;

#else
	float  z2 = (float)sensorTraits.aoiHeight * 1.f / std::fabsf(sensorTraits.scaleZ) - 1.f;
	float  z3 = (float)sensorTraits.aoiHeight * 1.f / std::fabsf(sensorTraits.scaleZ) - 1.f;

#endif // __linux__

	float sensorZ[4] = { z0,z1,z2,z3 };
	float sensorX[4] = { x0,x1,x2,x3 };


	unsigned short sensorMinZ = (unsigned short)std::floor(*std::min_element(sensorZ, sensorZ + 4));
	unsigned short sensorMaxZ = (unsigned short)std::floor(*std::max_element(sensorZ, sensorZ + 4));
	unsigned short sensorMinX = (unsigned short)std::floor(*std::min_element(sensorX, sensorX + 4));
	unsigned short sensorMaxX = (unsigned short)std::floor(*std::max_element(sensorX, sensorX + 4));

	// Initialize with some valid value. 
	float outX, outZ;
	calibratePoint(sensorMinX, sensorMinZ, outX, outZ);
	float minwr, maxwr, minwx, maxwx;
	minwr = outZ;
	maxwr = outZ;
	minwx = outX;
	maxwx = outX;

	unsigned short z;
	unsigned short x;
	//Top border
	z = sensorMinZ;
	for (x = sensorMinX; x <= sensorMaxX; x++) {
		calibratePoint(x, z, outX, outZ);
		if (outZ != 0.0f)
		{
			minwx = std::min(minwx, outX);
			maxwx = std::max(maxwx, outX);
			minwr = std::min(minwr, outZ);
			maxwr = std::max(maxwr, outZ);
		}
	}
	//Bottom border
	z = sensorMaxZ;
	for (x = sensorMinX; x <= sensorMaxX; x++) {
		calibratePoint(x, z, outX, outZ);
		if (outZ != 0.0f)
		{
			minwx = std::min(minwx, outX);
			maxwx = std::max(maxwx, outX);
			minwr = std::min(minwr, outZ);
			maxwr = std::max(maxwr, outZ);
		}
	}

	//Left border
	x = sensorMinX;
	for (z = sensorMinZ; z <= sensorMaxZ; z++) {
		calibratePoint(x, z, outX, outZ);
		if (outZ != 0.0f)
		{
			minwx = std::min(minwx, outX);
			maxwx = std::max(maxwx, outX);
			minwr = std::min(minwr, outZ);
			maxwr = std::max(maxwr, outZ);
		}
	}
	//Right border
	x = sensorMaxX;
	for (z = sensorMinZ; z <= sensorMaxZ; z++) {
		calibratePoint(x, z, outX, outZ);
		if (outZ != 0.0f)
		{
			minwx = std::min(minwx, outX);
			maxwx = std::max(maxwx, outX);
			minwr = std::min(minwr, outZ);
			maxwr = std::max(maxwr, outZ);
		}
	}


	worldTraits.lowerBoundX = minwx;
	worldTraits.upperBoundX = maxwx;
	worldTraits.lowerBoundR = minwr;
	worldTraits.upperBoundR = maxwr;
}


void CalibrationFilter::calculateWorldTraitsV2() {

	float x0 = (float)0;
	float x1 = (float)sensorTraits.width - 1.f;
	float x2 = (float)0;
	float x3 = (float)sensorTraits.width - 1.f;

	float  z0 = (float)1;
	float  z1 = (float)1;
	float  z2 = (float)sensorTraits.aoiHeight * 16 - 1.f;
	float  z3 = (float)sensorTraits.aoiHeight * 16 - 1.f;

	float sensorZ[4] = { z0,z1,z2,z3 };
	float sensorX[4] = { x0,x1,x2,x3 };

	float scaleX = sensorTraits.scaleX;
	float scaleZ = sensorTraits.scaleZ;


	unsigned short sensorMinZ = (unsigned short)std::floor(*std::min_element(sensorZ, sensorZ + 4));
	unsigned short sensorMaxZ = (unsigned short)std::floor(*std::max_element(sensorZ, sensorZ + 4));
	unsigned short sensorMinX = (unsigned short)std::floor(*std::min_element(sensorX, sensorX + 4));
	unsigned short sensorMaxX = (unsigned short)std::floor(*std::max_element(sensorX, sensorX + 4));

	//Setup v2 layout of data.
	double lens[2 + 4];
#ifdef __linux__
	std::copy(calibrationParameters.opticalCenter + 0, calibrationParameters.opticalCenter + 2, lens);
	std::copy(calibrationParameters.lensDistortion + 0, calibrationParameters.lensDistortion + 4, lens + 2);

#else
	std::copy(calibrationParameters.opticalCenter + 0, calibrationParameters.opticalCenter + 2, stdext::checked_array_iterator<double*>(lens + 0, 6));
	std::copy(calibrationParameters.lensDistortion + 0, calibrationParameters.lensDistortion + 4, stdext::checked_array_iterator<double*>(lens + 2, 4));

#endif // __linux__

	CalibrationParameters cpr = calibrationParameters;

	float xOffset = sensorTraits.xOffset;
	float zOffset = sensorTraits.zOffset;

	// Initialize with some valid value. 
	float outX, outZ;
	Point2D point = sensorToWorld(
		Point2D(
			sensorMinX*scaleX + xOffset,
			sensorMinZ*scaleZ + zOffset),
		lens,
		cpr.scheimpflugRoll,
		cpr.scheimpflugTilt,
		cpr.H
	);
	outX = point.x;
	outZ = point.y;
	float minwr, maxwr, minwx, maxwx;
	minwr = outZ;
	maxwr = outZ;
	minwx = outX;
	maxwx = outX;

	unsigned short z;
	unsigned short x;
	//Top border
	z = sensorMinZ;
	for (x = sensorMinX; x <= sensorMaxX; x++) {
		Point2D point = sensorToWorld(
			Point2D(
				x*scaleX + xOffset,
				z*scaleZ + zOffset),
			lens,
			cpr.scheimpflugRoll,
			cpr.scheimpflugTilt,
			cpr.H
		);
		outX = point.x;
		outZ = point.y;
		if (outZ != 0.0f)
		{
			minwx = std::min(minwx, outX);
			maxwx = std::max(maxwx, outX);
			minwr = std::min(minwr, outZ);
			maxwr = std::max(maxwr, outZ);
		}
	}
	//Bottom border
	z = sensorMaxZ;
	for (x = sensorMinX; x <= sensorMaxX; x++) {
		Point2D point = sensorToWorld(
			Point2D(
				x*scaleX + xOffset,
				z*scaleZ + zOffset),
			lens,
			cpr.scheimpflugRoll,
			cpr.scheimpflugTilt,
			cpr.H
		);
		outX = point.x;
		outZ = point.y;
		if (outZ != 0.0f)
		{
			minwx = std::min(minwx, outX);
			maxwx = std::max(maxwx, outX);
			minwr = std::min(minwr, outZ);
			maxwr = std::max(maxwr, outZ);
		}
	}

	//Left border
	x = sensorMinX;
	for (z = sensorMinZ; z <= sensorMaxZ; z++) {
		Point2D point = sensorToWorld(
			Point2D(
				x*scaleX + xOffset,
				z*scaleZ + zOffset),
			lens,
			cpr.scheimpflugRoll,
			cpr.scheimpflugTilt,
			cpr.H
		);
		outX = point.x;
		outZ = point.y;
		if (outZ != 0.0f)
		{
			minwx = std::min(minwx, outX);
			maxwx = std::max(maxwx, outX);
			minwr = std::min(minwr, outZ);
			maxwr = std::max(maxwr, outZ);
		}
	}
	//Right border
	x = sensorMaxX;
	for (z = sensorMinZ; z <= sensorMaxZ; z++) {
		Point2D point = sensorToWorld(
			Point2D(
				x*scaleX + xOffset,
				z*scaleZ + zOffset),
			lens,
			cpr.scheimpflugRoll,
			cpr.scheimpflugTilt,
			cpr.H
		);
		outX = point.x;
		outZ = point.y;
		if (outZ != 0.0f)
		{
			minwx = std::min(minwx, outX);
			maxwx = std::max(maxwx, outX);
			minwr = std::min(minwr, outZ);
			maxwr = std::max(maxwr, outZ);
		}
	}

	worldTraits.lowerBoundX = minwx;
	worldTraits.upperBoundX = maxwx;
	worldTraits.lowerBoundR = minwr;
	worldTraits.upperBoundR = maxwr;
}


inline void ranger3calibration::CalibrationFilter::calibratePointFast(
	const int & col,
	const float & scaleX,
	const float & xOffset,
	const float & scaleZ,
	const float & zOffset,
	const unsigned short & data,
	float * xOut,
	float * rOut)
{

	float x = (float)col*scaleX + xOffset;
	float y = (float)data*scaleZ + zOffset;

	double* pOC = calibrationParameters.opticalCenter;
	double* pLensDist = calibrationParameters.lensDistortion;
	float x3, y3, x2, y2, r2, xUD, yUD;

	x3 = x - pOC[0];
	y3 = y - pOC[1];
	x2 = x3 * x3;
	y2 = y3 * y3;
	r2 = x2 + y2;

	xUD = (float)((x + x3 * (pLensDist[0] * r2 + pLensDist[1] * r2*r2) + 2 * pLensDist[2] * x3*y3 + pLensDist[3] * (r2 + 2 * x2)));

	yUD = (float)((y + y3 * (pLensDist[0] * r2 + pLensDist[1] * r2*r2) + 2 * pLensDist[3] * x3*y3 + pLensDist[2] * (r2 + 2 * y2)));

	float alpha = calibrationParameters.H[6] * xUD + calibrationParameters.H[7] * yUD + calibrationParameters.H[8];
	float xRes = (calibrationParameters.H[0] * xUD + calibrationParameters.H[1] * yUD + calibrationParameters.H[2]) / alpha;
	float rRes = (calibrationParameters.H[3] * xUD + calibrationParameters.H[4] * yUD + calibrationParameters.H[5]) / alpha;
	*xOut = xRes;
	*rOut = rRes;
}

void CalibrationFilter::calibratePoint(const unsigned short& col, const unsigned short & range, float & xOut, float & rangeOut)
{
	const int width = sensorTraits.width;
	const int height = sensorTraits.height;
	const float scaleX = sensorTraits.scaleX;
	const float scaleZ = sensorTraits.scaleZ;
	const float xOffset = sensorTraits.xOffset;
	const float zOffset = sensorTraits.zOffset;

	if (range != 0) {
		calibratePointFast(col, scaleX, xOffset, scaleZ, zOffset, range, &xOut, &rangeOut);
	}
	else {
		xOut = 0;
		rangeOut = 0;
	}
}
