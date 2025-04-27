#pragma once

#ifdef __linux__
#   define SICK_CALIB_EXPORT_TO_DLL

#else
#if defined(DLL_EXPORTS) // inside DLL
#   define SICK_CALIB_EXPORT_TO_DLL   __declspec(dllexport)
#else // outside DLL
#   define SICK_CALIB_EXPORT_TO_DLL   __declspec(dllimport)
#endif

#endif

#include "Ranger3RuntimeCalibration/include/CalibrationFilter.h"
#include "Ranger3RuntimeCalibration/include/RectificationFilter.h"


// todo[]: update sheetoflight, but arm not update. -----
#if defined(__aarch64__) || defined(__arm__)
#include "RuntimeCalibration/include/arm/SickSheetOfLight.h"
using namespace geniranger::sheetoflight; // 0.2.0.555
#else
#include "RuntimeCalibration/include/SickSheetOfLight.h"
using namespace genistream::sheetoflight; // 0.4.0.909
#endif

#include <vector>
#include <map>
#include <memory>

template<class T> using Ve = std::vector<T>;

constexpr auto MISSING_DATA = -1.0e6f;
constexpr auto RECTIFICATION_WIDTH = 3200;


#ifdef __cplusplus
namespace SiCaliWrapper{
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////


/**	Names of calibrated data pointers. */
typedef enum PointerNames
{
	RAN_C,		//! < float		* | Range			| [Calibrated]	| ComponentSelector_Range_______RegionSelector_Scan3dExtraction1
	REF_C,		//! < uint8_t	* | Reflectance		| [Calibrated]	| ComponentSelector_Reflectance_RegionSelector_Scan3dExtraction1
	SCA_8,		//! < uint8_t	* |	Scatter 8 bit	| [Calibrated]	| ComponentSelector_Scatter_____RegionSelector_Scan3dExtraction1
	SCA_16,		//! < uint16_t	* |	Scatter 16 bit	| [Calibrated]	| ComponentSelector_Scatter_____RegionSelector_Scan3dExtraction1
	RAN_X		//! < float		* | Range X without rectification   | [Calibrated]	| Calibration only, without rectification
} PtrN;

enum RectMethod
{
	Mean,
	Top,
	Bottom,
	DARKEST,
	BRIGHTEST,
	NOT_DO,
};


/** @brief calibratedData													 
*																			 
*	Calibrated data Class													 
*																		
*/
class SICK_CALIB_EXPORT_TO_DLL calibratedData
{
public:
	calibratedData	(const size_t & _Width, const size_t & _Height);

	~calibratedData	();

	void *		getData			(PtrN _PN);
	bool		insertDataCopy	(Ve<PtrN> _names, Ve<void*> _pointers); // deep copy

	size_t		Width;
	size_t		Height;

	double		scaleX;
	double		offsetX;

	double		lower_bound_x;
	double		upper_bound_x;
	double		lower_bound_r;
	double		upper_bound_r;

	RectMethod	rm;
	bool		doRectify;

private:
	std::map<PtrN, void *>		m_Data;
	size_t						m_size;

};


/** Interface for calling calibration functions	*/
class CalibrationWrapper final
{
public:
    SICK_CALIB_EXPORT_TO_DLL CalibrationWrapper();
    SICK_CALIB_EXPORT_TO_DLL ~CalibrationWrapper();

