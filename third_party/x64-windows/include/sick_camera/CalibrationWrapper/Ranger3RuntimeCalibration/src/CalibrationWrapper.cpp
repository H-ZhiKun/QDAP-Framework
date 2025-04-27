#include "CalibrationWrapper.h"
#include <fstream>

#include <iostream>
#include <cstring>
//#include <chrono>


namespace SiCaliWrapper
{

// ----------------------------------------------------------------------------
// calibratedData

calibratedData::calibratedData(const size_t & _Width, const size_t & _Height)
{
	Width	= _Width;
	Height	= _Height;
	m_size	= Width * Height;
}

calibratedData::~calibratedData()
{
	for (auto sub : m_Data)
	{
		delete[] (uint8_t*)sub.second;
	}
}

void * calibratedData::getData(PtrN _PN)
{
	if (m_Data.count(_PN) == 1)
	{
		return m_Data[_PN];
	}

	return nullptr;
}

bool calibratedData::insertDataCopy(Ve<PtrN> _names, Ve<void*> _pointers)
{
	if (_names.size() != _pointers.size()) return false;

	for (size_t i = 0; i < _names.size(); ++i)
	{
		if (_pointers[i] == nullptr)
			continue;
		
		void * pData = nullptr;
		if (m_Data.count(_names[i]) == 1)
		{
			pData = m_Data.at(_names[i]);
		}
		delete[] (uint8_t*)pData;

		switch (_names[i])
		{
		case PtrN::RAN_C:
			pData = new float[m_size];
			std::memcpy((uint8_t*)pData, (uint8_t*)_pointers[i], m_size * sizeof(float));
			break;
		case PtrN::REF_C:
			pData = new uint8_t[m_size];
			std::memcpy((uint8_t*)pData, (uint8_t*)_pointers[i], m_size * sizeof(uint8_t));
			break;
		case PtrN::SCA_8:
			pData = new uint8_t[m_size];
			std::memcpy((uint8_t*)pData, (uint8_t*)_pointers[i], m_size * sizeof(uint8_t));
			break;
		case PtrN::SCA_16:
			pData = new uint16_t[m_size];
			std::memcpy((uint8_t*)pData, (uint8_t*)_pointers[i], m_size * sizeof(uint16_t));
			break;
		case PtrN::RAN_X:
			pData = new float[m_size];
			std::memcpy((uint8_t*)pData, (uint8_t*)_pointers[i], m_size * sizeof(float));
			break;
		default:
			break;
		}

		if (m_Data.count(_names[i]) == 0)
		{
			m_Data.insert({ _names[i], pData });
		}
	}

	return true;
}


// ----------------------------------------------------------------------------
// CalibrationWrapper


CalibrationWrapper::CalibrationWrapper() 
{
}

CalibrationWrapper::~CalibrationWrapper()
{
	destroyCalibrationFilter(&m_calibrationFilterHandle);
	destroyModel(&m_modelHandle);
}

bool
CalibrationWrapper::initCalibrationFilter(	const std::string	& filePath_or_jsonString,
											const uint32_t		& bufferWidth,
											const uint32_t		& bufferHeight,
											const uint32_t		& aoiWidth,
											const uint32_t		& aoiHeight,
											const float			& scaleX,
											const float			& originX,
											const float			& scaleZ,
											const float			& originZ,
											const size_t		& scatter_pixelWidth,
											const RectMethod	& method,
											const uint32_t		& rectificationWidth,
											const float			& missingDataValue,
											const bool			  enableSSE,
											const bool			  is_from_string,
											const double		& rectificationSpread)
{
	if (filePath_or_jsonString.empty())
		return false;

	
	if (!is_from_string)
	{
		auto beg = filePath_or_jsonString.find_last_of(".");
		if (beg == std::string::npos)
			return false;

		m_mode = "XML";
		if (filePath_or_jsonString.substr(beg + 1, 3) == "XML" || filePath_or_jsonString.substr(beg + 1, 3) == "xml")
			return _initCalibrationFilter_XML(	filePath_or_jsonString,
												bufferWidth,
												bufferHeight,
												aoiWidth,
												aoiHeight,
												scaleX,
												originX,
												scaleZ,
												originZ,
												method,
												rectificationWidth,
												missingDataValue,
												enableSSE);

	}



	m_mode = "JSON";
	bool isok = _initCalibrationFilter_JSON(	filePath_or_jsonString,
												bufferWidth,
												bufferHeight,
												aoiWidth,
												aoiHeight,
												scaleX,
												originX,
												scaleZ,
												originZ,
												scatter_pixelWidth,
												method,
												rectificationWidth,
												missingDataValue,
												is_from_string);

	if (isok)
	{
		m_splatSettings.splat = rectificationSpread;
	}


	return isok;
}

bool
CalibrationWrapper::calibrate(				std::shared_ptr<calibratedData> pOut,
											uint8_t * range,
											uint8_t * reflectance,
											uint8_t * scatter, 
											bool doRectify)
{
	if (m_mode == "XML")
		return _calibrate_XML(pOut, range, reflectance, doRectify); // no support scatter
	else if (m_mode == "JSON")
		return _calibrate_JSON(pOut, range, reflectance, scatter, doRectify);
	else
		return false;
}

bool
CalibrationWrapper::_initCalibrationFilter_XML(
											const std::string	& filePath,
											const uint32_t		& bufferWidth,
											const uint32_t		& bufferHeight,
											const uint32_t		& aoiWidth,
											const uint32_t		& aoiHeight,
											const float			& scaleX,
											const float			& originX,
											const float			& scaleZ,
											const float			& originZ,
											const RectMethod	& method,
											const uint32_t		& rectificationWidth,
											const float			& missingDataValue,
											const bool			  enableSSE)
{
	// init sensor trait
	m_pSensorTraits = std::make_unique<ranger3calibration::SensorTraits>();
	m_pSensorTraits->height		= bufferHeight;
	m_pSensorTraits->width		= bufferWidth;
	m_pSensorTraits->aoiHeight  = aoiHeight;
	m_pSensorTraits->xOffset	= originX;
	m_pSensorTraits->scaleX		= scaleX;
	m_pSensorTraits->zOffset	= originZ;
	m_pSensorTraits->scaleZ		= scaleZ;

	m_params = std::make_unique<ranger3calibration::CalibrationParameters>();
	std::ifstream inputStream(filePath);
	if (inputStream.good())
		ranger3calibration::loadCalibrationParameters(filePath.c_str(), m_params.get());
	else
		return false;

	m_pCalFilter	= std::make_unique<ranger3calibration::CalibrationFilter>	(*m_params, *(m_pSensorTraits.get()), missingDataValue);
	m_pRectFilter	= std::make_unique<ranger3calibration::RectificationFilter>	(*m_pCalFilter, rectificationWidth);

	//Get container for data
	m_pCalibrationData		= std::make_unique<ranger3calibration::CalibrationData>		(m_pCalFilter->generateOutputContainer());
	m_pRectificationData	= std::make_unique<ranger3calibration::RectificationData>	(m_pRectFilter->generateOutputContainer());

	m_rm = method;
	m_params->missingData	= missingDataValue;
	m_params->SSE			= enableSSE;

	return true;
}


bool
CalibrationWrapper::_initCalibrationFilter_JSON(
											const std::string	& filePath_or_jsonString,
											const uint32_t		& bufferWidth,
											const uint32_t		& bufferHeight,
											const uint32_t		& aoiWidth,
											const uint32_t		& aoiHeight,
											const float			& scaleX,		
											const float			& originX,
											const float			& scaleZ,		
											const float			& originZ,
											const uint32_t		& scatter_pixelWidth,
											const RectMethod	& method, 
											const uint32_t		& rectificationWidth,
											const float			& missingDataValue, 
											const bool			  is_from_string)
{
	// init sensor trait
	m_sensorTraits.aoiHeight	= static_cast<uint16_t>(aoiHeight);
	m_sensorTraits.aoiWidth		= static_cast<uint16_t>(aoiWidth);
	m_sensorTraits.xT.origin	= originX;
	m_sensorTraits.xT.scale		= scaleX;
	m_sensorTraits.zT.origin	= static_cast<double>(originZ);
	m_sensorTraits.zT.scale		= static_cast<double>(scaleZ);

	// image size
	m_imageSize.width			= bufferWidth;
	m_imageSize.lineCount		= bufferHeight;
	m_imageSizeRectify.width	= rectificationWidth;
	m_imageSizeRectify.lineCount= bufferHeight;

	// mode handle
	if (is_from_string)
	{
		if (!createModelFromJson(filePath_or_jsonString.c_str(), &m_modelHandle))
		{
			std::cerr <<m_Ranger3Info <<", " 
				<< "[error] CalibrationWrapper : createModelFromFilePath, jsonString = " << filePath_or_jsonString
				<< ", m_modelHandle = " << m_modelHandle << std::endl;
			return false;
		}
	}
	else
	{

		if (!createModelFromFilePath(filePath_or_jsonString.c_str(), &m_modelHandle))
		{
			std::cerr <<m_Ranger3Info <<", " 
				<< "[error] CalibrationWrapper : createModelFromFilePath, filePath = " << filePath_or_jsonString
				<< ", m_modelHandle = " << m_modelHandle << std::endl;
			return false;
		}


	}



	// calculate Calibration Bounds
	// todo[]: update sheetoflight, but arm not update. -----
#ifdef __aarch64__
	if (!calculateCalibrationBounds(m_modelHandle,
									m_sensorTraits,
									m_imageSize,
									&m_worldRangeTraits,
									&m_splatSettings))
#else
	if (!calculateCalibrationBounds(m_modelHandle,
									m_sensorTraits,
									m_imageSize,
									&m_worldRangeTraits,
									&m_splatSettings, 
									rectificationWidth))
#endif
	{
		std::cerr <<m_Ranger3Info <<", " << "[error] CalibrationWrapper : calculateCalibrationBounds, modelHandle = " << m_modelHandle << std::endl;
		return false;
	}
	


	// user define
	m_missingData			= missingDataValue;
	m_scatter_pixelWidth	= scatter_pixelWidth;
	m_rm					= method;

	//// create Calibration Filter
	//if (!createCalibrationFilter(	m_modelHandle,
	//								m_sensorTraits,
	//								m_imageSize,
	//								PFNC_COORD_3D_C16,
	//								&m_calibrationFilterHandle))
	//{
	//	std::cerr <<m_Ranger3Info <<", " << "[error] CalibrationWrapper : createCalibrationFilter" << std::endl;
	//	return false;
	//}

	//setRunMode(m_calibrationFilterHandle, RunMode::FLOAT32);

	return true;
}


bool CalibrationWrapper::_calibrate_XML(std::shared_ptr<calibratedData> pOut, uint8_t * range, uint8_t * reflectance, bool doRectify)
{
	if (range == nullptr)
		return false;

	m_pCalFilter->applyCalibration(range, *m_pCalibrationData, m_params->SSE);
	pOut->insertDataCopy({ PtrN::RAN_X }, { m_pCalibrationData->rangeX.get() });

	if (doRectify)
	{
		m_pRectFilter->applyRectification(	reflectance, 
											*m_pCalibrationData, 
											*m_pRectificationData, 
											static_cast<ranger3calibration::RectificationMethod>(m_rm));
	
		pOut->insertDataCopy(
			{ PtrN::RAN_C, PtrN::REF_C }, 
			{ m_pRectificationData->range.get(), m_pRectificationData->reflectanceByte.get() }
		);

		pOut->Height	= m_pRectificationData->height;
		pOut->Width		= m_pRectificationData->width;
	}
	else
	{
		pOut->insertDataCopy({ PtrN::RAN_C }, { m_pCalibrationData->rangeR.get()});
		pOut->Height = m_pCalibrationData->height;
		pOut->Width = m_pCalibrationData->width;
	}

	pOut->offsetX = m_pCalFilter->worldTraits.lowerBoundX;
	pOut->scaleX = (m_pCalFilter->worldTraits.upperBoundX - m_pCalFilter->worldTraits.lowerBoundX) / pOut->Width;
	pOut->lower_bound_x = m_pCalFilter->worldTraits.lowerBoundX;
	pOut->upper_bound_x = m_pCalFilter->worldTraits.upperBoundX;
	pOut->lower_bound_r = m_pCalFilter->worldTraits.lowerBoundR;
	pOut->upper_bound_r = m_pCalFilter->worldTraits.upperBoundR;
	pOut->rm = m_rm;

	return true;
}

bool CalibrationWrapper::_calibrate_JSON(std::shared_ptr<calibratedData> pOut, uint8_t * range, uint8_t * reflectance, uint8_t * scatter, bool doRectify)
{
	if (range == nullptr)
		return false;

	// status
	bool hasReflectance = reflectance	!= nullptr;
	bool hasScatter		= scatter		!= nullptr;
	auto nImage2D		(static_cast<unsigned int>(hasReflectance) + static_cast<unsigned int>(hasScatter));

	// pixel format
	PixelFormat* format2D = new PixelFormat[nImage2D];
	if (nImage2D >0)
		format2D[0] = { MONO_8 };
	if (nImage2D >1)
		format2D[1] = { m_scatter_pixelWidth == 16 ? MONO_16 : MONO_8 };

	if (nImage2D > 2)
	{
		delete[] format2D;
		return false;
	}

	//allocate some data for the output containers.
	m_imageSize.lineCount = static_cast<uint16_t>(pOut->Height);
	size_t dataSize = m_imageSize.width * m_imageSize.lineCount;

	std::vector<float>	X			(dataSize); // calibrated data size
	std::vector<float>	Z			(dataSize); // calibrated data size
	if (doRectify) // if do rectify, output image size is defined by rectification width.
		dataSize = m_imageSizeRectify.width * pOut->Height;

	std::vector<float>	rangeOut	(dataSize);
	auto reflectanceOut	= hasReflectance						? std::vector<uint8_t>	(dataSize)  : std::vector<uint8_t>();
	auto scatterOut16	= hasScatter && format2D[1] == MONO_16	? std::vector<uint16_t>	(dataSize)	: std::vector<uint16_t>();
	auto scatterOut8	= hasScatter && format2D[1] == MONO_8	? std::vector<uint8_t>	(dataSize)	: std::vector<uint8_t>();

	// CoRectified Description
	m_coRectifiedDescription = { nImage2D, format2D }; // todo[] : scatter16 supporting will be fix in next version of sheet_of_light 
	m_worldRangeTraits.xRange.steps = m_imageSizeRectify.width;
	m_worldRangeTraits.yRange.steps = pOut->Height; // todo: ddd

	// create Rectification Filter
	if (!createRectificationFilter(	m_splatSettings,
									static_cast<RectificationMode>(m_rm),
									m_worldRangeTraits,
									m_imageSize,
									m_coRectifiedDescription,
									m_missingData,
									&m_rectificationFilterHandle))
	{
		std::cerr <<m_Ranger3Info <<", " << "[error] CalibrationWrapper : createRectificationFilter" << std::endl;
		return false;
	}
	void * pScatterOut = nullptr;
	if (nImage2D > 1)
	{
		if		(format2D[1] == MONO_16)	pScatterOut = scatterOut16.data();
		else if (format2D[1] == MONO_8)		pScatterOut = scatterOut8.data();
		else								pScatterOut = nullptr;
	}

	void* rectifiedImages2D[2] = {
		!hasReflectance ? nullptr : reflectanceOut.data(),
		!hasScatter		? nullptr : (uint8_t*)pScatterOut
	};

	void* inputCoRectified[2]	= { reflectance, scatter };

	if (nImage2D > 2)
	{
		delete[] format2D;
		return false;
	}

	// reflectance data will not change during calibration

	// create Calibration Filter
	if (!createCalibrationFilter(m_modelHandle,
		m_sensorTraits,
		m_imageSize,
		PFNC_COORD_3D_C16,
		&m_calibrationFilterHandle))
	{
		std::cerr << m_Ranger3Info << ", " << "[error] CalibrationWrapper : createCalibrationFilter" << std::endl;
		return false;
	}

	setRunMode(m_calibrationFilterHandle, RunMode::FLOAT32);
	if (!applyCalibration(m_calibrationFilterHandle, range, X.data(), Z.data()))
	{
		std::cerr << m_Ranger3Info << ", " << "[error] CalibrationWrapper : applyCalibration failed!" << std::endl;
		return false;
	}

	if (doRectify)
	{
		rectify(m_rectificationFilterHandle, X.data(), Z.data(), inputCoRectified, rangeOut.data(), rectifiedImages2D);
		pOut->insertDataCopy(
			{	PtrN::RAN_C, PtrN::REF_C, PtrN::SCA_8, PtrN::SCA_16 },
			{
				rangeOut		.empty() ? nullptr : rangeOut.data(), // rectified Z
				reflectanceOut	.empty() ? nullptr : reflectanceOut.data(),
				scatterOut8		.empty() ? nullptr : scatterOut8.data(),
				scatterOut16	.empty() ? nullptr : scatterOut16.data(),
			}
		);
		pOut->Width = m_imageSizeRectify.width;
	}
	else
	{
		pOut->insertDataCopy(
			{	PtrN::RAN_C, PtrN::REF_C, PtrN::SCA_8, PtrN::SCA_16, PtrN::RAN_X },
			{
				Z				.empty() ? nullptr : Z.data(), // calibrated Z
				reflectanceOut	.empty() ? nullptr : reflectanceOut.data(),
				scatterOut8		.empty() ? nullptr : scatterOut8.data(),
				scatterOut16	.empty() ? nullptr : scatterOut16.data(),
				X				.empty() ? nullptr : X.data()  // calibrated X, only avaliable when not rectify
			}
		);
		pOut->Width = m_imageSize.width;
	}

	//pOut->Height		= m_imageSize.lineCount;
	pOut->lower_bound_x = m_worldRangeTraits.xRange.minimum;
	pOut->upper_bound_x = m_worldRangeTraits.xRange.maximum;
	pOut->lower_bound_r = m_worldRangeTraits.zRange.minimum;
	pOut->upper_bound_r = m_worldRangeTraits.zRange.maximum;
	pOut->offsetX		= pOut->lower_bound_x;
	pOut->scaleX		= (pOut->upper_bound_x - pOut->lower_bound_x) / pOut->Width;
	pOut->rm = m_rm;

	delete[] format2D;
	destroyRectificationFilter(&m_rectificationFilterHandle);
	return true;
}


}
