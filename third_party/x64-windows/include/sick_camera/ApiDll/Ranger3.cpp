#include "Ranger3.h"
#ifdef WIN32
#include <public/Crc32.h>
#endif //  WIN32

namespace SickCam 
{
Ranger3::Ranger3(const SPtr<Ranger3Shared>& RS, cStr & ip_mac, const bool useIP) 
	: m_pR3S				(nullptr)
	, m_device				(nullptr)
	, m_consumer			(nullptr)
	, m_chunkAdapter		(nullptr)
#ifndef DISABLE_CAL_IN_PC
	, m_pCalibrationWrapper	(nullptr)
#endif
	, m_pParts				(nullptr)
	, m_ChunkModeActive		(false)
	, m_isUsingReflectance	(false)
	, m_IsOutputSensor		(false)
	, m_buffer16Size		(0)
	, m_payloadSize			(0)
	, m_DeviceName			("")
	, m_status				(CAM_STATUS::ERROR_NULL_PTR_DEV)
	, m_canStop				(true)
	, m_calibration_inDevice (false)
#ifdef CALLBACK_NEW
	, m_callback_is_on		(false)
	, m_callback_require_stop(false)
#endif
{
    _initDevice(RS, ip_mac, useIP);
}

Ranger3::~Ranger3()
{
	*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::deconstruct]\n";

	if (m_device != nullptr)	{
		m_device->closeDataStream();
		m_device->closeDevice();
	}
}

EXPORT_TO_DLL CAM_STATUS 
Ranger3::isReady()
{
	if (getStatus() == CAM_STATUS::ERROR_NULL_PTR_DEV)	
											return CAM_STATUS::ERROR_NULL_PTR_DEV;

	if (!m_device->isReachable())			return CAM_STATUS::ERROR_CAM_IS_UNREACHABLE;
	if (m_device->isOccupied())				return CAM_STATUS::ERROR_CAM_IS_OCCUPIED;

	return CAM_STATUS::All_OK;
}

EXPORT_TO_DLL CAM_STATUS
Ranger3::connectCamera()
{
	if (getStatus() == CAM_STATUS::CAM_IS_CONNECTED)	return CAM_STATUS::All_OK;
	if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
	if (getStatus() == CAM_STATUS::CAM_IS_STOPPED)		return CAM_STATUS::All_OK;

	// connect
	auto e = _connectCamera();
	
	if (e != CAM_STATUS::All_OK) { m_status = CAM_STATUS::CAM_IS_DISCONNECTED; return e; }
	m_status = CAM_STATUS::CAM_IS_CONNECTED;	
	*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::connectCamera]: Connect OK!\n";
	
	if (m_DeviceModelName.empty())
	{
		m_DeviceModelName = getParameterValue("DeviceModelName");
		m_DeviceVersion = getParameterValue("DeviceVersion");
		m_DeviceFirmwareVersion = getParameterValue("DeviceFirmwareVersion");
		if (m_DeviceFirmwareVersion[m_DeviceFirmwareVersion.length()-1] == '\n')
		{
			m_DeviceFirmwareVersion = m_DeviceFirmwareVersion.substr(0, m_DeviceFirmwareVersion.length()-1);
		}
		m_DeviceSerialNumber = getParameterValue("DeviceSerialNumber");
		*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::connectCamera]: Update device information OK!\n";
		*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::connectCamera]:     DeviceVersion=" << m_DeviceVersion << "\n";
		*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::connectCamera]:     DeviceFirmwareVersion=" << m_DeviceFirmwareVersion << "\n";
	}

	// load csv if has.
	if (!m_Param.m_ParaPath.empty())	{
		e = loadParameterFrCSV(m_Param.m_ParaPath);
		if (e != CAM_STATUS::All_OK) { m_status = CAM_STATUS::CAM_IS_DISCONNECTED; return e; }
		m_status = CAM_STATUS::CAM_IS_CONNECTED;	
		*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::connectCamera]: Load csv file OK!\n";
	}else
		*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::connectCamera]: Skip loading csv file!\n";

	// scan device parameter.
	e = _scanDeviceParameter(m_Param.m_TmpPath);
	
	if (e != CAM_STATUS::All_OK) { m_status = CAM_STATUS::CAM_IS_DISCONNECTED; return e; }
	m_status = CAM_STATUS::CAM_IS_CONNECTED;	
	*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::connectCamera]: Scan device parameter OK!\n";

#ifndef __linux__
	// init userSets.
	m_userSets.clear();
	m_userSets.insert({ geniranger::UserSetId::DEFAULT,		UserSet(m_deviceNode, geniranger::UserSetId::DEFAULT) });
	m_userSets.insert({ geniranger::UserSetId::USER_SET_1,	UserSet(m_deviceNode, geniranger::UserSetId::USER_SET_1) });
	m_userSets.insert({ geniranger::UserSetId::USER_SET_2,	UserSet(m_deviceNode, geniranger::UserSetId::USER_SET_2) });
	m_userSets.insert({ geniranger::UserSetId::USER_SET_3,	UserSet(m_deviceNode, geniranger::UserSetId::USER_SET_3) });
	m_userSets.insert({ geniranger::UserSetId::USER_SET_4,	UserSet(m_deviceNode, geniranger::UserSetId::USER_SET_4) });
	m_userSets.insert({ geniranger::UserSetId::USER_SET_5,	UserSet(m_deviceNode, geniranger::UserSetId::USER_SET_5) });
	*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::connectCamera]: (x64 only) Init userSets OK!\n";
#endif

#ifdef CALLBACK_NEW
	m_heartbeat_is_on = 2;
#endif // CALLBACK_NEW
	
	return e;
}

EXPORT_TO_DLL CAM_STATUS
Ranger3::startCamera()
{
	if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::All_OK;
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;
	if (getStatus() == CAM_STATUS::ERROR_NULL_PTR_DEV)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;

	auto e = _startAcquisition();
	if (e == CAM_STATUS::All_OK) m_status = CAM_STATUS::CAM_IS_STARTED;
	return e; 
}

#ifdef CALLBACK_NEW

EXPORT_TO_DLL CAM_STATUS
Ranger3::connectCamera(CallbackEvent_HeartBeats pCallback, const uint32_t& microSecond, void * any)
{
	try{
		// add heartbeat callback
		auto e = connectCamera();
		if(e == CAM_STATUS::All_OK)
		{
			m_heartbeat_is_on = 1;
			m_on_lost_function = pCallback;
			m_on_lost_inputs = any;
			m_heartbeat_interval = microSecond;
			auto _thread = std::make_shared<std::thread>(&Ranger3::_check_HeartBeats_run, this);
			_thread->detach();
		}
		return e;
	}
	catch (std::exception& e)
	{
		std::cerr << "[" << m_DeviceName << "][Ranger3::connectCamera]: try catch: \n" << e.what() << std::endl;
		*m_log << CustomerLog::time() << e.what() << "\n";
		return CAM_STATUS::ERROR_COMMUNICATION;
	}
	catch (...)
	{
		std::cerr << "[" << m_DeviceName << "][Ranger3::connectCamera]: try catch: unknown \n" << std::endl;
		*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::connectCamera]: try catch: unknown \n";
		return CAM_STATUS::UNKNOWN;
	}
}

EXPORT_TO_DLL CAM_STATUS
Ranger3::connectCamera(CallbackEvent_HeartBeats pCallback, const uint32_t& microSecond, const size_t& numCallbackThreadsInPool, void* any)
{
	try {
		// add heartbeat callback
		auto e = connectCamera();
		if (e == CAM_STATUS::All_OK)
		{
			m_heartbeat_is_on = 1;
			m_on_lost_function = pCallback;
			m_on_lost_inputs = any;
			m_heartbeat_interval = microSecond;
			auto _thread = std::make_shared<std::thread>(&Ranger3::_check_HeartBeats_run, this);
			_thread->detach();

			enableCallbackThreadsInPool(numCallbackThreadsInPool, true);
		}
		return e;
	}
	catch (std::exception& e)
	{
		std::cerr << "[" << m_DeviceName << "][Ranger3::connectCamera]: try catch: \n" << e.what() << std::endl;
		*m_log << CustomerLog::time() << e.what() << "\n";
		return CAM_STATUS::ERROR_COMMUNICATION;
	}
	catch (...)
	{
		std::cerr << "[" << m_DeviceName << "][Ranger3::connectCamera]: try catch: unknown \n" << std::endl;
		*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::connectCamera]: try catch: unknown \n";
		return CAM_STATUS::UNKNOWN;
	}
}

EXPORT_TO_DLL CAM_STATUS
Ranger3::reconnectCamera()
{
	try {
		if (getStatus() == CAM_STATUS::ERROR_CAM_IS_LOST)
		{
			// scan
			auto err = m_pR3S->scanDevice();
			*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::reconnectCamera]: scanDevice(), return = " << CAM_STATUS_str(err) << "\n";

			if (m_pR3S->getConDevListMAC().count(m_on_lost_mac) == 1) // check is_found
			{
				*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::reconnectCamera]: re-found lost device(" << m_on_lost_mac << ") ok!\n";
				err = _initDevice(m_pR3S, m_on_lost_mac, false);
				if (CAM_STATUS::All_OK == err) {
					*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::reconnectCamera]: re-init lost device(" << m_on_lost_mac << ") ok!\n";
					return connectCamera(m_on_lost_function, m_heartbeat_interval, m_on_lost_inputs);
				}
				else {
					*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::reconnectCamera]: re-init lost device(" << m_on_lost_mac << ") failed! return = " << CAM_STATUS_str(err) << "\n";
					return getStatus();
				}
			}
			else {
				*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::reconnectCamera]: re-found lost device(" << m_on_lost_mac << ") failed!\n";
				*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::reconnectCamera]: Mac address of all found devices are: \n";
				for (auto sub : m_pR3S->getConDevListMAC())
					*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::reconnectCamera]:    " << sub.first << "\n";

				*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::reconnectCamera]: What lost is: " << m_on_lost_mac << "\n\n";
				return getStatus();
			}
		}
		else
			return getStatus();
	}
	catch (std::exception& e)
	{
		std::cerr << "[" << m_DeviceName << "][Ranger3::reconnectCamera]: re-init lost device(" << m_on_lost_mac << "), try catch: \n" << e.what() << std::endl;
		*m_log << CustomerLog::time() << e.what() << "\n";
		throw e;
	}
}

EXPORT_TO_DLL CAM_STATUS 
Ranger3::startCamera(CallbackEvent_GetImage pCallback, void * any, const uint32_t& grab_sleep_time_ms)
{
	//auto d = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	auto err = startCamera();
	//auto end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	//std::cout << "@@@@@@@@@@@@@@@   startCamera : " << end - d << " microseconds. \n";

	//d = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	if (err == CAM_STATUS::All_OK)
	{
		m_callBack_function = std::move(pCallback);

		m_callBack_inputs = any;
		m_callback_is_on = true;
		std::unique_lock<std::mutex> lock(m_callback_require_stop_locker);
		m_callback_require_stop = false;
		m_grab_interval = grab_sleep_time_ms;
		auto _thread = std::make_shared<std::thread>(&Ranger3::_callback_run, this);
		_thread->detach();
	}
	//end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	//std::cout << "@@@@@@@@@@@@@@@   startCamera threads : " << end - d << " microseconds. \n";
	return err;
}

#endif // CALLBACK_NEW

EXPORT_TO_DLL CAM_STATUS
Ranger3::stopCamera()
{
	//auto d = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	if (getStatus() == CAM_STATUS::CAM_IS_CONNECTED)	return CAM_STATUS::All_OK;
	if (getStatus() == CAM_STATUS::CAM_IS_STOPPED)		return CAM_STATUS::All_OK;
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;
	if (getStatus() == CAM_STATUS::ERROR_NULL_PTR_DEV)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;

#ifdef CALLBACK_NEW
	if (m_callback_is_on)
	{
		{
			std::unique_lock<std::mutex> lock(m_callback_require_stop_locker);
			m_callback_require_stop = true;
		}
		while (m_callback_is_on)
		    __sleep1MS(5);
	}
#endif
	//auto end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	//std::cout << "=============   stopCamera thread : " << end - d << " microseconds. \n";

	//d = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	auto e = _stopAcquisition();
	if (e == CAM_STATUS::All_OK)
	{
		m_status = CAM_STATUS::CAM_IS_STOPPED;
	}
	//end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	//std::cout << "@@@@@@@@@@@@@@@   stopCamera : " << end - d << " microseconds. \n";
	return e;
}

EXPORT_TO_DLL CAM_STATUS 
Ranger3::stopCamera_WaitCallbackFinish(bool please_do_not_use_it_any_more)
{
	// todo: add funtion of stopCamera_WaitCallbackFinish ----
	return CAM_STATUS::All_OK;
}