	/** 
	/if Chinese
	* @brief 初始化标定。\n\n
	* \n
	* @param[in] filePath				请输入标定文件路径：xml 或者 json。
	* @param[in] bufferWidth			Range 图像的宽度		见参数：${Width_RegionSelector_Region1}
	* @param[in] bufferHeight			Range 图像的高度		见参数：${Height_RegionSelector_Scan3dExtraction1}
	* @param[in] aoiWidth				见参数：${Width_RegionSelector_Region1}
	* @param[in] aoiHeight				见参数：${Height_RegionSelector_Region1}
	* @param[in] scaleX					始终为 1.0
	* @param[in] originX				见参数：${OffsetX_RegionSelector_Region1}
	* @param[in] scaleZ					scaleZ = (${RangeAxis_Scan3dExtractionSelector_Scan3dExtraction1} == Standard ) ? 0.0625 : -0.0625;
	* @param[in] originZ				见参数：${OffsetY_RegionSelector_Region1}
	* @param[in] scatter_pixelWidth		若有 scatter 图像，则根据实际情况设置。见参数：${PixelFormat_ComponentSelector_Scatter_RegionSelector_Scan3dExtraction1}
	* @param[in] method					Rectify 的方法。见： RectMethod
	* @param[in] rectificationWidth		Rectify 时，X 方向的采样列数。假如原始图像是 2560 列，该值设置为 2000，则输出图像为 2000 列。默认大小为是${Width_RegionSelector_Region1}。注意，该值不能大于 X 方向最大列数。
	* @param[in] missingDataValue		填充无效区域的值，默认是 -1.0e6
	* @param[in] enableSSE				是否使用 SSE 加速。使能前请确定 CPU 支持 SSE 指令集。
	* @param[in] is_from_string			请设置为 false。【注意】 设置为 true 是特殊模式，【请不要轻易使用】。
	* @param[in] rectificationSpread	Rectify 时，X 方向的采样宽度。推荐1.2，此参数请参考 EZR Setup 中的推荐值。
	* \n
	/else
	* @brief Init calibration filter from file-path. \n\n
	* @param[in] filePath				The path of calibration file.
	* @param[in] bufferWidth			The width of range image,		${Width_RegionSelector_Region1}
	* @param[in] bufferHeight			The height of range image,		${Height_RegionSelector_Scan3dExtraction1}
	* @param[in] aoiWidth				${Width_RegionSelector_Region1}
	* @param[in] aoiHeight				${Height_RegionSelector_Region1}
	* @param[in] scaleX					Always be 1.0
	* @param[in] originX				${OffsetX_RegionSelector_Region1}
	* @param[in] scaleZ					scaleZ = (${RangeAxis_Scan3dExtractionSelector_Scan3dExtraction1} == Standard ) ? 0.0625 : -0.0625;
	* @param[in] originZ				${OffsetY_RegionSelector_Region1}
	* @param[in] scatter_pixelWidth		${PixelFormat_ComponentSelector_Scatter_RegionSelector_Scan3dExtraction1}
	* @param[in] method					See RectMethod
	* @param[in] rectificationWidth		<= ${Width_RegionSelector_Region1}
	* @param[in] missingDataValue		The default value is -1.0e6
	* @param[in] enableSSE				The defalut is true. To enabele SSE to accelerate calibration.
	* @param[in] is_from_string			Please always set false.
	* @param[in] rectificationSpread	To enable splat setting, make missing data less.
	* \n
	* \n
	/endif
	*/
    SICK_CALIB_EXPORT_TO_DLL bool initCalibrationFilter(
									const std::string	& filePath,
									const uint32_t		& bufferWidth, 
									const uint32_t		& bufferHeight,
									const uint32_t		& aoiWidth,
									const uint32_t		& aoiHeight,
									const float			& scaleX,
									const float			& originX,
									const float			& scaleZ,
									const float			& originZ,
									const size_t		& scatter_pixelWidth = 16,
									const RectMethod	& method = RectMethod::Top,
									const uint32_t		& rectificationWidth = 2560,
									const float			& missingDataValue = MISSING_DATA,
									const bool			  enableSSE = true,
									const bool			  is_from_string = false,
									const double		& rectificationSpread = 1.2,
									const int			& runMode=-1
		);

#ifndef __aarch64__
	/**
	* @brief X64 only 设置标定函数将使用的线程数。如果不设置，默认为 CPU 的逻辑核数，如此，则一般会达到 100% 的CPU使用率。如果希望降低 CPU 使用率，可以减少线程数，这样也会增加标定处理的使用时间。  \n\n
	* \n
	* @param[in] threadNumber	标定函数将使用的线程数。
	*/
	SICK_CALIB_EXPORT_TO_DLL void set_NumberOfThreads_jsonOnly_X64(const int & threadNumber) { setNumberOfThreads(threadNumber); };
	SICK_CALIB_EXPORT_TO_DLL int  get_NumberOfThreads_jsonOnly_X64() { return getNumberOfThreads(); };
#endif
    SICK_CALIB_EXPORT_TO_DLL bool calibrate		(
									std::shared_ptr<calibratedData> pOut,
									uint8_t * range, 
									uint8_t * reflectance, 
									uint8_t * scatter, 
									bool doRectify);

protected:

	bool _initCalibrationFilter_XML(const std::string	& filePath, 
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
									const bool			  enableSSE);

	bool _initCalibrationFilter_JSON(
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
									const bool			  is_from_string);


	bool _calibrate_XML				(std::shared_ptr<calibratedData> pOut, uint8_t * range, uint8_t * reflectance, bool doRectify = true);
	bool _calibrate_JSON			(std::shared_ptr<calibratedData> pOut, uint8_t * range, uint8_t * reflectance, uint8_t * scatter = nullptr, bool doRectify = true);

private:

	// xml
	std::unique_ptr<ranger3calibration::CalibrationFilter>		m_pCalFilter;
	std::unique_ptr<ranger3calibration::RectificationFilter>	m_pRectFilter;
	std::unique_ptr<ranger3calibration::CalibrationData>		m_pCalibrationData;
	std::unique_ptr<ranger3calibration::RectificationData>		m_pRectificationData;
	std::unique_ptr<ranger3calibration::SensorTraits>			m_pSensorTraits;
	std::unique_ptr<ranger3calibration::CalibrationParameters>	m_params; //! < Calibration Parameters

	//json
	SensorTraits				m_sensorTraits;
	ModelHandle					m_modelHandle;
	ImageSize					m_imageSize;
	ImageSize					m_imageSizeRectify;
	CoRectifiedDescription		m_coRectifiedDescription;
	WorldRangeTraits			m_worldRangeTraits;
	float						m_missingData;

	CalibrationFilterHandle		m_calibrationFilterHandle;
	RectificationFilterHandle	m_rectificationFilterHandle;


	RectMethod					m_rm;
	uint32_t					m_scatter_pixelWidth;
	std::string					m_mode;
	SplatSettings				m_splatSettings;
	int							m_runMode;

public:
	std::string					m_Ranger3Info; // debug 
};	


#ifdef __cplusplus
	};
}
#endif

typedef SiCaliWrapper::PtrN PN;
