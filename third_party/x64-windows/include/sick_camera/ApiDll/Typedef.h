/**	@file	Typdef.h
*
*	@brief	typdef and #define set for STL \ GenICam \ SICK GenIRanger
*
*	@attention
*	This is the Ranger3 GenIcam API wrapper.
*
*
*	@copyright	Copyright 2016-2023 SICK AG. All rights reserved.
*	@author		Vision Lab, SICK GCN
*
*/


#pragma once
#define GENICAM_NO_AUTO_IMPLIB 

/**************************************************
* Macro - DLL
**************************************************/
#ifdef __linux__
#   define EXPORT_TO_DLL
#include <zconf.h>
# define __sleep1MS(x) usleep(x*1000)

#else
#ifdef DLL_EXPORTS
#   define EXPORT_TO_DLL   __declspec(dllexport)
#else
#   define EXPORT_TO_DLL   __declspec(dllimport)
#endif
# define __sleep1MS(x) Sleep(x)

#endif

/**************************************************
* Macro - Debug and test only
**************************************************/


//#define COMPILE_DLL_FOR_CLR // CLR is not allow #include <thread> 
#ifndef COMPILE_DLL_FOR_CLR
#define CALLBACK_NEW          // Only declare in C++ project setting. In C# this is not defined. C++ callback needs #include <thread> 
#define CALLBACK_THREAD_POOL  // test 
#endif

//#define UNIT_TEST
//#define DEBUG_CODE

/**************************************************
* HEADs - CalibrationWrapper
**************************************************/

// Disable cal in PC as default since 3.4.0.0
#define DISABLE_CAL_IN_PC // to disbale sheet-of-light.dll.  // you need to run install_X64_Windows_developer.bat
#ifndef DISABLE_CAL_IN_PC
#include <CalibrationWrapper.h>
#endif

#ifndef MISSING_DATA
#define MISSING_DATA -1.0e6f
#endif


/**************************************************
* HEADs - GenIRanger
**************************************************/

#ifdef __linux__
#include "../../Common/public/Ranger3LineMetadata.h"
#include "ChunkAdapter.h"
#include "Consumer.h"
#include "GenIRanger.h"
#include "SampleUtils.h"
#include "StreamData.h"
#endif

#ifdef _WIN32
#include "public/GenIRanger.h"
#include "public/ChunkAdapter.h"
#include "public/ConnectedDevice.h"
#include "public/DeviceDiscovery.h"
#include "public/FrameGrabber.h"
#include "public/StreamData.h"
#include "public/frame/Frame.h"
#include "public/NodeMap.h"
#include "public/Ranger3LineMetadata.h"

#include "public/gentlcpp/CApi.h"
#include "public/gentlcpp/Producer.h"
#include "public/gentlcpp/BufferPart.h"
#include "public/gentlcpp/Device.h"
#include "public/gentlcpp/Port.h"
#include "public/FileOperation.h"
#include "public/UserSet.h"
#include <GenApi/Filestream.h>
#endif


/**************************************************
* HEADs - STL
**************************************************/

#include <map>
#include <set>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <algorithm>

/**************************************************
* Short Name - STL
**************************************************/

template <class T> using SPtr = std::shared_ptr<T>;
template <class T> using UPtr = std::unique_ptr<T>;
template <class T> using Vec  = std::vector<T>;
template <class T> using Set  = std::set<T>;
using		std::cout;
using		std::endl;
using 		Str		= std::string;
using 		cStr	= const Str;


/**************************************************
* Short Name - GenIRanger
**************************************************/

#ifdef __linux__

#define ShowDeviceList(x)           common::showDeviceList(x)
#define StringFromIp4(x)            common::stringFromIp4(x)
#define StringFromMAC(x)            common::stringFromMac(x)
#define Ip4FromString(x)            common::ip4FromString(x)
#define ExportParamCSV(x,y)         GenIRanger::exportDeviceParameters(x,y)
#define ImportParamCSV(x,y)         GenIRanger::importDeviceParameters(x,y)
#define Convert12pTo16(a1,a2,a3,a4) GenIRanger::convert12pTo16(a1,a2,a3,a4)
#define IVP_BUFFER_PART_CUSTOM_INFO_REGION_ID GenIRanger::BUFFER_PART_CUSTOM_INFO_CMD_LIST::BUFFER_PART_CUSTOM_INFO_REGION_ID
#define IVP_BUFFER_PART_CUSTOM_INFO_DATA_PURPOSE_ID GenIRanger::BUFFER_PART_CUSTOM_INFO_CMD_LIST::BUFFER_PART_CUSTOM_INFO_DATA_PURPOSE_ID