EXPORT_TO_DLL CAM_STATUS
Ranger3::disconnectCamera()
{
	if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::All_OK;
	if (getStatus() == CAM_STATUS::CAM_IS_CONNECTED || getStatus() == CAM_STATUS::CAM_IS_STOPPED)
	{
#ifdef CALLBACK_NEW
		if (m_heartbeat_is_on == 1)		
		{
			m_heartbeat_is_on = 0;
			while (m_heartbeat_is_on != 2)
				__sleep1MS(10);
		}
#endif
		auto e = _disconnectCamera();
		if (e == CAM_STATUS::All_OK) m_status = CAM_STATUS::CAM_IS_DISCONNECTED;
		return e;
	}
	return CAM_STATUS::UNKNOWN;
}

EXPORT_TO_DLL CAM_STATUS
Ranger3::getImageData(ImgT & imgTable)
{
	if (getStatus() == CAM_STATUS::CAM_IS_CONNECTED)	return CAM_STATUS::ERROR_CAM_IS_CONNECTED;
	if (getStatus() == CAM_STATUS::CAM_IS_STOPPED)		return CAM_STATUS::ERROR_CAM_IS_STOPPED;
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;


	m_canStop = false;
	//auto d = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	auto res = _getImageData(imgTable);
	//auto end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	//std::cout << "===============   _getImageData : " << end - d << " microseconds. \n";
	m_canStop = true;
	//std::cout << ".";
	return res;
}

