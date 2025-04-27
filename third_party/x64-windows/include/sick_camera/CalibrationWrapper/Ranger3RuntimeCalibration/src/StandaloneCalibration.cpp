// StandaloneCalibration.cpp : Defines the entry point for the console application.
//
#include "../include/CalibrationFilter.h"
#include "../include/RectificationFilter.h"
#include "../include/CalibrationParametersFactory.h"
#include <chrono>
#include <iostream>
#include <fstream>
#include "../include/CalibrationAPI.h"
int main()
{
	std::cin.ignore();

	ranger3calibration::CalibrationParameters calParams;
	{
		ranger3calibration::CalibrationParametersFactory factory;
		try {
			factory.loadFromFile("..\\TestData\\test_calibrationV2.xml", calParams);
		}
		catch (std::exception& e) {
			std::cout << "Error loading calibration file: " << e.what() << std::endl;
			return -1;
		}
	}

	//Expected data when running for instance ranger3's cont. acq. sample. 
	int64_t bufferWidth = 2560;
	int64_t bufferHeight = 3200;
	int64_t aoiHeight = 832;
	float aoiOffsetX = 0;
	float aoiOffsetY = 0;
	float scaleZ = 0.0625;

	//Setup traits
	ranger3calibration::SensorTraits traits;
	traits.width = bufferWidth;
	traits.height = bufferHeight;
	traits.aoiHeight = aoiHeight;

	//Initialize the calibrationFilter
	ranger3calibration::CalibrationFilter calFilter{ calParams, traits };
	//std::unique_ptr<ranger3calibration::CalibrationData>
	 //       calibrationData(&calFilter.generateOutputContainer());

    auto calibrationData =
            std::make_unique<ranger3calibration::CalibrationData>(calFilter.generateOutputContainer());


	int i = 0;


	////Get container for data 
	//ranger3calibration::CalibrationData calibrationData = calFilter.generateOutputContainer();

	////Initialize rectification filter
	//ranger3calibration::RectificationFilter rectFilter{ calFilter };

	////Get container for data 
	//ranger3calibration::RectificationData rectificationData = rectFilter.generateOutputContainer();

	////Create dummy data, which would come from Ranger3 normally.
	//uint8_t* rawDataRange = new uint8_t[bufferWidth*bufferHeight * 2]; memset(rawDataRange, 128, bufferWidth*bufferHeight * 2 * sizeof(uint8_t)); //Dummy data
	//uint8_t* rawDataReflectance = new uint8_t[bufferWidth*bufferHeight]; memset(rawDataReflectance, 128, bufferWidth*bufferHeight * sizeof(uint8_t)); //Dummy data


	//std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	////Apply Calibration
	//calFilter.applyCalibration(rawDataRange, calibrationData, false);
	//std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

	//auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
	//std::cout << duration << std::endl;

	//t1 = std::chrono::high_resolution_clock::now();
	////Apply Calibration
	//calFilter.applyCalibration(rawDataRange, calibrationData, true);
	//t2 = std::chrono::high_resolution_clock::now();

	//duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
	//std::cout << duration << std::endl;


	////Apply Rectification
	//try {
	//	rectFilter.applyRectification(rawDataReflectance, calibrationData, rectificationData, ranger3calibration::RectificationMethod::Mean);
	//}
	//catch (std::runtime_error e) {
	//	std::cout << e.what() << std::endl;
	//	std::cin.ignore();
	//	return -1;
	//	//Error in rectFilter.
	//}
	//const auto openMode = std::ios::binary | std::ios::trunc | std::ios::out;
	//std::ofstream mDataStream{ "test.dat",openMode };
	//mDataStream.exceptions(std::ios::failbit | std::ios::badbit);

	//for (int i = 0; i < 3200 * 2560; i++) {
	//	const float* p = calibrationData.rangeR.get() + i;
	//	const char* c = reinterpret_cast<const char*>(p);
	//	mDataStream.write(
	//		c,
	//		static_cast<const size_t>(sizeof(float)));

	//}

	std::cin.ignore();
	return 0;
}