typedef common::Consumer        SiConsumer;
typedef common::GenTLPort       SiGenTLPort;
typedef common::ChunkAdapter    SiChunkAdapter;
typedef common::InterfaceList   SiInterfaceList;
typedef common::InterfaceId     SiInterfaceId;

typedef common::DeviceList      SiDeviceList;
typedef common::InterfaceList   SiInterfaceList;
typedef common::InterfaceList   SiInterfaceList;

typedef GenApi::CNodeMapRef			GNodeMap;
typedef GenTL::BUFFER_HANDLE		GBufferH;
typedef GenTL::DEV_HANDLE			GDeviceH;
typedef GenTL::TL_HANDLE			GTL_H;
typedef GenTLApi *					GTl;
typedef GenIRanger::RangeAxis		RA;
#define RA_STANDARD RA::STANDARD
#define RA_REVERSED RA::REVERSED

typedef const std::string			cStr;
typedef std::string					Str;
typedef SPtr<SiConsumer>			ConsumerPtr;
typedef SPtr<GenTLApi>				GenTLPtr;
typedef Vec<GBufferH>				VecBufferHandle;
typedef Vec<Ranger3LineMetadata>	ChunkData;
typedef UPtr<SiChunkAdapter>		ChunkPtr;

#else
#define StringFromIp4(x)			geniranger::Ip4Address::to_String_VLB(x)
#define Ip4FromString(x)			geniranger::Ip4Address::to_uint32_t_VLB(x)
#define StringFromMAC(x)			geniranger::MacAddress::to_String_VLB(x)
#define MakeIp4						geniranger::Ip4Address
#define MakeMac						geniranger::MacAddress
#define ExportParamCSV(x,y)         geniranger::exportDeviceParameters(x,y)
#define ImportParamCSV(x,y)         geniranger::importDeviceParameters(x,y)
#define Convert12pTo16(a1,a2,a3,a4) geniranger::convert12pTo16(a1,a2,a3,a4)
#define IVP_BUFFER_PART_CUSTOM_INFO_REGION_ID geniranger::BUFFER_PART_CUSTOM_INFO_CMD_LIST::BUFFER_PART_CUSTOM_INFO_REGION_ID
#define IVP_BUFFER_PART_CUSTOM_INFO_DATA_PURPOSE_ID geniranger::BUFFER_PART_CUSTOM_INFO_CMD_LIST::BUFFER_PART_CUSTOM_INFO_DATA_PURPOSE_ID

using RA		= geniranger::RangeAxis;
#define RA_STANDARD RA::STANDARD
#define RA_REVERSED RA::REVERSE

using NodeMap	= geniranger::NodeMap;
using UserSetId = geniranger::UserSetId;
using UserSet	= geniranger::UserSet;

using GNodeMap = GenApi::CNodeMapRef;
using GBufferH = GenTL::BUFFER_HANDLE;
using GDeviceH = GenTL::DEV_HANDLE;

using SiConsumer	= gentlcpp::Producer;
using GenTLApi		= const gentlcpp::CApi;
using SiGenTLPort	= gentlcpp::Port;

using ConsumerPtr		= SPtr<SiConsumer>;
using DevicePtr			= SPtr<geniranger::ConnectedDevice>;
using FrameGrabberPtr	= SPtr<geniranger::FrameGrabber>;
using VecBufferHandle	= Vec<GenTL::BUFFER_HANDLE>;
using ChunkData			= Vec<Ranger3LineMetadata>;

using SiDeviceList = std::vector<std::pair<std::string, std::string>>;
#endif

/**************************************************
/* Buffer
/**************************************************/