EXPORT_TO_DLL CAM_STATUS	
Ranger3::setCalibration_inDevice(const uint32_t& rectificationWidth, const bool range_Uint16, const bool convertRange_ToFloat_ByOpenMP)
{
	// in device calibration enable -----
	if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;

	if (CAM_STATUS::All_OK != setParameter("Scan3dOutputMode_Scan3dExtractionSelector_Scan3dExtraction1", "RectifiedC")) 
	{
		m_calibration_inDevice = false;
		*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" 
			<< "try to set [Scan3dOutputMode_Scan3dExtractionSelector_Scan3dExtraction1] as [RectifiedC] failed! \
				1. Check if in-device calibration file is in device now? 2. Check if firmware is higher than 3.0?\n";
		return CAM_STATUS::ERROR_OPERATION_NOT_ALLOW;
	}

	if (CAM_STATUS::All_OK != setParameter("Scan3dRectificationWidth_Scan3dExtractionSelector_Scan3dExtraction1", std::to_string(rectificationWidth)))
	{
		m_calibration_inDevice = false;
		*m_log << CustomerLog::time()
			<< "[" << m_DeviceName << "]" << "try to set [Scan3dRectificationWidth_Scan3dExtractionSelector_Scan3dExtraction1] as [" << rectificationWidth
			<< "] failed! 1. Value must be 32's multiple(" << rectificationWidth << "/32=" << (rectificationWidth / 32.0)
			<< "); 2. Check if in-device calibration file is in device now? 3. Check if firmware is higher than 3.0?\n";
		return CAM_STATUS::ERROR_OPERATION_NOT_ALLOW;
	}

	m_info.m_CI.genistreamtraits.output_mode = "rectified c";
	m_info.m_CI.genistreamtraits.width    = atof(getParameter("Scan3dRectificationWidth_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
	m_info.m_CI.genistreamtraits.height   = atof(getParameter("Height_RegionSelector_Region1").c_str());
	m_info.m_CI.genistreamtraits.offset_X = 0;
	m_info.m_CI.genistreamtraits.offset_Y = 0;

	m_info.m_RI.cols = m_info.m_CI.genistreamtraits.width;
	m_info.m_RI.rows = m_info.m_CI.genistreamtraits.height;

	m_info.m_CI.genistreamtraits.a_axis_range_scale         = atof(getParameter("Scan3dCoordinateScale_Scan3dCoordinateSelector_CoordinateA_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
	m_info.m_CI.genistreamtraits.a_axis_range_offset        = atof(getParameter("Scan3dCoordinateOffset_Scan3dCoordinateSelector_CoordinateA_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
	m_info.m_CI.genistreamtraits.a_axis_range_min           = 0;
	m_info.m_CI.genistreamtraits.a_axis_range_max           = m_info.m_CI.genistreamtraits.width;
	m_info.m_CI.genistreamtraits.a_axis_range_missing       = "false";
	m_info.m_CI.genistreamtraits.a_axis_range_missing_value = 0;
	
	m_info.m_CI.genistreamtraits.b_axis_range_scale         = atof(getParameter("Scan3dCoordinateScale_Scan3dCoordinateSelector_CoordinateB_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
	m_info.m_CI.genistreamtraits.b_axis_range_offset        = atof(getParameter("Scan3dCoordinateOffset_Scan3dCoordinateSelector_CoordinateB_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
	m_info.m_CI.genistreamtraits.b_axis_range_min           = "-inf";
	m_info.m_CI.genistreamtraits.b_axis_range_max           = "inf";
	m_info.m_CI.genistreamtraits.b_axis_range_missing       = "false";
	m_info.m_CI.genistreamtraits.b_axis_range_missing_value = 0;
	
	
	m_info.m_CI.genistreamtraits.c_axis_range_scale  = atof(getParameter("Scan3dCoordinateScale_Scan3dCoordinateSelector_CoordinateC_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
	m_info.m_CI.genistreamtraits.c_axis_range_offset = atof(getParameter("Scan3dCoordinateOffset_Scan3dCoordinateSelector_CoordinateC_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
	m_info.m_CI.genistreamtraits.c_axis_range_min           = 1;
	m_info.m_CI.genistreamtraits.c_axis_range_max           = 65535;
	m_info.m_CI.genistreamtraits.c_axis_range_missing = "true";
	m_info.m_CI.genistreamtraits.c_axis_range_missing_value = 0;

	m_info.m_CI.genistreamtraits.unit = "millimeter";


	m_Param.m_CaliPath = "";
	m_calibration_inDevice = true;
	m_keepCalibratedDataAsUint16 = range_Uint16;
	m_convCalibratedDataToFloat_enableOMP = convertRange_ToFloat_ByOpenMP;

	return CAM_STATUS::All_OK;
}

EXPORT_TO_DLL CAM_STATUS 
Ranger3::setCalibration_inDevice_CalibrateAC(const bool range_Uint16, const bool rangeX_Uint16, const bool enableOpenMP)
{
	// in device calibration enable -----
	if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;

	if (CAM_STATUS::All_OK != setParameterValue("Scan3dOutputMode_Scan3dExtractionSelector_Scan3dExtraction1", "CalibratedAC")) 
	{
		m_calibration_inDevice = false;
		*m_log << CustomerLog::time() << "[" << m_DeviceName << "]"
			<< "try to set [Scan3dOutputMode_Scan3dExtractionSelector_Scan3dExtraction1] as [CalibratedAC] failed! \
				1. Check if in-device calibration file is in device now? 2. Check if firmware is higher than 3.0?\n";
		return CAM_STATUS::ERROR_OPERATION_NOT_ALLOW;
	}

	//if (CAM_STATUS::All_OK != setParameterValue("Scan3dRectificationWidth_Scan3dExtractionSelector_Scan3dExtraction1", std::to_string(rectificationWidth)))
	//{
	//	m_calibration_inDevice = false;
	//	*m_log << CustomerLog::time()
	//		<< "[" << m_DeviceName << "]" << "try to set [Scan3dRectificationWidth_Scan3dExtractionSelector_Scan3dExtraction1] as [" << rectificationWidth
	//		<< "] failed! 1. Value must be 32's multiple(" << rectificationWidth << "/32=" << (rectificationWidth / 32.0)
	//		<< "); 2. Check if in-device calibration file is in device now? 3. Check if firmware is higher than 3.0?\n";
	//	return CAM_STATUS::ERROR_OPERATION_NOT_ALLOW;
	//}

	m_info.m_CI.genistreamtraits.output_mode = "calibrated ac";
	m_info.m_CI.genistreamtraits.width = atoi(getParameter("Width_RegionSelector_Region1").c_str());
	m_info.m_CI.genistreamtraits.height = atoi(getParameter("Height_RegionSelector_Region1").c_str());;
	m_info.m_CI.genistreamtraits.offset_X = 0;
	m_info.m_CI.genistreamtraits.offset_Y = 0;

	m_info.m_RI.cols = m_info.m_CI.genistreamtraits.width;
	m_info.m_RI.rows = m_info.m_CI.genistreamtraits.height;


	m_info.m_CI.genistreamtraits.a_axis_range_scale = atof(getParameter("Scan3dCoordinateScale_Scan3dCoordinateSelector_CoordinateA_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
	m_info.m_CI.genistreamtraits.a_axis_range_offset = atof(getParameter("Scan3dCoordinateOffset_Scan3dCoordinateSelector_CoordinateA_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
	m_info.m_CI.genistreamtraits.a_axis_range_min = 0;
	m_info.m_CI.genistreamtraits.a_axis_range_max = m_info.m_CI.genistreamtraits.width;
	m_info.m_CI.genistreamtraits.a_axis_range_missing = "false";
	m_info.m_CI.genistreamtraits.a_axis_range_missing_value = 0;

	m_info.m_CI.genistreamtraits.b_axis_range_scale = atof(getParameter("Scan3dCoordinateScale_Scan3dCoordinateSelector_CoordinateB_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
	m_info.m_CI.genistreamtraits.b_axis_range_offset = atof(getParameter("Scan3dCoordinateOffset_Scan3dCoordinateSelector_CoordinateB_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
	m_info.m_CI.genistreamtraits.b_axis_range_min = "-inf";
	m_info.m_CI.genistreamtraits.b_axis_range_max = "inf";
	m_info.m_CI.genistreamtraits.b_axis_range_missing = "false";
	m_info.m_CI.genistreamtraits.b_axis_range_missing_value = 0;


	m_info.m_CI.genistreamtraits.c_axis_range_scale = atof(getParameter("Scan3dCoordinateScale_Scan3dCoordinateSelector_CoordinateC_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
	m_info.m_CI.genistreamtraits.c_axis_range_offset = atof(getParameter("Scan3dCoordinateOffset_Scan3dCoordinateSelector_CoordinateC_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
	m_info.m_CI.genistreamtraits.c_axis_range_min = 1;
	m_info.m_CI.genistreamtraits.c_axis_range_max = 65535;
	m_info.m_CI.genistreamtraits.c_axis_range_missing = "true";
	m_info.m_CI.genistreamtraits.c_axis_range_missing_value = 0;

	m_info.m_CI.genistreamtraits.unit = "millimeter";


	m_Param.m_CaliPath = "";
	m_calibration_inDevice = true;
	m_calibration_inDevice_CalibratedAC = true;

	m_keepCalibratedDataAsUint16 = range_Uint16;
	m_keepCalibratedAC_DataX_AsUint16 = rangeX_Uint16;
	m_convCalibratedDataToFloat_enableOMP = enableOpenMP;

	return CAM_STATUS::All_OK;
}

EXPORT_TO_DLL CAM_STATUS	
Ranger3::setCalibration_No()
{
	if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;
	m_calibration_inDevice = false;
	m_Param.m_CaliPath = "";
	
	setParameterValue("Scan3dOutputMode_Scan3dExtractionSelector_Scan3dExtraction1", "UncalibratedC");

	m_info.m_CI.genistreamtraits.output_mode = "uncalibrated c";
	m_info.m_CI.genistreamtraits.width = atoi(getParameter("Width_RegionSelector_Region1").c_str());
	m_info.m_CI.genistreamtraits.height = atoi(getParameter("Height_RegionSelector_Region1").c_str());;
	m_info.m_CI.genistreamtraits.offset_X = 0;
	m_info.m_CI.genistreamtraits.offset_Y = 0;

	m_info.m_RI.cols = m_info.m_CI.genistreamtraits.width;
	m_info.m_RI.rows = m_info.m_CI.genistreamtraits.height;


	m_info.m_CI.genistreamtraits.a_axis_range_scale = atof(getParameter("Scan3dCoordinateScale_Scan3dCoordinateSelector_CoordinateA_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
	m_info.m_CI.genistreamtraits.a_axis_range_offset = atof(getParameter("Scan3dCoordinateOffset_Scan3dCoordinateSelector_CoordinateA_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
	m_info.m_CI.genistreamtraits.a_axis_range_min = 0;
	m_info.m_CI.genistreamtraits.a_axis_range_max = m_info.m_CI.genistreamtraits.width;
	m_info.m_CI.genistreamtraits.a_axis_range_missing = "false";
	m_info.m_CI.genistreamtraits.a_axis_range_missing_value = 0;

	m_info.m_CI.genistreamtraits.b_axis_range_scale = atof(getParameter("Scan3dCoordinateScale_Scan3dCoordinateSelector_CoordinateB_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
	m_info.m_CI.genistreamtraits.b_axis_range_offset = atof(getParameter("Scan3dCoordinateOffset_Scan3dCoordinateSelector_CoordinateB_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
	m_info.m_CI.genistreamtraits.b_axis_range_min = "-inf";
	m_info.m_CI.genistreamtraits.b_axis_range_max = "inf";
	m_info.m_CI.genistreamtraits.b_axis_range_missing = "false";
	m_info.m_CI.genistreamtraits.b_axis_range_missing_value = 0;


	m_info.m_CI.genistreamtraits.c_axis_range_scale = atof(getParameter("Scan3dCoordinateScale_Scan3dCoordinateSelector_CoordinateC_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
	m_info.m_CI.genistreamtraits.c_axis_range_offset = atof(getParameter("Scan3dCoordinateOffset_Scan3dCoordinateSelector_CoordinateC_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
	m_info.m_CI.genistreamtraits.c_axis_range_min = 1;
	m_info.m_CI.genistreamtraits.c_axis_range_max = 13311; // This is not the real value
	m_info.m_CI.genistreamtraits.c_axis_range_missing = "true";
	m_info.m_CI.genistreamtraits.c_axis_range_missing_value = 0;
	m_info.m_CI.genistreamtraits.unit = "pixel";

	return CAM_STATUS::All_OK;
}

EXPORT_TO_DLL CAM_STATUS 
Ranger3::importConfiguretionFile(const Str & filePath)
{
	if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;

	if (!filePath.empty())
	{
		// Connect device node map to GenTL
		std::ifstream inputStream(filePath);
		if (inputStream.good()) {
			try {
				ImportParamCSV(m_deviceNodeMap._Ptr, inputStream);
			}
			catch (std::exception e) {
				*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" << e.what() << "\n";
				return CAM_STATUS::ERROR_COMMUNICATION;
			}
			catch (...) {
				*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" << "Fatal Error happend when importing parameters from " << filePath << "\n";
				return CAM_STATUS::ERROR_COMMUNICATION;
			}

			*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" << "Imported device parameters from " << filePath << "\n";
			m_Param.m_ParaPath = filePath;
		}
		else {
			*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" << "ECode: ERROR_CSV_PATH. Parameter file " << filePath << " could not be opened!" << "\n";
			return CAM_STATUS::ERROR_CSV_PATH;
		}
	}
	else {
		*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" << "* Warning: Parameter file is empty! Using inner parameters instead!" << "\n";
		return CAM_STATUS::All_OK;
	}

	return CAM_STATUS::All_OK;
}

EXPORT_TO_DLL CAM_STATUS 
Ranger3::exportConfiguretionFile(const Str & filePath)
{
	if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;

	if (!filePath.empty())
	{
		// Connect device node map to GenTL
		std::ofstream outputStream(filePath);
		if (outputStream.good()) {
			try {
				ExportParamCSV(m_deviceNodeMap._Ptr, outputStream);
			}
			catch (std::exception e) {
				*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" << e.what() << "\n";
				return CAM_STATUS::ERROR_COMMUNICATION;
			}
			catch (...) {
				*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" << "Fatal Error happend when exporting parameters from " << filePath << "\n";
				return CAM_STATUS::ERROR_COMMUNICATION;
			}

			*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" << "Exporting device parameters from " << filePath << "\n";
			m_Param.m_ParaPath = filePath;
			return CAM_STATUS::All_OK;
		}
		
	}

	*m_log << CustomerLog::time() << "[" << m_DeviceName << "]" << "ECode: ERROR_CSV_PATH. Parameter saving path " << filePath << " is invalid!" << "\n";
	return CAM_STATUS::ERROR_CSV_PATH;
};

EXPORT_TO_DLL CAM_STATUS
Ranger3::setParametPath(const Str & value)
{
	mark_Obsolete(m_log, m_DeviceName, "setParametPath", "importConfiguretionFile()");
	return importConfiguretionFile(value);
}

EXPORT_TO_DLL CAM_STATUS 
Ranger3::enableCallbackThreadsInPool(const size_t& numThreads, const bool enable)
{
	if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;

#ifdef CALLBACK_THREAD_POOL
	m_callbackPool = enable ? std::make_unique<ThreadPool>(numThreads) : nullptr;
#endif // CALLBACK_THREAD_POOL

	return CAM_STATUS::All_OK;
}

EXPORT_TO_DLL CAM_STATUS
Ranger3::setIp(cStr& _IP, bool isPersistent)
{
	if (isPersistent)
	{
		if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
		if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;
	}
	else
	{
		if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
		if (getStatus() == CAM_STATUS::CAM_IS_STOPPED)		return CAM_STATUS::ERROR_CAM_IS_STOPPED;
		if (getStatus() == CAM_STATUS::CAM_IS_CONNECTED)	return CAM_STATUS::ERROR_CAM_IS_CONNECTED;
	}

	if (isPersistent && m_status == CAM_STATUS::CAM_IS_CONNECTED)
	{
		if (true == m_Param.setIP(	m_device->mDeviceNodeMap, m_device->mInterfaceHandle, m_consumer->tl(),
									m_device->mId, _IP, true))
		{
			// diconnect \ forceIP \ connect --------------
			if (CAM_STATUS::All_OK == disconnectCamera())
			{
				if (true == m_Param.setIP(m_device->mInterfaceNodeMap, m_device->mInterfaceHandle, m_consumer->tl(),
					m_device->mId, _IP, false))
				{
				    __sleep1MS(100);
					return connectCamera();
				}
			}
		}
	}else{
		if (true == m_Param.setIP(m_device->mInterfaceNodeMap, m_device->mInterfaceHandle, m_consumer->tl(),
			m_device->mId, _IP, false))
			return CAM_STATUS::All_OK;
	}
	
	return CAM_STATUS::ERROR_SET_IP;
}

EXPORT_TO_DLL CAM_STATUS
Ranger3::setSubnet(cStr& _Su, bool isPersistent)
{
	if (isPersistent)
	{
		if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
		if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;
	}	else	{
		if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
	}

	if (isPersistent && m_status == CAM_STATUS::CAM_IS_CONNECTED)
	{
		if (true == m_Param.setSubnet(  m_device->mDeviceNodeMap,
                                        m_device->mInterfaceHandle,
                                        m_consumer->tl(),
                                        m_device->mId, _Su, true))
		{
			// diconnect \ forceIP \ connect --------------
			if (CAM_STATUS::All_OK == disconnectCamera())	{
				if (true == m_Param.setSubnet(m_device->mInterfaceNodeMap, m_device->mInterfaceHandle, m_consumer->tl(),
					m_device->mId, _Su, false))
					return connectCamera();
			}
		}
	}	else	{
		if (true == m_Param.setSubnet(m_device->mInterfaceNodeMap, m_device->mInterfaceHandle, m_consumer->tl(),
			m_device->mId, _Su, false))
			return CAM_STATUS::All_OK;
	}

	return CAM_STATUS::ERROR_SET_SUBNET;
}

EXPORT_TO_DLL CAM_STATUS
Ranger3::setParameterValue(cStr& _ParamterName, cStr& value)
{
	if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;

	if (!m_Param.isValid())	{
		auto err = _scanDeviceParameter();
		if (err != CAM_STATUS::All_OK)					return err;
	}

	if (m_Param.isValid())	{
		if (true == m_Param.setParameter(m_deviceNodeMap, _ParamterName, value))
			return CAM_STATUS::All_OK;
		else
			return CAM_STATUS::ERROR_PARAMETER_VALUE_INVALID;
	}
	return CAM_STATUS::ERROR_PARAMETER_INVALID;
}

EXPORT_TO_DLL CAM_STATUS
Ranger3::setParameter(cStr& _ParamterName, cStr& value)
{
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;
	if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;

	try
	{
		// move selector
		std::vector<std::pair<std::string, std::string>> selectors; // top is close to the root
		auto parameter = _ParamterName;
		while (!parameter.empty())
		{
			auto id = parameter.find_last_of("_");
			if (id == std::string::npos)
				break;

			std::string selector_value = parameter.substr(id + 1, parameter.size() - id - 1);
			parameter = parameter.substr(0, id);

			id = parameter.find_last_of("_");
			std::string selector_name = parameter.substr(id + 1, parameter.size() - id - 1);
			parameter = parameter.substr(0, id);

			selectors.push_back({ selector_name , selector_value });
		}

		for (auto sub : selectors)
		{
			if (sub.second != "")
				*(GenApi::CEnumerationPtr(m_deviceNodeMap._GetNode(sub.first.c_str()))) = sub.second.c_str();
		}

		// get type by node name, set parameter by string.
		auto node = m_deviceNodeMap._GetNode(parameter.c_str());
		if (node == nullptr)
		{
			*m_log << CustomerLog::time() << "[Ranger3::setParameter]: " << parameter << " is invalid! \n";
			return CAM_STATUS::ERROR_PARAMETER_INVALID;
		}

		auto type = node->GetPrincipalInterfaceType();
		if (type == GenApi::intfIInteger) {
			GenApi::CIntegerPtr val = static_cast<GenApi::CIntegerPtr>(node);
			val->FromString(GENICAM_NAMESPACE::gcstring(value.c_str()), false);
		}
		else if (type == GenApi::intfIFloat) {
			GenApi::CFloatPtr val = static_cast<GenApi::CFloatPtr>(node);
			val->FromString(GENICAM_NAMESPACE::gcstring(value.c_str()), false);
		}
		else if (type == GenApi::intfIString) {
			GenApi::CStringPtr val = static_cast<GenApi::CStringPtr>(node);
			val->SetValue(GENICAM_NAMESPACE::gcstring(value.c_str()), false);
		}
		else if (type == GenApi::intfIBoolean) {
			GenApi::CBooleanPtr val = static_cast<GenApi::CBooleanPtr>(node);
			val->FromString(GENICAM_NAMESPACE::gcstring(value.c_str()), false);
		}
		else if (type == GenApi::intfIEnumeration) {
			GenApi::CEnumerationPtr val = static_cast<GenApi::CEnumerationPtr>(node);
			val->FromString(GENICAM_NAMESPACE::gcstring(value.c_str()), false);
		}
		else if (type == GenApi::intfICommand) {
			GenApi::CCommandPtr val = static_cast<GenApi::CCommandPtr>(node);
			val->Execute();
		}
		else {
			// Do nothing, the other types aren't interesting
			return CAM_STATUS::ERROR_PARAMETER_INVALID;
		}
		return CAM_STATUS::All_OK;
	}
	catch (std::exception &e)
	{
		*m_log << CustomerLog::time() << "[Ranger3::setParameter]: Failed! Current device status = " << CAM_STATUS_str(getStatus()) << ", exception: " << e.what() << " \n";
		return CAM_STATUS::ERROR_PARAMETER_INVALID;
	}
	catch (...)
	{
		*m_log << CustomerLog::time() << "[Ranger3::setParameter]: Failed! Current device status = " << CAM_STATUS_str(getStatus()) << " \n";
		return CAM_STATUS::ERROR_PARAMETER_INVALID;
	}

}

EXPORT_TO_DLL Str
Ranger3::getIp(bool isPersistent)
{
	if (isPersistent)	{
		if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return Str();
		if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return Str();
		
		return m_DeviceIP = std::string(m_Param.getIP(
			m_device->mDeviceNodeMap, m_device->mInterfaceHandle, m_consumer->tl(),
			m_device->mId, true).c_str());
	}	else	{
		if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return Str();

		return m_DeviceIP = m_Param.getIP(
			m_device->mInterfaceNodeMap, m_device->mInterfaceHandle, m_consumer->tl(),
			m_device->mId, false);
	}
}

EXPORT_TO_DLL Str
Ranger3::getSubNet(bool isPersistent) const
{
	if (isPersistent)	{
		if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return Str();
		if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return Str();

		return m_Param.getSubnet(
			m_device->mDeviceNodeMap, m_device->mInterfaceHandle, m_consumer->tl(),
			m_device->mId, true);
	}	else	{
		if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return Str();
		
		return m_Param.getSubnet(
			m_device->mInterfaceNodeMap, m_device->mInterfaceHandle, m_consumer->tl(),
			m_device->mId, false);
	}
}

EXPORT_TO_DLL cStr 
Ranger3::getTemperature() const
{
	Str value("");
	m_Param.getParameter(m_deviceNodeMap, "DeviceTemperature", value);
	return value;
}

EXPORT_TO_DLL cStr
Ranger3::getParameterValue(cStr& _ParameterName)
{
	if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return Str();
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return Str();

	CAM_STATUS status = CAM_STATUS::All_OK;
	if (!m_Param.isValid())
		status = _scanDeviceParameter();
	if (CAM_STATUS::All_OK != status)					return Str();

	if (m_Param.isValid())	
	{
		Str value;
		if (true == m_Param.getParameter(m_deviceNodeMap, _ParameterName, value))
			return value;
	}
	return Str();
}

EXPORT_TO_DLL cStr
Ranger3::getParameter(cStr& _ParameterName)
{
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return "";

	try
	{
		// move selector
		std::vector<std::pair<std::string, std::string>> selectors; // top is close to the root
		auto parameter = _ParameterName;
		while (!parameter.empty())
		{
			auto id = parameter.find_last_of("_");
			if (id == std::string::npos)
				break;

			std::string selector_value = parameter.substr(id + 1, parameter.size() - id - 1);
			parameter = parameter.substr(0, id);

			id = parameter.find_last_of("_");
			std::string selector_name = parameter.substr(id + 1, parameter.size() - id - 1);
			parameter = parameter.substr(0, id);

			selectors.push_back({ selector_name , selector_value });
		}

		for (auto sub : selectors)
		{
			if (sub.second != "")
				*(GenApi::CEnumerationPtr(m_deviceNodeMap._GetNode(sub.first.c_str()))) = sub.second.c_str();
		}

		// get type by node name, set parameter by string.
		auto node = m_deviceNodeMap._GetNode(parameter.c_str());
		if (node == nullptr)
		{
			*m_log << CustomerLog::time() << "[Ranger3::getParameter]: " << _ParameterName << " is invalid!\n";
			return "";
		}

		auto type = node->GetPrincipalInterfaceType();
		if (type == GenApi::intfIInteger || type == GenApi::intfIString ||
			type == GenApi::intfIBoolean || type == GenApi::intfIEnumeration)
		{
			auto val = static_cast<GenApi::CValuePtr>(node);
			return std::string(val->ToString().c_str());
		}
		else if (type == GenApi::intfIFloat) {
			auto val = static_cast<GenApi::CFloatPtr>(node);
			return std::string(val->ToString().c_str());
		}
		return "";
	}
	catch (std::exception &e)
	{
		*m_log << CustomerLog::time() << "[Ranger3::getParameter]: Failed! Current device status = " << CAM_STATUS_str(getStatus()) << ", exception: " << e.what() << " \n";
		return "";
	}
	catch (...)
	{
		*m_log << CustomerLog::time() << "[Ranger3::getParameter]: Failed! Current device status = " << CAM_STATUS_str(getStatus()) <<" \n";
		return "";
	}
}

EXPORT_TO_DLL CAM_STATUS
Ranger3::saveParameterToCSV(cStr& path)
{
	mark_Obsolete(m_log, m_DeviceName, "saveParameterToCSV", "exportConfiguretionFile()");
	return exportConfiguretionFile(path);
}

EXPORT_TO_DLL CAM_STATUS
Ranger3::loadParameterFrCSV(cStr & para_csv_path)
{
	mark_Obsolete(m_log, m_DeviceName, "loadParameterFrCSV", "importConfiguretionFile()");
	return importConfiguretionFile(para_csv_path);
}

EXPORT_TO_DLL Str			
Ranger3::getAllParameterInfo(bool do_not_use_it_any_more__replaced_by_getParametersAsStructureString) const
{
	mark_Obsolete(m_log, m_DeviceName, "getAllParameterInfo", "getParametersAsStructureString()");
	return getParametersAsStructureString();
}

EXPORT_TO_DLL CAM_STATUS
Ranger3::getAllParameterInfo(AllParams & Info, const bool forceUpdate)
{
	if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;

	CAM_STATUS status = CAM_STATUS::All_OK;
	if (!m_Param.isValid() || forceUpdate)
		status = _scanDeviceParameter();

	if (CAM_STATUS::All_OK != status)					return status;

	Info = m_Param.getAllParams();
	if (Info.empty())
		return CAM_STATUS::ERROR_PARAMETERS_EMPTY;
	
	return status;
}

EXPORT_TO_DLL CAM_STATUS
Ranger3::getAllParameterInfo(Categories & cate, Params & para, const bool forceUpdate)
{
	if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;

	CAM_STATUS status = CAM_STATUS::All_OK;
	if (!m_Param.isValid() || forceUpdate)
		status = _scanDeviceParameter();
	if (CAM_STATUS::All_OK != status)					return status;

	cate = m_Param.getCategories();
	para = m_Param.getParams();

	return CAM_STATUS::All_OK;
}

EXPORT_TO_DLL Str
Ranger3::getDeviceInfoAsStructureString		() const
{
    std::stringstream ss;
    ss
        << "Device Name       =" << getDeviceName()
        << "\nDevice IP         =" << m_DeviceIP
        << "\nDevice SubnetMask =" << getSubNet()
		<< "\nDevice MAC        =" << getMac()
		<< "\nDevice DeviceModelName      =" << m_DeviceModelName
		<< "\nDevice DeviceVersion        =" << m_DeviceVersion
		<< "\nDevice DeviceFirmwareVersion=" << m_DeviceFirmwareVersion
		<< "\nDevice DeviceSerialNumber   =" << m_DeviceSerialNumber
		<< "\nDevice DeviceUserID         =" << const_cast<Ranger3&>(*this).getParameterValue("DeviceUserID")
		<< "\nDevice DeviceTemperature    =" << const_cast<Ranger3&>(*this).getParameterValue("DeviceTemperature")
    ;
    return ss.str();
}

#ifdef _WIN32

EXPORT_TO_DLL CAM_STATUS
Ranger3::sendFileToCamera(std::istream& contents)
{
#ifndef DEBUG_CODE
	try
#endif // !DEBUG_CODE
	{
		if (contents.good())
		{
			std::ostringstream ss;
			ss << contents.rdbuf();
			const std::string str = ss.str();
			GenApi::FileProtocolAdapter adapter;
			geniranger::sendFileContents(str, "UserFile", m_deviceNodeMap._Ptr, adapter);
			return CAM_STATUS::All_OK;
		}
		else
		{
			*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" << "Failed when sendFileToCamera(), the file is invalid. Please check the path is correct and make sure the file exists in your PC.";
			return CAM_STATUS::ERROR_USER_FILE_NOT_FOUND_IN_PC;
		}
	}
#ifndef DEBUG_CODE
	catch (std::exception &e)
	{
		*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" << e.what();
		return CAM_STATUS::ERROR_USER_FILE_SEND_FAILED;
	}
	catch (...)
	{
		return CAM_STATUS::ERROR_USER_FILE_SEND_FAILED;
	}
#endif // !DEBUG_CODE
}

EXPORT_TO_DLL CAM_STATUS 
Ranger3::sendFileToCamera(const std::string & filePath)
{
	std::ifstream contents(filePath, std::ifstream::binary);
	return sendFileToCamera(contents);
}

EXPORT_TO_DLL std::string
Ranger3::retrieveFileFromCamera()
{
	try
	{
		return geniranger::retrieveFileContents("UserFile", m_deviceNodeMap._Ptr);
	}
	catch (std::exception &e)
	{
		*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" << e.what();
		return std::string();
	}
	catch (...)
	{
		return std::string();
	}
}

EXPORT_TO_DLL CAM_STATUS
Ranger3::deleteFileFromCamera()
{
	try
	{
		GenApi::FileProtocolAdapter adapter;
		geniranger::deleteFile("UserFile", m_deviceNodeMap._Ptr, adapter);
		return CAM_STATUS::All_OK;
	}
	catch (std::exception &e)
	{
		*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" << e.what();
		return CAM_STATUS::ERROR_USER_FILE_NOT_EXIST;
	}
	catch (...)
	{
		return CAM_STATUS::ERROR_USER_FILE_NOT_EXIST;
	}

}

EXPORT_TO_DLL UserSet
Ranger3::getUserSet(const UserSetId & id, CAM_STATUS & is_All_OK)
{
	if (m_userSets.count(id) == 1)
	{
		return m_userSets.at(id);
	}
	else
		is_All_OK = CAM_STATUS::ERROR_USER_SET_NOT_FOUND;
	
	return m_userSets.at(UserSetId::DEFAULT);
};

EXPORT_TO_DLL UserSet		
Ranger3::getUserSet(const UserSetId & id)
{
	return m_userSets.count(id) == 1 ? m_userSets.at(id) : throw "Invalid UserSetId!!!";
};

EXPORT_TO_DLL CAM_STATUS	
Ranger3::userSet_load(const int & id)
{
	if (m_userSets.count(UserSetId(id)) == 1)
	{
		auto userSet = m_userSets.at(UserSetId(id));
		if (userSet.exists())
		{
			userSet.load();
			return CAM_STATUS::All_OK;
		}
	}
	return CAM_STATUS::ERROR_USER_FILE_NOT_EXIST;
};

EXPORT_TO_DLL CAM_STATUS	
Ranger3::userSet_save(const int & id)
{
	if (id == 0)
		return CAM_STATUS::ERROR_OPERATION_NOT_ALLOW;

	if (m_userSets.count(UserSetId(id)) == 1)
	{
		auto userSet = m_userSets.at(UserSetId(id));
		userSet.save();
		return CAM_STATUS::All_OK;
	}
	return CAM_STATUS::ERROR_USER_FILE_NOT_EXIST;
}

EXPORT_TO_DLL CAM_STATUS 
Ranger3::userSet_set_default(const int & id)
{
	if (id == 0)
		return CAM_STATUS::ERROR_OPERATION_NOT_ALLOW;

	if (m_userSets.count(UserSetId(id)) == 1)
	{
		auto userSet = m_userSets.at(UserSetId(id));
		if (userSet.exists())
		{
			userSet.useAtStartup();
			return CAM_STATUS::All_OK;
		}
	}
	return CAM_STATUS::ERROR_USER_FILE_NOT_EXIST;
}

EXPORT_TO_DLL CAM_STATUS	
Ranger3::userSet_set_description(const int & id, const std::string & description)
{
	if (id == 0)
		return CAM_STATUS::ERROR_OPERATION_NOT_ALLOW;

	if (m_userSets.count(UserSetId(id)) == 1)
	{
		auto userSet = m_userSets.at(UserSetId(id));
		if (userSet.exists())
		{
			userSet.setDescription(description);
			return CAM_STATUS::All_OK;
		}
	}
	return CAM_STATUS::ERROR_USER_FILE_NOT_EXIST;
}

EXPORT_TO_DLL std::string	
Ranger3::userSet_get_description(const int & id)
{
	if (m_userSets.count(UserSetId(id)) == 1)
	{
		auto userSet = m_userSets.at(UserSetId(id));
		if (userSet.exists())
		{
			return userSet.getDescription();
		}
	}
	return "";
}

EXPORT_TO_DLL bool 
Ranger3::excuteCommand(cStr & CommandName, bool do_not_use_it_any_more)
{
	GenApi::CCommandPtr command = m_deviceNodeMap._GetNode(CommandName.c_str());
	try
	{
		command->Execute();
		return true;
	}
	catch (...)
	{
		return false;
	}
}

EXPORT_TO_DLL bool
Ranger3::updateFirmware(const std::string& firmwarePackagePath)
{
	*m_log << CustomerLog::time() << "[Ranger3::updateFirmware]: Check file ... (Path=" << firmwarePackagePath << ") \n";
	std::ifstream fin(firmwarePackagePath, std::ifstream::binary);
	if (!fin.is_open())
	{
		*m_log << CustomerLog::time() << "[Ranger3::updateFirmware]: Error! Invalid firmware path! (Path=" << firmwarePackagePath << ")\n";
		return false;
	}
	*m_log << CustomerLog::time() << "[Ranger3::updateFirmware]: Check file OK!\n";



	*m_log << CustomerLog::time() << "[Ranger3::updateFirmware]: Send firmware to device (\"Link/Data\" LED flashing fast) ... \n";
	const uint32_t checksum = geniranger::Crc32().calculate(firmwarePackagePath);
	try {
		geniranger::updateFirmware(firmwarePackagePath,
			checksum,
			m_deviceNodeMap);
	}
	catch (const char * exp)
	{
		if (std::string(exp) == "FirmwareChecksum node not found in node map.")
		{
			*m_log << CustomerLog::time() << "[Ranger3::updateFirmware]: Error! Parameter [FirmwareChecksum] not found in device! (Path=" << firmwarePackagePath << ")\n";
			return false;
		}

		if (std::string(exp) == "FirmwareChecksum node not found in node map.")
		{
			*m_log << CustomerLog::time() << "[Ranger3::updateFirmware]: Error! Parameter [FirmwarePerformUpdate] not found in device! (Path=" << firmwarePackagePath << ")\n";
			return false;
		}
	}
	catch(...)
	{
		*m_log << CustomerLog::time() << "[Ranger3::updateFirmware]: Error! geniranger::updateFirmware() failed! (Path=" << firmwarePackagePath << ")\n";
		return false;
	}
	*m_log << CustomerLog::time() << "[Ranger3::updateFirmware]: Send firmware to device OK\n";


	*m_log << CustomerLog::time() << "[Ranger3::updateFirmware]: Excute firmware update (\"State\" LED flashing fast)... \n";
	std::string status;
	*m_log << CustomerLog::time() << "[Ranger3::updateFirmware]: Firmware updating will use 230 seconds roughly. \n";
	//for(int i=0; i< waitForNSeconds; ++i)
	int i = 0;
	while(true)
	{
		status = getParameter("FirmwareUpdateProgress");
		//if ((status = getParameter("FirmwareUpdateProgress")) == "Completed")
		//{
		//	// For most of time, it will not be called. Because when updating completed, camera will lost.
		//	*m_log << "\n";
		//	*m_log << CustomerLog::time() << "[Ranger3::updateFirmware]: Excute firmware update OK! \n";
		//	return true;
		//}

		if (status.find("Error") != std::string::npos)
		{
			*m_log << "\n";
			*m_log << CustomerLog::time() << "[Ranger3::updateFirmware]: Firmware update failed with (" + status + ")\n";
			_disconnectCamera();
			_freeDevice(); 
			return false;
		}

		if (status == "")
		{
			// When updating completed, camera will lost. getParameter() retrun ""
			*m_log << "\n";
			*m_log << CustomerLog::time() << "[Ranger3::updateFirmware]: Excute firmware update OK! Please rescan device!\n";
			_disconnectCamera();
			_freeDevice();
			return true;
		}

		if(++i%50 == 0)
			*m_log << " 50 seconds passed, please keep patience!\n";
		*m_log << ".";
		__sleep1MS(1000);
	}
}

#endif //  _WIN32


////////////////////////////////////////////////////////////////////////////////

#pragma region not_use_anymore

#ifndef DISABLE_CAL_IN_PC

EXPORT_TO_DLL bool 
Ranger3::resetEncoder(bool do_not_use_it_any_more)
{
	GenApi::CCommandPtr EncoderReset = m_deviceNodeMap._GetNode("EncoderReset");
	EncoderReset->Execute();
	return true;
}

EXPORT_TO_DLL CAM_STATUS
Ranger3::setCalibraPath(const Str& do_not_use_it_any_more)
{
	mark_Obsolete(m_log, m_DeviceName, "setCalibraPath", "setCalibration_inPC()");

	if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;
	std::string value = do_not_use_it_any_more;
	if (value == "" || m_pR3S->_fileExists(value))
	{
		m_Param.m_CaliPath = value;
		return CAM_STATUS::All_OK;
	}
	else
		return CAM_STATUS::ERROR_CALIBRATION_PATH;

};

EXPORT_TO_DLL CAM_STATUS
Ranger3::setCalibration_inPC(const Str& filePath, const bool toDoRectify, const uint32_t& rectificationWidth, const int threadsNumber_X64Only, const int runMode)
{
	if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;
	m_calibration_inDevice = false;

	if (filePath == "" || m_pR3S->_fileExists(filePath)) {
		setParameterValue("Scan3dOutputMode_Scan3dExtractionSelector_Scan3dExtraction1", "UncalibratedC");
		m_Param.m_CaliPath = filePath;
		m_Param.m_ThreadNumber = threadsNumber_X64Only; // X64 only
		m_Param.m_runMode = runMode; // cpu avx

		setDoRectify(toDoRectify, rectificationWidth);
		return CAM_STATUS::All_OK;
	}
	else
	{
		*m_log << CustomerLog::time() << "[" << m_DeviceName << "]" << "setCalibration_inPC failed! Path=" << (filePath.empty() ? "empty" : filePath + " invalid! Please make sure the file exist!") << "\n";
		return CAM_STATUS::ERROR_CALIBRATION_PATH;
	}
};



#endif



#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Ranger3 protected functions

CAM_STATUS
Ranger3::_initDevice(const SPtr<Ranger3Shared> & RS, cStr & ip_mac, const bool useIP)
{
    m_pR3S = RS;

   // m_log = m_pR3S->m_log;
    Str deviceStr = ip_mac;
    std::transform(deviceStr.begin(), deviceStr.end(), deviceStr.begin(), ::tolower);
    auto deviceList = useIP ? m_pR3S->getConDevListIP() : m_pR3S->getConDevListMAC();

    if (deviceList.count(deviceStr) == 1) {
        m_device = deviceList.at(deviceStr);
		m_DeviceIP = m_device->getIp();
        m_deviceHandle = m_device->mDeviceHandle;
        m_DeviceName = m_device->mDeviceName;
        m_consumer = m_pR3S->getConsumer();

		auto _t = m_pR3S->m_logPath.find_last_of(".txt");
		m_log = std::make_shared<CustomerLog>(m_pR3S->m_logPath.substr(0, _t) + m_DeviceName + ".txt",
			m_pR3S->m_enableLogOutput,
			m_pR3S->m_enableLogWriteToFile);

        m_status = CAM_STATUS::CAM_IS_DISCONNECTED;
        m_camType = CamType::_RANGER3;

        if (!m_device->isReachable()) {
            m_status = CAM_STATUS::ERROR_CAM_IS_UNREACHABLE;
            *m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]: Failed to init Ranger3 by " << deviceStr << "! Camera is unreachable! Please check IP settings! \n";
        } else if (m_device->isOccupied()) {
            m_status = CAM_STATUS::ERROR_CAM_IS_OCCUPIED;
            *m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]: Failed to init Ranger3 by " << deviceStr
                   << "! Camera is occupied! Please close the other program which is using camera now! \n";
        } else
            *m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]: Init " << m_DeviceName.c_str() << " by " << (useIP ? "IP(" : "MAC(") << deviceStr << "), OK! \n";
        return CAM_STATUS::All_OK;
    }
    else
        return CAM_STATUS::ERROR_NULL_PTR_DEV;
}

CAM_STATUS 
Ranger3::_freeDevice()
{
	if (m_ChunkModeActive)
	{
		m_chunkAdapter->detachNodeMap();
		m_chunkAdapter.reset();
		m_ChunkModeActive = false;
	}

	m_device->teardownBuffersWhenLost();
	m_device->closeDataStream();
	m_device->closeDevice();
	m_device = nullptr;
	m_deviceNodeMap = GNodeMap();
#ifndef DISABLE_CAL_IN_PC
	m_pCalibrationWrapper = nullptr;
#endif
	m_pParts = nullptr;
#ifdef _WIN32
	m_deviceNode = NodeMap();
	m_userSets.clear();
#endif
	m_pR3S->_clear_invalid_device(m_DeviceName);


	return CAM_STATUS();
}

void
Ranger3::_setThreadPriority(GNodeMap &dataStreamNodeMap)
{
	GenApi::CIntegerPtr threadPriority		= dataStreamNodeMap._GetNode("StreamThreadPriority");
	GenApi::CCommandPtr setStreamPrio		= dataStreamNodeMap._GetNode("StreamThreadApplyPriority");

	if (threadPriority.IsValid())
		threadPriority->SetValue(15);
	else
		*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" << "WARNING: Could not set thread priority!";

	if (setStreamPrio.IsValid())
		setStreamPrio->Execute();
	else
		*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" << "WARNING: Could not set thread priority!";
}


void
Ranger3::_praseData(
#ifdef __linux__
					GenTLApi* tl,
#else
					SPtr<GenTLApi> tl,
#endif // __linux__
					Vec<GBufferH>&	bufferHandles,
					const size_t&	bufferId,
					ImgT &			imgTable)
{
	imgTable = ImgT();

	// fix [Ranger3-6]
	{
		GenTL::INFO_DATATYPE infoType = GenTL::INFO_DATATYPE_UNKNOWN;
		uint64_t mBufferFrameID;
		auto infoSize = sizeof(mBufferFrameID);
		CC(tl, tl->DSGetBufferInfo(m_device->mDataStreamHandle, bufferHandles[bufferId], GenTL::BUFFER_INFO_FRAMEID, &infoType, &mBufferFrameID, &infoSize));
		m_info.m_id = mBufferFrameID;
	}

	if (!m_IsOutputSensor) 
	{
		// get buffer info
		BufferInfo bufferInfo(tl, m_device->mDataStreamHandle, bufferHandles[bufferId]);
		bool isMultiPart = bufferInfo.mBufferPayloadType == GenTL::PAYLOAD_TYPE_MULTI_PART;

		// VLD-21
		if (0 < bufferInfo.mDeliveredLineCount)
		{
			m_info.m_RI.rows = bufferInfo.mDeliveredLineCount;
		}

		// prepare raw data
		if (isMultiPart){
			m_pParts = std::make_unique<PartInfoCollection>(PartInfoCollection::fromBuffer(
				tl, m_device->mDataStreamHandle, bufferHandles[bufferId], bufferInfo.mPartCount));
		}

		// image data converting to 16 bits
		if (isMultiPart)
		{
			// [RangeA] check rangeA
			_checkRangeA();

			_convertAndCalibrate(imgTable, "Scan3dExtraction1", "Range");
			_convertAndCalibrate(imgTable, "Scan3dExtraction1", "Reflectance");
			_convertAndCalibrate(imgTable, "Scan3dExtraction1", "Scatter");
			_convertAndCalibrate(imgTable, "Scan3dExtraction2", "Range");
			_convertAndCalibrate(imgTable, "Scan3dExtraction2", "Reflectance");
			_convertAndCalibrate(imgTable, "Scan3dExtraction2", "Scatter");
		}
		else
		{
			uint8_t * pBuffer16 = new uint8_t[m_buffer16Size];
			if (bufferInfo.mDataFormat == PFNC_Coord3D_C12p || bufferInfo.mDataFormat == PFNC_Mono12p)
                Convert12pTo16(bufferInfo.mDataPointer, bufferInfo.mBufferDataSize, pBuffer16, &m_buffer16Size);
			else if (bufferInfo.mDataPointer != nullptr)
				std::memcpy(pBuffer16, bufferInfo.mDataPointer, bufferInfo.mBufferDataSize);
			else
			{
				delete[] pBuffer16;
				return;
			}

#ifndef DISABLE_CAL_IN_PC
			if (m_pCalibrationWrapper != nullptr)
			{
				// calibration in PC
				auto cpData = std::make_shared<SiCaliWrapper::calibratedData>(
					m_Param.m_doRectify ?  m_Param.m_RectWidth : m_info.m_RI.cols,
					m_info.m_RI.rows); 
				m_pCalibrationWrapper->calibrate(cpData, pBuffer16, nullptr, nullptr, m_Param.m_doRectify); // range only
				imgTable.setCaliInfo(
									cpData->Width,
									cpData->Height,
									cpData->offsetX, 
									0.0, 0.0,
									cpData->scaleX, 
									m_Param.m_scaleY,1.0,
									cpData->lower_bound_x, 
									cpData->upper_bound_x, 
									cpData->lower_bound_r,
									cpData->upper_bound_r,
					m_info.m_CI.genistreamtraits.a_axis_range_scale, m_info.m_CI.genistreamtraits.a_axis_range_offset, m_info.m_CI.genistreamtraits.c_axis_range_scale, m_info.m_CI.genistreamtraits.c_axis_range_offset);
				imgTable.insertDataCopy(RAN_CAL, (uint8_t*)(cpData->getData(PN::RAN_C)), m_info.m_id);
			}
			else 
#endif
			if (m_calibration_inDevice)
			{
				// calibration in device
				imgTable.setCaliInfo(
					m_Param.m_RectWidth, m_info.m_RI.rows,
					m_info.m_CI.offsetX, m_info.m_CI.offsetY, m_info.m_CI.offsetZ,
					m_info.m_CI.scaleX,  m_info.m_CI.scaleY,  m_info.m_CI.scaleZ,
					m_info.m_CI.lower_bound_x, m_info.m_CI.upper_bound_x, m_info.m_CI.lower_bound_r, m_info.m_CI.upper_bound_r,
					m_info.m_CI.genistreamtraits.a_axis_range_scale, m_info.m_CI.genistreamtraits.a_axis_range_offset, 
					m_info.m_CI.genistreamtraits.c_axis_range_scale, m_info.m_CI.genistreamtraits.c_axis_range_offset);

				if (!m_keepCalibratedDataAsUint16)
				{
					uint8_t* pF = nullptr;
					pF = new uint8_t[4 * m_info.m_RI.cols * m_info.m_RI.rows];
					_ConvertUint16ToFloat(pBuffer16, pF, m_info.m_RI.cols * m_info.m_RI.rows, m_info.m_CI.offsetZ, m_info.m_CI.scaleZ);
					imgTable.insertDataCopy(RAN_CAL, (uint8_t*)pF, m_info.m_id);
					delete[] pF;
				}
				else
				{
					imgTable.insertDataCopy(RAN_CAL_16, (uint8_t*)pBuffer16, m_info.m_id);
				}
			}
			else
			{
				// raw data
				imgTable.setRangeInfo(m_info.m_RI.cols,
					m_info.m_RI.rows,
					m_info.m_RI.aoiOffsetX,
					m_info.m_RI.aoiOffsetY,
					m_info.m_RI.aoiHeight,
					m_info.m_RI.aoiWidth,
					m_info.m_RI.m_RangeAxis);
				imgTable.insertDataCopy(RAN, pBuffer16, m_info.m_id);
			}

			delete[] pBuffer16;
		}

		if (m_ChunkModeActive)
		{
			m_chunkAdapter->attachBuffer(bufferHandles[bufferId], m_device->getBufferData()[bufferId]);

			if(m_FastExtractor == nullptr)
				m_FastExtractor = std::make_shared<ChunkDataExtractor>(m_deviceNodeMap);

			imgTable.setChunkInfo(m_FastExtractor->extract(), m_info.m_id);

			m_chunkAdapter->detachBuffer();
		}

	}
	else
	{   // sensor image
		imgTable.setRangeInfo(	m_info.m_RI.cols, 
								m_info.m_RI.rows, 
								m_info.m_RI.aoiOffsetX, 
								m_info.m_RI.aoiOffsetY, 
								m_info.m_RI.aoiHeight, 
								m_info.m_RI.aoiWidth, 
								m_info.m_RI.m_RangeAxis);

		imgTable.setSensorInfo(	m_info.m_SI.cols, 
								m_info.m_SI.rows, 
								m_info.m_SI.senOffsetX, 
								m_info.m_SI.senOffsetY); // sensor must after range

		imgTable.insertDataCopy(SEN, m_device->getBufferData()[bufferId], m_info.m_id);
	}
}

void
Ranger3::_checkRangeA()
{
	// [RangerA] Alex: No such component but Ranger3Studio will return this! 
	if (m_pParts->mParts.size() >= 2)
	{
		auto rangeA = m_pParts->mParts[0]; // RAN_X
		auto range_ = m_pParts->mParts[1]; // RAN

		if (rangeA.mRegionId == 11 && rangeA.mPurposeId == 4 && range_.mRegionId == 11 && range_.mPurposeId == 4)
		{
			// define in DeviceConnect.cpp, EnumSelectorEntries::EnumSelectorEntries(GenApi::CNodeMapRef& device, const std::string& selectorName)
			// [RangeA] Alex: use a random number 1715 to represent "RangeA" ---
			m_pParts->mParts[0].mPurposeId = RANGE_A_ID;
		}
	}
}

void
Ranger3::_convertAndCalibrate(ImgT & imgTable, cStr& _region, cStr& _component)
{
	if (m_pParts->hasPart(m_regionLookup->value(_region.c_str()), m_componentLookup->value(_component.c_str()))) 
	{
		auto bpi = m_pParts->findPart(m_regionLookup->value(_region.c_str()), m_componentLookup->value(_component.c_str()));
		if (_component == "Range")
		{
			uint8_t* pBuffer16 = new uint8_t[m_buffer16Size];
			// data format
			if (bpi.mPartDataFormat == PFNC_Coord3D_C12p || bpi.mPartDataFormat == PFNC_Mono12p)
                Convert12pTo16(bpi.mPartDataPointer, static_cast<int64_t>(bpi.mPartDataSize), pBuffer16, &m_buffer16Size);
			else
				std::memcpy(pBuffer16, bpi.mPartDataPointer, bpi.mPartDataSize);

			// get reflectance
			bool hasRef		= m_pParts->hasPart	(m_regionLookup->value(_region.c_str()), m_componentLookup->value("Reflectance"));
			uint8_t* pRef	= nullptr;
			if (hasRef)	
			{
				auto bpiRef = m_pParts->findPart(m_regionLookup->value(_region.c_str()), m_componentLookup->value("Reflectance"));
				pRef		= bpiRef.mPartDataPointer;
			}

			// get scatter
			bool hasSca		= m_pParts->hasPart	(m_regionLookup->value(_region.c_str()), m_componentLookup->value("Scatter"));
			uint8_t* pSca	= nullptr;
			if (hasSca)	
			{
				auto bpiSca	= m_pParts->findPart(m_regionLookup->value(_region.c_str()), m_componentLookup->value("Scatter"));
				pSca		= bpiSca.mPartDataPointer;
			}

			// get RangeA
			bool hasRangeA = m_pParts->hasPart(m_regionLookup->value(_region.c_str()), m_componentLookup->value("RangeA"));
			uint8_t* pRangeA16 = nullptr;
			if (hasRangeA)
			{
				pRangeA16 = new uint8_t[m_buffer16Size];
				auto bpiRangeA = m_pParts->findPart(m_regionLookup->value(_region.c_str()), m_componentLookup->value("RangeA"));
				if (bpiRangeA.mPartDataFormat == PFNC_Coord3D_C12p || bpiRangeA.mPartDataFormat == PFNC_Mono12p)
					Convert12pTo16(bpiRangeA.mPartDataPointer, static_cast<int64_t>(bpiRangeA.mPartDataSize), pRangeA16, &m_buffer16Size);
				else
					std::memcpy(pRangeA16, bpiRangeA.mPartDataPointer, bpiRangeA.mPartDataSize);
			}

#ifndef DISABLE_CAL_IN_PC
			if (m_pCalibrationWrapper != nullptr)
			{
				// calibration
				auto cpData = std::make_shared<SiCaliWrapper::calibratedData>(m_Param.m_RectWidth, m_info.m_RI.rows);
				m_pCalibrationWrapper->calibrate(	cpData, 
													pBuffer16, 
													pRef, 
													pSca, 
													m_Param.m_doRectify); 

				auto ptrRef = hasRef ? (uint8_t *)cpData->getData(PN::REF_C) : nullptr;
				auto ptrSca = hasSca ? (uint8_t *)cpData->getData(m_scatterSize == 8 ? PN::SCA_8 : PN::SCA_16) : nullptr;

				DN RANGE_CAL = ImgT::Str2DN("ComponentSelector_Range_RegionSelector_"			+ _region + "_CAL");
				DN REFLE_CAL = ImgT::Str2DN("ComponentSelector_Reflectance_RegionSelector_"		+ _region + "_CAL");
				DN SCATT_CAL = ImgT::Str2DN((m_scatterSize == 8 ? 
											"8_Byte_ComponentSelector_Scatter_RegionSelector_" : 
											"16_Byte_ComponentSelector_Scatter_RegionSelector_") + _region + "_CAL");

				imgTable.setCaliInfo(	cpData->Width,
										cpData->Height,
										cpData->offsetX, 
										0.0, 0.0,
										cpData->scaleX, 
										m_Param.m_scaleY,1.0,
										cpData->lower_bound_x, 
										cpData->upper_bound_x, 
										cpData->lower_bound_r, 
										cpData->upper_bound_r,
					m_info.m_CI.genistreamtraits.a_axis_range_scale, m_info.m_CI.genistreamtraits.a_axis_range_offset, m_info.m_CI.genistreamtraits.c_axis_range_scale, m_info.m_CI.genistreamtraits.c_axis_range_offset);

				imgTable.insertDataCopy(RANGE_CAL, (uint8_t *)cpData->getData(PN::RAN_C),	m_info.m_id);
				imgTable.insertDataCopy(REFLE_CAL, ptrRef,									m_info.m_id);
				imgTable.insertDataCopy(SCATT_CAL, ptrSca,									m_info.m_id);
				if(!m_Param.m_doRectify) // Calibration without rectification.
					imgTable.insertDataCopy(DN::RAN_X, (uint8_t *)cpData->getData(PN::RAN_X), m_info.m_id);
				
			}
			else 
#endif
			if (m_calibration_inDevice)
			{
				imgTable.setCaliInfo(m_info.m_CI);
				imgTable.setRangeInfo(m_info.m_RI);

				if (m_keepCalibratedDataAsUint16)
				{
					imgTable.insertDataCopy(RAN_CAL_16, (uint8_t*)pBuffer16, m_info.m_id);
				}
				else
				{
					uint8_t* pF1 = nullptr;
					pF1 = new uint8_t[4 * m_info.m_RI.cols * m_info.m_RI.rows];
					_ConvertUint16ToFloat(pBuffer16, pF1, m_info.m_RI.cols * m_info.m_RI.rows, m_info.m_CI.offsetZ, m_info.m_CI.scaleZ);
					imgTable.insertDataCopy(RAN_CAL, (uint8_t*)pF1, m_info.m_id);
					delete[] pF1;
				}

				// [RangeA]
				if (hasRangeA)
				{
					if (m_keepCalibratedAC_DataX_AsUint16)
					{
						imgTable.insertDataCopy(RAN_X_16, (uint8_t*)pRangeA16, m_info.m_id); // [RangeA]
					}
					else
					{
						uint8_t* pF = nullptr;
						pF = new uint8_t[4 * m_info.m_RI.cols * m_info.m_RI.rows];
						_ConvertUint16ToFloat(pRangeA16, pF, m_info.m_RI.cols * m_info.m_RI.rows, m_info.m_CI.offsetX, m_info.m_CI.scaleX);
						imgTable.insertDataCopy(RAN_X, (uint8_t*)pF, m_info.m_id);
						delete[] pF;
					}
				}
				

				DN REFLE_CAL = ImgT::Str2DN("ComponentSelector_Reflectance_RegionSelector_" + _region + "_CAL");
				DN SCATT_CAL = ImgT::Str2DN((m_scatterSize == 8 ?
					"8_Byte_ComponentSelector_Scatter_RegionSelector_" :
					"16_Byte_ComponentSelector_Scatter_RegionSelector_") + _region + "_CAL");
				imgTable.insertDataCopy(REFLE_CAL, pRef, m_info.m_id);
				imgTable.insertDataCopy(SCATT_CAL, pSca, m_info.m_id);

			}
			else
			{
				// raw data
				imgTable.setRangeInfo(	m_info.m_RI.cols,
										m_info.m_RI.rows,
										m_info.m_RI.aoiOffsetX,
										m_info.m_RI.aoiOffsetY,
										m_info.m_RI.aoiHeight,
										m_info.m_RI.aoiWidth,
										m_info.m_RI.m_RangeAxis);

				DN RANGE = ImgT::Str2DN("ComponentSelector_Range_RegionSelector_" + _region);
				DN REFLE = ImgT::Str2DN("ComponentSelector_Reflectance_RegionSelector_" + _region);
				DN SCATT = ImgT::Str2DN((m_scatterSize == 8 ?
					"8_Byte_ComponentSelector_Scatter_RegionSelector_" :
					"16_Byte_ComponentSelector_Scatter_RegionSelector_") + _region);

				imgTable.insertDataCopy(RANGE, pBuffer16, m_info.m_id);
				imgTable.insertDataCopy(REFLE, pRef, m_info.m_id);
				imgTable.insertDataCopy(SCATT, pSca, m_info.m_id);
			}
			
			delete[] pBuffer16;
			if (hasRangeA)
			{
				delete[] pRangeA16;
			}
		}
	}
}

CAM_STATUS
Ranger3::_grabSetting(	cStr & calib_path_or_json_string,
						#ifndef DISABLE_CAL_IN_PC
						const RectMethod				& method,
						#endif
						const float						& missingData,
						const bool						_doRectify,
						const double					& rectificationSpread)
{
	m_info.m_RI.rows		= atoi(getParameterValue("Height_RegionSelector_Scan3dExtraction1"	).c_str());
	m_info.m_RI.cols		= atoi(getParameterValue("Width_RegionSelector_Region1"				).c_str());
	m_buffer16Size			= m_info.m_RI.cols * m_info.m_RI.rows * (m_isUsingReflectance ? 3 : 2); // range 2 bytes, reflectance 1 byte.
	m_info.m_RI.aoiHeight	= atoi(getParameterValue("Height_RegionSelector_Region1")	.c_str());
	m_info.m_RI.aoiWidth	= atoi(getParameterValue("Width_RegionSelector_Region1")	.c_str());
	m_info.m_RI.aoiOffsetY	= atoi(getParameterValue("OffsetY_RegionSelector_Region1")	.c_str());
	m_info.m_RI.aoiOffsetX	= atoi(getParameterValue("OffsetX_RegionSelector_Region1")	.c_str());

	// Get Region0 offsetXY
	{
		GenApi::CEnumerationPtr regionSelector	= m_deviceNodeMap._GetNode("RegionSelector");
		*regionSelector							= "Region0";
		GenApi::CIntegerPtr offsetX				= m_deviceNodeMap._GetNode("OffsetX");
		GenApi::CIntegerPtr offsetY				= m_deviceNodeMap._GetNode("OffsetY");
		m_info.m_SI.senOffsetX					= offsetX->GetValue();
		m_info.m_SI.senOffsetY					= offsetY->GetValue();
	}

	// Get range axis
	bool rangeAxisStandard = "Reverse" != getParameterValue("RangeAxis_Scan3dExtractionSelector_Scan3dExtraction1");
	float scaleZ = rangeAxisStandard ? 0.0625f : -0.0625f;
	float originZ(0.0f);
	if (!calib_path_or_json_string.empty())
	{
		auto suffix = calib_path_or_json_string.substr(calib_path_or_json_string.length() - 3, 3);
		if (suffix == "XML" || suffix == "xml")
			originZ = (float) (rangeAxisStandard ? m_info.m_RI.aoiOffsetY + 2.0f * scaleZ : m_info.m_RI.aoiOffsetY + m_info.m_RI.aoiHeight - 2.0f * scaleZ);
		else
			originZ = (float) (rangeAxisStandard ? m_info.m_RI.aoiOffsetY : m_info.m_RI.aoiOffsetY + m_info.m_RI.aoiHeight);
	}

	m_info.m_RI.m_RangeAxis = rangeAxisStandard ? RA_STANDARD : RA_REVERSED;


	//Initialize the calibrationFilter and rectification filter, in pc

	if (!_inDeviceCalibrationCheck())
	{
#ifndef DISABLE_CAL_IN_PC
		// if no in device, in PC or grab raw data 
		if (calib_path_or_json_string.empty()) {
			*m_log << CustomerLog::time() << "[" << m_DeviceName << "]" << "* Warning: No using calibration file!! Get raw data!\n";
			m_pCalibrationWrapper = nullptr;
			return CAM_STATUS::All_OK;
		}
		else
		{
			*m_log << CustomerLog::time() << "[" << m_DeviceName << "]" << "* Calibration file in PC: (" << calib_path_or_json_string << ")\n";
			m_Param.m_doRectify = _doRectify;

			m_pCalibrationWrapper = std::make_unique<SiCaliWrapper::CalibrationWrapper>();
			m_pCalibrationWrapper->m_Ranger3Info = m_DeviceName;
			if (false == m_pCalibrationWrapper->initCalibrationFilter(calib_path_or_json_string,
				m_info.m_RI.cols,
				m_info.m_RI.rows,
				m_info.m_RI.aoiWidth,
				m_info.m_RI.aoiHeight,
				1.0,
				m_info.m_RI.aoiOffsetX,
				scaleZ,
				originZ,
				m_scatterSize,
				method,
				m_Param.m_RectWidth,
				missingData,
				m_Param.m_enableSSE,
				false,
				rectificationSpread,
				m_Param.m_runMode))
			{
				*m_log << CustomerLog::time() << "[" << m_DeviceName << "]" << "* Calibration in PC: off\n";
				m_pCalibrationWrapper = nullptr;
				return CAM_STATUS::ERROR_CALIBRATION_PATH;
			}
			
#if defined(__aarch64__) || defined(__arm__)
			* m_log << CustomerLog::time() << "[" << m_DeviceName << "]" << "* Calibration in PC [ARM]: on\n";
#else
			if (m_Param.m_ThreadNumber > 0)
			{
				m_pCalibrationWrapper->set_NumberOfThreads_jsonOnly_X64(m_Param.m_ThreadNumber);
				if (m_Param.m_ThreadNumber != m_pCalibrationWrapper->get_NumberOfThreads_jsonOnly_X64())
					*m_log << CustomerLog::time() << "[" << m_DeviceName << "]" << "* Warning: Calibration in PC: set number of working threads to " << m_Param.m_ThreadNumber << " failed! The default value is " << m_pCalibrationWrapper->get_NumberOfThreads_jsonOnly_X64() << "\n";
			}

			*m_log << CustomerLog::time() << "[" << m_DeviceName << "]" << "* Calibration in PC: number of working threads is " << m_pCalibrationWrapper->get_NumberOfThreads_jsonOnly_X64() << "\n";
			*m_log << CustomerLog::time() << "[" << m_DeviceName << "]" << "* Calibration in PC [X64]: on\n";

#endif
			*m_log << CustomerLog::time() << "[" << m_DeviceName << "]" << "* Calibration in PC: raw data width=" << m_info.m_RI.cols << ", rectification width=" << m_Param.m_RectWidth << "\n";
		}
#endif
	}


	return CAM_STATUS::All_OK;
}

void
Ranger3::_chunkDataSetting()
{
	GenApi::CBooleanPtr chunkModeActive = m_deviceNodeMap._GetNode("ChunkModeActive");
	m_ChunkModeActive = chunkModeActive->GetValue();

	if (m_ChunkModeActive)
	{
#ifdef __linux__
        m_chunkAdapter = std::make_unique<SiChunkAdapter>(m_pR3S->m_pTl, m_device->mDataStreamHandle);
#else
        m_chunkAdapter = std::make_unique<SiChunkAdapter>(m_pR3S->m_sTl, m_device->mDataStreamHandle);
#endif
		m_chunkAdapter->attachNodeMap(m_deviceNodeMap._Ptr);
		*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]: ChunkModeActive : Enable \n";
	}
	else
	{
		m_chunkAdapter = nullptr;
		*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]: ChunkModeActive : Disable \n";
	}
}

void
Ranger3::_reflectanceCheck()
{
	Str enable				= getParameterValue("ComponentEnable_ComponentSelector_Reflectance_RegionSelector_Scan3dExtraction1");
	m_isUsingReflectance	= enable == "1";
	*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" << "Reflectance: " << (m_isUsingReflectance ? "Enable\n" : "Disable\n");
}

void
Ranger3::_scatterCheck()
{
	Str enable				= getParameterValue("ComponentEnable_ComponentSelector_Scatter_RegionSelector_Scan3dExtraction1");
	m_isUsingScatter		= enable == "1";

	m_scatterSize = 8;
	if (m_isUsingScatter)
	{
		Str mono8			= getParameterValue("PixelFormat_ComponentSelector_Scatter_RegionSelector_Scan3dExtraction1");
		m_scatterSize		= (mono8 == "Mono8" ? 8 : 16);
	}

	*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" << "Scatter" << std::to_string(m_scatterSize) << ": " << (m_isUsingScatter ? "Enable\n" : "Disable\n");
}

bool
Ranger3::_inDeviceCalibrationCheck()
{
	auto originname = getParameterValue("Scan3dOutputMode_Scan3dExtractionSelector_Scan3dExtraction1");

	// VLD-727
	if (originname == "") // means "Scan3dOutputMode_Scan3dExtractionSelector_Scan3dExtraction1" is not existing.
	{
		*m_log << CustomerLog::time() << "[" << m_DeviceName << "]" << "[Warn] In-device Calibration Check, invalid! Maybe the FW is lower than 2.6. Don't worry!\n";
		m_calibration_inDevice = false;
		return false;
	}
	//Scan3dCoordinateScale_Scan3dCoordinateSelector_CoordinateA_Scan3dExtractionSelector_Scan3dExtraction1
	if (originname != "UncalibratedC")
	{
		GenApi::CEnumerationPtr selector = m_deviceNodeMap._GetNode("Scan3dExtractionSelector");
		*selector = "Scan3dExtraction1";
		selector = m_deviceNodeMap._GetNode("Scan3dCoordinateSelector");

		if (selector != nullptr)
		{
			m_calibration_inDevice = true;
			*selector = "CoordinateA";
			m_info.m_CI.scaleX = ((GenApi::CFloatPtr)m_deviceNodeMap._GetNode("Scan3dCoordinateScale"))->GetValue();
			m_info.m_CI.offsetX = ((GenApi::CFloatPtr)m_deviceNodeMap._GetNode("Scan3dCoordinateOffset"))->GetValue();
			m_info.m_CI.lower_bound_x = m_info.m_CI.offsetX;
			auto tmp = ((GenApi::CFloatPtr)m_deviceNodeMap._GetNode("Scan3dAxisMax"))->GetValue();
			m_info.m_CI.upper_bound_x = m_info.m_CI.offsetX + m_info.m_CI.scaleX * tmp;

			*selector = "CoordinateB";
			m_info.m_CI.scaleY = ((GenApi::CFloatPtr)m_deviceNodeMap._GetNode("Scan3dCoordinateScale"))->GetValue();
			m_info.m_CI.offsetY = ((GenApi::CFloatPtr)m_deviceNodeMap._GetNode("Scan3dCoordinateOffset"))->GetValue();

			*selector = "CoordinateC";
			m_info.m_CI.scaleZ = ((GenApi::CFloatPtr)m_deviceNodeMap._GetNode("Scan3dCoordinateScale"))->GetValue();
			m_info.m_CI.offsetZ = ((GenApi::CFloatPtr)m_deviceNodeMap._GetNode("Scan3dCoordinateOffset"))->GetValue();
			m_info.m_CI.lower_bound_r = m_info.m_CI.offsetZ;
			tmp = ((GenApi::CFloatPtr)m_deviceNodeMap._GetNode("Scan3dAxisMax"))->GetValue();
			m_info.m_CI.upper_bound_r = m_info.m_CI.offsetZ + m_info.m_CI.scaleZ * tmp;

			if (originname == "RectifiedC")
			{
				// If grabbing calibrated data, output image width is rectification width. -----
				m_Param.m_RectWidth = ((GenApi::CIntegerPtr)m_deviceNodeMap._GetNode("Scan3dRectificationWidth"))->GetValue();
				m_buffer16Size = m_Param.m_RectWidth * m_info.m_RI.rows * (m_isUsingReflectance ? 3 : 2);
				*m_log << CustomerLog::time() << "[" << m_DeviceName << "]" << "In device Calibratiion on! Scan3dOutputMode = " << originname << ", raw data width=" << m_info.m_RI.cols << ", rectification width=" << m_Param.m_RectWidth << "(Scan3dRectificationWidth)\n";
			}
			else
			{
				*m_log << CustomerLog::time() << "[" << m_DeviceName << "]" << "In device Calibratiion on! Scan3dOutputMode = " << originname << ", raw data width=" << m_info.m_RI.cols << "\n";
			}

#ifndef DISABLE_CAL_IN_PC
			m_pCalibrationWrapper = nullptr;
#endif
			return true;
		}
	}
	//m_Param.m_RectWidth = m_info.m_RI.aoiWidth; // todo[done]: If grabbing raw data, output image width is the same as aoiwidth. -----
	*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" << "In device Calibratiion off! Scan3dOutputMode = " << originname << "\n";
	//*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" << "In device Calibratiion off! Rectification = " << (m_Param.m_doRectify? "ON, output_data_width = rectification_width = " :"OFF, output_data_width = roi_width = ") << m_Param.m_RectWidth <<  + "\n";
	m_calibration_inDevice = false;
	return false;
}

CAM_STATUS
Ranger3::_connectCamera()
{
	if (m_pR3S			== nullptr)	return CAM_STATUS::ERROR_NULL_PTR_DEV;
	if (m_deviceHandle	== nullptr)	return CAM_STATUS::ERROR_NULL_DEV_HANDLE;

	if (m_status == CAM_STATUS::CAM_IS_DISCONNECTED)
	{
		m_device->openDevice	();
		m_device->openDataStream();

		if (m_device->mDataStreamHandle == GENTL_INVALID_HANDLE)
		{
			*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]: \n    Error: dataStreamHandle == GENTL_INVALID_HANDLE " << "\n";
			return CAM_STATUS::ERROR_NULL_DS_HANDLE;
		}
		
		// Get node map for data stream module to boost receiver thread priority
		m_device->createDataStreamNodeMap(*m_pR3S->getConsumer());
		_setThreadPriority(m_device->mDataStreamNodeMap);

		// Connect device node map to GenTL
		m_device->createDeviceNodeMap(*m_pR3S->getConsumer());
		m_deviceNodeMap = m_device->mDeviceNodeMap;
#ifndef __linux__
		m_deviceNode	= m_device->mDeviceNode;
#endif
	}
	*m_log << CustomerLog::time() << "["<<m_DeviceName<<"]: Ranger3 class: Connect Device. " << "\n";
	return CAM_STATUS::All_OK;
}

CAM_STATUS
Ranger3::_getImageData(ImgT & imgTable)
{
	GenTL::EVENT_NEW_BUFFER_DATA bufferData;
	size_t eventSize = sizeof(bufferData);

	try 
	{
        CC(m_pR3S->getTL(), m_pR3S->getTL()->EventGetData(m_device->mNewBufferEventHandle, &bufferData, &eventSize, m_Param.m_timeOut));
	}
	catch (...) { return CAM_STATUS::TIME_OUT; }

	auto  bufferId		= reinterpret_cast<intptr_t>(bufferData.pUserPointer);
	auto& bufferHandles	= m_device->getBufferHandles();
#ifdef __linux__
    _praseData(m_pR3S->m_pTl, bufferHandles, bufferId, imgTable);
#else
    _praseData(m_pR3S->m_sTl, bufferHandles, bufferId, imgTable);
#endif

    if (imgTable.isEmpty())
        return CAM_STATUS::ERROR_EMPTY_IMG;

	CC(m_pR3S->getTL(), m_pR3S->getTL()->DSQueueBuffer(m_device->mDataStreamHandle, bufferHandles[bufferId]));

	// mark last image
	imgTable.m_previousImageID = m_previousImageID;

	// check lost frames
	if (m_previousImageID + 1 == imgTable.get_ID())
	{
		m_previousImageID = imgTable.get_ID();
	}
	else
	{
		*m_log << CustomerLog::time() << "[" << m_DeviceName << "]: Warning: Image lost!! (Previous ID=" 
			<< m_previousImageID << ", Current ID=" << imgTable.get_ID() << "). Please check network settings!\n"
			<< "    1. Enable net-card jumbo-frames (mtu 9000k). \n"
			<< "    2. Each camera must be connected using a separate network card. \n"
			<< "    3. Do not use a switch to obtain data from multiple cameras. \n"
			<< "    4. Please check the network cable connection and check for strong electromagnetic interference. \n"
			<< "\n";
		return CAM_STATUS::WARN_IMAGE_LOST;
	}

	return CAM_STATUS::All_OK;
}

CAM_STATUS
Ranger3::_disconnectCamera()
{
	if (m_status != CAM_STATUS::CAM_IS_DISCONNECTED)
	{
		if (m_device->isAcquisitionRunning())
			_stopAcquisition();

		m_device->closeDataStream();
		m_device->closeDevice();
	}

	*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]: Ranger3 class: Disconnect Device! Version info: " << VER::__version__() << "\n";
	return CAM_STATUS::All_OK;
}

CAM_STATUS
Ranger3::_startAcquisition()
{
	if (!m_device->isAcquisitionRunning())
	{
		_reflectanceCheck();
		_scatterCheck();
		auto isOK = _grabSetting(m_Param.m_CaliPath, 
#ifndef DISABLE_CAL_IN_PC
								m_Param.m_RectMethod,
#endif
								m_Param.m_missingData, m_Param.m_doRectify , m_Param.m_RectSpread);

		if (isOK != CAM_STATUS::All_OK)
			return isOK;

		// Wrapper to be able to access chunk metadata
		_chunkDataSetting();

		// init buffer
		{
			// Setup a sufficient amount of buffers
			GenApi::CIntegerPtr payload = m_deviceNodeMap._GetNode("PayloadSize");
			m_payloadSize				= static_cast<size_t>(payload->GetValue());
			m_device->initializeBuffers(m_Param.m_buffersCount, m_payloadSize);
			size_t totalAllocatedMemory	= m_Param.m_buffersCount * m_payloadSize;

			*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]: Total memory used for buffers: " << (totalAllocatedMemory / 1024 / 1024) << " MB" 
				<< "\n        BufferCount = " << m_Param.m_buffersCount
				<< "\n        Init buffer done.\n";
		}

		// register event
		m_device->registerNewBufferEvent();
		*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]: Register New Buffer Event done. " << "\n";

		// Lock all parameters before starting
		GenApi::CIntegerPtr paramLock = m_deviceNodeMap._GetNode("TLParamsLocked");
		paramLock->SetValue(1);

		// Start acquisition
		auto tl = m_pR3S->getConsumer()->tl();
		CC(tl, tl->DSStartAcquisition(m_device->mDataStreamHandle, GenTL::ACQ_START_FLAGS_DEFAULT, GENTL_INFINITE));

		m_device->startAcquisition();

		*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]: Ranger3 class: Start Acquisition. Version info: " << VER::__version__() << "\n";
		m_info.m_id = 0;
		m_FastExtractor = nullptr;
	}

	if (m_device->isAcquisitionRunning())
	{
		Str DeviceScanType				= getParameterValue("DeviceScanType");
		m_IsOutputSensor				= DeviceScanType == "Areascan";
		GenApi::CEnumerationPtr regionSelector = m_deviceNodeMap._GetNode("RegionSelector");

		if (m_IsOutputSensor)
		{
            *m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]: Ranger3 class: Grab Sensor Image." << "\n";
			*regionSelector				= "Region0";
			GenApi::CIntegerPtr height	= m_deviceNodeMap._GetNode("Height");
			GenApi::CIntegerPtr width	= m_deviceNodeMap._GetNode("Width");
			m_info.m_SI.cols			= width	->GetValue();
			m_info.m_SI.rows			= height->GetValue();

			*regionSelector				= "Region1";
			m_info.m_RI.aoiOffsetX		= ((GenApi::CIntegerPtr)m_deviceNodeMap._GetNode("OffsetX"))->GetValue();
			m_info.m_RI.aoiOffsetY		= ((GenApi::CIntegerPtr)m_deviceNodeMap._GetNode("OffsetY"))->GetValue();
			m_info.m_RI.aoiHeight		= ((GenApi::CIntegerPtr)m_deviceNodeMap._GetNode("Height"))	->GetValue();
			m_info.m_RI.aoiWidth		= ((GenApi::CIntegerPtr)m_deviceNodeMap._GetNode("Width"))	->GetValue();
		}
		else
		{
            *m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]: Ranger3 class: Grab Range Image." << "\n";
		
			*regionSelector = "Scan3dExtraction1";
			GenApi::CIntegerPtr width = m_deviceNodeMap._GetNode("Width");
			m_info.m_RI.cols = width->GetValue();

			if (m_calibration_inDevice==true && m_calibration_inDevice_CalibratedAC==false)
			{
				m_info.m_RI.cols = atoi(getParameterValue("Scan3dRectificationWidth_Scan3dExtractionSelector_Scan3dExtraction1").c_str());
			}

			GenApi::CIntegerPtr height = m_deviceNodeMap._GetNode("Height");
			m_info.m_RI.rows = height->GetValue();
		}

		m_regionLookup = std::make_shared<EnumSelectorEntries>(m_deviceNodeMap, "RegionSelector");
		m_componentLookup = std::make_shared<EnumSelectorEntries>(m_deviceNodeMap, "ComponentSelector");

		return CAM_STATUS::All_OK;
	}
	else
		return CAM_STATUS::ERROR_START_ACQUISITION;
}

CAM_STATUS
Ranger3::_stopAcquisition()
{
	auto tl = m_pR3S->getConsumer()->tl();
	if (m_device->isAcquisitionRunning())
	{
		m_device->stopAcquisition();
		CC(tl, tl->DSStopAcquisition(m_device->mDataStreamHandle, GenTL::ACQ_STOP_FLAGS_KILL));
		GenApi::CIntegerPtr paramLock = m_deviceNodeMap._GetNode("TLParamsLocked");
		paramLock->SetValue(0);
	}
	else
    {
        *m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]: Ranger3 class: Stopped Acquisition. Grab timeout(ms)=" << m_Param.m_timeOut << ". Version info: " << VER::__version__() << "\n"; 
		return CAM_STATUS::All_OK;
    }

	if (!m_device->isAcquisitionRunning())
	{
		CC(tl, tl->DSFlushQueue(m_device->mDataStreamHandle, GenTL::ACQ_QUEUE_ALL_DISCARD));
		m_device->unregisterNewBufferEvent();

		if (m_ChunkModeActive)
		{
			m_chunkAdapter->detachNodeMap();
			m_chunkAdapter.reset();
			m_ChunkModeActive = false;
		}

		m_device->teardownBuffers();

        *m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]: Ranger3 class: Stopped Acquisition. Grab timeout(ms)=" << m_Param.m_timeOut << ". Version info: " << VER::__version__() << "\n";
		return CAM_STATUS::All_OK;
	}
	else
		return CAM_STATUS::ERROR_STOP_ACQUISITION;
}

CAM_STATUS
Ranger3::_scanDeviceParameter(cStr path)
{
	if (m_device->isAcquisitionRunning())
		return CAM_STATUS::ERROR_CAM_IS_STARTED;

	if (m_Param.ScanDeviceParameters(m_deviceNodeMap._Ptr, path))
		return CAM_STATUS::All_OK;

	return CAM_STATUS::ERROR_SCAN_PARAMETERS;
}

void 
Ranger3::_ConvertUint16ToFloat(uint8_t * p16In, uint8_t * pFloatOut, const size_t & s, const float & offerZ, const float & scaleZ) const
{
	auto p16 = (uint16_t*)p16In;
	auto pFloat = (float*)pFloatOut;
	if (m_convCalibratedDataToFloat_enableOMP)
	{
#pragma omp parallel for num_threads(4)
		for (int i = 0; i < s; ++i)
		{
			pFloat[i] = p16[i] == 0 ? MISSING_DATA : p16[i] * scaleZ + offerZ;
		}
	}
	else
	{
		for (int i = 0; i < s; ++i)
		{
			pFloat[i] = p16[i] == 0 ? MISSING_DATA : p16[i] * scaleZ + offerZ;
		}
	}
}


#ifdef CALLBACK_NEW

void
Ranger3::_callback_run()
{
	while (true)
	{
		//{	
		//std::unique_lock<std::mutex> lock(m_callback_require_stop_locker);
		if (m_callback_require_stop)
		{
			//std::cout << "***** _callback_run 1, m_grab_interval=" << m_grab_interval << std::endl;
			break; 
		}
		//}
	
		SickCam::ImgT img; 
		if (CAM_STATUS::All_OK == getImageData(img) && !img.isEmpty())
		{
			if (m_callback_require_stop)
			{
				//std::cout << "***** _callback_run 2" << std::endl;
				break;
			}

			{   std::unique_lock<std::mutex> lock(m_callback_is_finish_copy_locker);
				m_callback_is_finish_copy = false;	
			}

//#ifdef CALLBACK_THREAD_POOL
			if (m_callbackPool != nullptr)
			{
				std::string debugStr = "** thread pool ** get " + std::to_string(img.get_ID());
				std::cout << debugStr << std::endl;

				auto lambdaFunc = [this, &img]() -> void { _callback_run_on_grabbed(&img); }; 
				m_callbackPool->enqueue<void>(lambdaFunc);

			}
			else
//#endif // CALLBACK_THREAD_POOL
			{
				auto _thread = std::make_shared<std::thread>(&Ranger3::_callback_run_on_grabbed, this, &img);
				_thread->detach();
			}
			

			while (true){
				{	std::unique_lock<std::mutex> lock(m_callback_is_finish_copy_locker);
					if (m_callback_is_finish_copy)
						break;
				}
                __sleep1MS(m_grab_interval);
			}

		}
		else
			__sleep1MS(m_grab_interval);
	}

	m_callback_is_on = false;
}

void
Ranger3::_callback_run_on_grabbed(ImgT * _img)
{
	//m_callbackDebug.insert({ _img->get_ID(), callbackInfo(
	//	GetCurrentThreadId(), 
	//	m_callback_is_on, 
	//	m_callback_require_stop,
	//	SickCam::CustomerLog::time()
	//) });

	SickCam::ImgT img = (*_img);
	{
		std::unique_lock<std::mutex> lock(m_callback_is_finish_copy_locker);
		m_callback_is_finish_copy = true;
	}

	auto _thread = std::make_shared<std::thread>(m_callBack_function, &img, m_callBack_inputs);
	_thread->join(); // wait callback done, and free the img.

	//m_callbackDebug.erase(_img->get_ID());
}

void
Ranger3::_check_HeartBeats_run				()
{
	
	std::string msg = "Losing heartbeats from device: \n" + _getDeviceInfo_when_lost();
    m_on_lost_mac = getMac();
	*m_log << CustomerLog::time() <<"["<<m_DeviceName<<"]" << "_check_HeartBeats_run() on! " << m_DeviceIP << ", " << m_on_lost_mac << "\n";

    m_heartbeat_count = 0;
    while (m_heartbeat_is_on==1)
    {
        __sleep1MS(m_heartbeat_interval);
		if (m_heartbeat_is_on == 0)
		{
			m_heartbeat_is_on = 2;
			break;
		}

        try {
            Str value("");
            m_Param.getParameter(m_deviceNodeMap, "DeviceTemperature", value);
			//std::cout << "[" << m_DeviceName << "]" << "Heartbeat received! Heartbeat count=" << ++m_heartbeat_count << "\n";
			*m_log << CustomerLog::time() << "[" << m_DeviceName << "]: Heartbeat received! Heartbeat count=" << ++m_heartbeat_count << "\n";
        }
        catch (...) {
            m_status = CAM_STATUS::ERROR_CAM_IS_LOST;
			*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::_check_HeartBeats_run]: start callback, status = " << CAM_STATUS_str(m_status) << "\n";

            // release variables if device is started
#ifdef CALLBACK_NEW
            if (m_callback_is_on)
            {
                {
                    std::unique_lock<std::mutex> lock(m_callback_require_stop_locker);
                    m_callback_require_stop = true;
                }
				while (m_callback_is_on)
				{
					*m_log << CustomerLog::time() << "[" << m_DeviceName << "][Ranger3::_check_HeartBeats_run]: try to stop grabbing callback." << "\n";
					__sleep1MS(10);
				}
            }
#endif // CALLBACK_NEW
			_freeDevice();

			*m_log << CustomerLog::time() << "[" << m_DeviceName << "]: _check_HeartBeats_run() off! " << m_DeviceIP << ", " << m_on_lost_mac << "\n";
			*m_log << CustomerLog::time() << "[" << m_DeviceName << "]: Heartbeats lost! To call processing function defined by user. \n";
			auto _thread = std::make_shared<std::thread>(m_on_lost_function, &m_DeviceName, &m_DeviceIP, &m_on_lost_mac, &msg, m_on_lost_inputs);
			_thread->join();
            return;
        }
    }

}

Str 
Ranger3::_getDeviceInfo_when_lost			() const
{
	std::stringstream ss;
	ss
		<< "Device Name                =" << getDeviceName()
		<< "\nDevice IP                   =" << m_DeviceIP
		<< "\nDevice SubnetMask           =" << getSubNet()
		<< "\nDevice MAC                  =" << getMac()
		<< "\nDevice DeviceModelName      =" << m_DeviceModelName
		<< "\nDevice DeviceVersion        =" << m_DeviceVersion
		<< "\nDevice DeviceFirmwareVersion=" << m_DeviceFirmwareVersion
		<< "\nDevice DeviceSerialNumber   =" << m_DeviceSerialNumber
		; 
	return ss.str();
}

#endif // CALLBACK_NEW


#pragma endregion

////////////////////////////////////////////////////////////////////////////////


}
