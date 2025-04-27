#include "../include/CalibrationAPI.h"
#include "../include/CalibrationParametersFactory.h"
#include "../include/CalibrationFilter.h"
#include "../include/RectificationFilter.h"
#include <string>

using namespace ranger3calibration;

void 
ranger3calibration::loadCalibrationParameters(
								const char * pathToXML,
								CalibrationParameters* outParameters)
{
		CalibrationParametersFactory factory;
		factory.loadFromFile(std::string(pathToXML), *outParameters);
}

void 
ranger3calibration::rectifyAndCalibrate(
							unsigned char* inUnCalibratedRangeBuffer,
							float* outCalibratedAndRectifiedRangeBuffer,
							WorldTraits* worldTraits,
							SensorTraits* sensorTraits,
							CalibrationParameters* parameters,
							RectificationMethod rectificationMethod)
{

	if (inUnCalibratedRangeBuffer == nullptr ||
		outCalibratedAndRectifiedRangeBuffer == nullptr ||
		worldTraits == nullptr ||
		sensorTraits == nullptr ||
		parameters == nullptr)
	{
		return;
	}

	if (sensorTraits->width == 0 ||
		sensorTraits->height == 0) {
		return;
	}

	CalibrationFilter calibrationFilter(*parameters, *sensorTraits, parameters->missingData);
	RectificationFilter rectificationFilter(calibrationFilter, sensorTraits->width);

	auto calOutputContainer = calibrationFilter.generateOutputContainer();
	auto rectOutputContainer = rectificationFilter.generateOutputContainer();

	//Runtime Calibration SSE Version 
	calibrationFilter.applyCalibration(
		inUnCalibratedRangeBuffer,
		calOutputContainer,
		parameters->SSE);

	//Standalone Rectification
	rectificationFilter.applyRectification(
		nullptr,
		calOutputContainer,
		rectOutputContainer,
		rectificationMethod);

	*worldTraits = calibrationFilter.worldTraits;
	rectOutputContainer.copyRange(outCalibratedAndRectifiedRangeBuffer);
}