#define PFNC_Mono8			0x01080001			// Monochrome 8-bit
#define PFNC_Mono12p		0x010C0047			// Monochrome 12-bit packed
#define PFNC_Mono16			0x01100007			// Monochrome 16-bit
#define PFNC_Coord3D_C12p	0x010C00DA			// 3D coordinate C 12-bit packed
#define PFNC_Coord3D_C16	0x011000B8			// 3D coordinate C 16-bit
#define PFNC_Mono4p			0x01040039			// Monochrome 4-bit packed

#define CHUNK_DATA_SIZE 20
#define CHUNK_DATA_WIDTH 5

#define RANGE_A_ID 1715 // [RangeA] Alex: Use this random number to represent "RangeA" component

/**************************************************
* CAM_STATUS
**************************************************/

namespace SickCam {

/** @brief CAM_STATUS 相机状态 */
enum class CAM_STATUS
{
	All_OK = 1,
	CAM_IS_CONNECTED,
	CAM_IS_STARTED,
	CAM_IS_STOPPED,
	CAM_IS_DISCONNECTED,
	ERROR_OPEN_CAM,
	ERROR_SCAN_EMPTY_DEVICE,
	ERROR_CTI_NOT_FOUND,
	ERROR_OPEN_CONSUMER,
	ERROR_OPEN_TL_HANDLE,
	ERROR_EMPTY_IF_ID,
	ERROR_EMPTY_IF_HANDLE,
	ERROR_CSV_PATH,
	ERROR_CALIBRATION_PATH,
	ERROR_EMPTY_PATH,
	ERROR_EMPTY_IMG,
	ERROR_SET_CAM,
	ERROR_CLO_CAM,				
	ERROR_GET_IMG,				
	ERROR_GET_SEN_IMG,			
	ERROR_RST_CAM,				
	ERROR_UPDATE_PARA,				
	ERROR_STOP_ACQUISITION,
	ERROR_START_ACQUISITION,
	ERROR_PARAMETER_FORMAT_DISMATCH,
	ERROR_PARAMETER_VALUE_DISMATCH,
	ERROR_PARAMETER_READ_ONLY,
	ERROR_PARAMETER_INVALID,
	ERROR_PARAMETER_VALUE_INVALID,
	ERROR_PARAMETERS_EMPTY,
	ERROR_SCAN_PARAMETERS,
	ERROR_COMMUNICATION,
	ERROR_SET_IP,
	ERROR_SET_SUBNET,
	ERROR_NULL_PTR_DEV,				
	ERROR_NULL_DEV_HANDLE,
	ERROR_NULL_DS_HANDLE,
	ERROR_OPEN_DATASTREAM,
	ERROR_CAM_NOT_CONNECT,
	ERROR_CAM_NOT_START,
	ERROR_CAM_NOT_STOP,
	ERROR_CAM_NOT_DISCONNECT,
	ERROR_CAM_IS_CONNECTED,
	ERROR_CAM_IS_STARTED,
	ERROR_CAM_IS_STOPPED,
	ERROR_CAM_IS_DISCONNECTED,
	ERROR_CAM_IS_UNREACHABLE,
	ERROR_CAM_IS_OCCUPIED,
    ERROR_CAM_IS_LOST,
	ERROR_IMAGE_NOT_READY,
	ERROR_USER_FILE_NOT_EXIST,
	ERROR_USER_FILE_SEND_FAILED,
	ERROR_USER_FILE_NOT_FOUND_IN_PC,
	ERROR_USER_SET_NOT_FOUND,
	ERROR_OPERATION_NOT_ALLOW,
	WARN_IMAGE_LOST,
	TIME_OUT,
    NO_CHANGE_IN_NETWORK, // not use anymore
	UNKNOWN,
	DEFAULT
};
//Note! If you change the "CAM_STATUS", please also modify
// 1. C++, DeviceConnection.cpp, "EXPORT_TO_DLL std::string CAM_STATUS_str(const CAM_STATUS & e)"
// 2. C#, CommonFunctions.h, "public enum class CAM_STATUS"




/** @brief Device type */
enum class CamType
{
	_RANGER3 = 0,
	_TRISPECTOR = 1
};

}