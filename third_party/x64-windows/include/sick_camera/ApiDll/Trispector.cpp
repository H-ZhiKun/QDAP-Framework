#include "Trispector.h"
#include <omp.h>



namespace SickCam 
{

Trispector::Trispector(const SPtr<TrispectorShared>& TS, cStr & sn)
	: 
	m_pR3S(nullptr),
	m_device(nullptr),
	m_consumer(nullptr),
	m_pParts(nullptr),
	m_ChunkModeActive(false),
	m_isUsingReflectance(false),
	m_IsOutputSensor(false),
	m_buffer16Size(0),
	m_payloadSize(0),
	m_DeviceName(""),
	m_status(CAM_STATUS::ERROR_NULL_PTR_DEV),
	m_canStop(true),
	m_DeviceSN(sn)
#ifdef CALLBACK_NEW
	,
	m_callback_is_on(false),
	m_callback_require_stop(false)
#endif
{
	m_pR3S = TS;
	m_log = m_pR3S->m_log;
	Str deviceStr = m_DeviceSN;

	auto diviceList = m_pR3S->getConDevListSN();

	if (diviceList.count(deviceStr) == 1)
	{
		m_device		= diviceList.at(deviceStr);
		m_deviceHandle	= m_device->mDeviceHandle;
		m_deviceNodeMap = m_device->mDeviceNodeMap;
		m_DeviceName	= m_device->mDeviceName;
		m_consumer		= m_pR3S->getConsumer();
		m_status		= CAM_STATUS::CAM_IS_DISCONNECTED;
		m_camType		= CamType::_TRISPECTOR;

		*m_log << CustomerLog::time() << "Init " << m_DeviceName.c_str() << " by "	<< deviceStr << "), OK!" << endl;
	}
	else
		*m_log << CustomerLog::time() << "Faile to init Ranger3 by " << deviceStr << "! \n\nThe recommended IP format is \" 192.168.0.1\";\nThe recommended MAC format is \"00:06:77:0b:d6:e6\"." << endl;


}

Trispector::~Trispector()
{
	if (m_device != nullptr)
	{
		m_device->closeDataStream();
		m_device->closeDevice();
	}
}

EXPORT_TO_DLL CAM_STATUS 
Trispector::isReady()
{
	return (getStatus() == CAM_STATUS::ERROR_NULL_PTR_DEV) ? CAM_STATUS::ERROR_NULL_PTR_DEV : CAM_STATUS::All_OK;
}

EXPORT_TO_DLL CAM_STATUS
Trispector::connectCamera()
{
	if (getStatus() == CAM_STATUS::CAM_IS_CONNECTED)	return CAM_STATUS::All_OK;
	if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
	if (getStatus() == CAM_STATUS::CAM_IS_STOPPED)		return CAM_STATUS::All_OK;

	std::stringstream ss;
	ss	<< m_DeviceName.c_str() 
		<< " : connectCamera(_scaleY="	<< m_Param.m_scaleY
		<< ", \r\npara_csv_path="		<< m_Param.m_ParaPath
		<< ", \r\ncalib_xml_path="		<< m_Param.m_CaliPath
		<< ", \r\ntmp_path="			<< m_Param.m_TmpPath
#ifndef DISABLE_CAL_IN_PC
		<< ", \r\nrect method="			<< m_Param.m_RectMethod
#endif
		<< ", \r\nrect spread="			<< m_Param.m_RectSpread
		<< ", \r\nmissingData="			<< m_Param.m_missingData
		<< ", \r\n_doRectfy="			<< m_Param.m_doRectify
		<< "\r\n";

	// connect
	auto e = _connectCamera();
	
	if (e != CAM_STATUS::All_OK) { m_status = CAM_STATUS::CAM_IS_DISCONNECTED; return e; }
	m_status = CAM_STATUS::CAM_IS_CONNECTED;	ss << "Connect OK!\n";
	
	return e;
}

EXPORT_TO_DLL CAM_STATUS
Trispector::startCamera()
{
	if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::All_OK;
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;
	if (getStatus() == CAM_STATUS::ERROR_NULL_PTR_DEV)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;

	auto e = _startAcquisition();
	if (e == CAM_STATUS::All_OK) m_status = CAM_STATUS::CAM_IS_STARTED; 
	return e; 
}

#ifdef CALLBACK_NEW
EXPORT_TO_DLL CAM_STATUS Trispector::startCamera(CallbackEvent pCallback, void * any)
{
	auto err = startCamera();
	if (err == CAM_STATUS::All_OK)
	{
		m_callBack_function = pCallback;
		m_callBack_inputs = any;
		//m_callBack_inputs_size = 0;
		m_callback_is_on = true;
		std::unique_lock<std::mutex> lock(m_callback_require_stop_locker);
		m_callback_require_stop = false;

		auto _thread = std::make_shared<std::thread>(&Trispector::_callback_run, this);
		_thread->detach();
	}
	return err;
}
#endif

EXPORT_TO_DLL CAM_STATUS
Trispector::stopCamera()
{
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
		while (m_callback_is_on) Sleep(0);
	}
	else
#endif
	{
		while (!m_canStop); // thread grabbing mode
	}

	auto e = _stopAcquisition();
	if (e == CAM_STATUS::All_OK) m_status = CAM_STATUS::CAM_IS_STOPPED;
	return e; 
}

EXPORT_TO_DLL CAM_STATUS
Trispector::disconnectCamera()
{
	if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::All_OK;
	if (getStatus() == CAM_STATUS::CAM_IS_CONNECTED || getStatus() == CAM_STATUS::CAM_IS_STOPPED)
	{
		auto e = _disconnectCamera();
		if (e == CAM_STATUS::All_OK) m_status = CAM_STATUS::CAM_IS_DISCONNECTED;
		return e;
	}
	return CAM_STATUS::UNKNOWN;
}

EXPORT_TO_DLL CAM_STATUS
Trispector::getImageData(ImgT & imgTable)
{
	if (getStatus() == CAM_STATUS::CAM_IS_CONNECTED)	return CAM_STATUS::ERROR_CAM_IS_CONNECTED;
	if (getStatus() == CAM_STATUS::CAM_IS_STOPPED)		return CAM_STATUS::ERROR_CAM_IS_STOPPED;
	if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;

	m_canStop = false;
	auto res = _getImageData(imgTable);
	m_canStop = true;
	return res;
}

EXPORT_TO_DLL bool 
Trispector::socketInit(const std::string& ip, const int & port)
{
	try
	{
		WORD sockVersion = MAKEWORD(2, 2);
		WSADATA data;
		if (WSAStartup(sockVersion, &data) != 0)
		{
			std::cerr << "socket init error : WSA Startup failed!" << std::endl;
			return false;
		}

		m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_socket == INVALID_SOCKET)
		{
			std::cerr << "invalid socket!" << std::endl;
			return false;
		}

		// socket connect
		sockaddr_in serAddr;
		serAddr.sin_family = AF_INET;
		serAddr.sin_port = htons(USHORT(port));
		serAddr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
		if (connect(m_socket, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
		{
			std::cerr << "socket connect error! Please check current camera ip :" << ip << " and port : " << port << ".";
			return false;
		}


	}
	catch (const std::exception& e)
	{
		std::stringstream ss;
		std::cerr << "socket init error : invalid socket!\n\t"
			<< e.what() << "\nPlease check the IP and ports." << std::endl;
		return false;
	}

	return true;
}

EXPORT_TO_DLL std::string 
Trispector::socketSendCommand(const std::string & command)
{
	std::string sendstr = "\x2" + command + "\x3";
	int sta = send(m_socket, sendstr.c_str(), strlen(sendstr.c_str()), 0);
	if (sta == -1)
	{
		std::cerr << "socket " << m_DeviceSN << " send error! commamd:" + command + ". \n" << std::endl;
		return std::string();
	}

	char recData[255];
	std::string result(recData);
	int ret = recv(m_socket, recData, 255, 0);
	if (ret > 0)
	{
		recData[ret] = 0x00;
		std::string tmp(recData);
		if (tmp.length() < 2)
		{
			std::cerr << "socket " << m_DeviceSN << " receive error! receive:" + tmp + ". \n" << std::endl;
			return std::string();
		}
		else
			result = tmp.substr(1, tmp.length() - 2);
	}

	return result;
}

void
Trispector::_setThreadPriority(GNodeMap &dataStreamNodeMap)
{
	GenApi::CIntegerPtr threadPriority
		= dataStreamNodeMap._GetNode("StreamThreadPriority");
	GenApi::CCommandPtr setStreamPrio
		= dataStreamNodeMap._GetNode("StreamThreadApplyPriority");
	if (threadPriority.IsValid())
	{
		// Default priority is "HIGHEST" = 2, set to TIME CRITICAL = 15
		threadPriority->SetValue(15);
	}
	else
	{
		*m_log << CustomerLog::time() << "WARNING: Could not set thread priority!" << endl;
	}
	if (setStreamPrio.IsValid())
	{
		setStreamPrio->Execute();
	}
	else
	{
		*m_log << CustomerLog::time() << "WARNING: Could not set thread priority!" << endl;
	}
}

CAM_STATUS
Trispector::_connectCamera()
{
	if (m_pR3S == nullptr)			return CAM_STATUS::ERROR_NULL_PTR_DEV;
	if (m_deviceHandle == nullptr)	return CAM_STATUS::ERROR_NULL_DEV_HANDLE;

	if (m_status == CAM_STATUS::CAM_IS_DISCONNECTED)
	{
		m_device->openDevice();
		m_device->openDataStream();

		if (m_device->mDataStreamHandle == GENTL_INVALID_HANDLE)
		{
			*m_log << CustomerLog::time() << "Error: dataStreamHandle == GENTL_INVALID_HANDLE " << endl;
			return CAM_STATUS::ERROR_NULL_DS_HANDLE;
		}

		// Get node map for data stream module to boost receiver thread priority
		m_device->createDataStreamNodeMap(*m_pR3S->getConsumer());
		_setThreadPriority(m_device->mDataStreamNodeMap);

		// Connect device node map to GenTL
		m_device->createDeviceNodeMap(*m_pR3S->getConsumer());
		m_deviceNodeMap = m_device->mDeviceNodeMap;

	}

	return CAM_STATUS::All_OK;
}

CAM_STATUS
Trispector::_getImageData(ImgT & imgTable)
{
	auto tl = m_pR3S->getConsumer()->tl();
	// Wait for buffer to be received
	GenTL::EVENT_NEW_BUFFER_DATA bufferData;
	size_t eventSize = sizeof(bufferData);

	try {
		CC(tl, tl->EventGetData(m_device->mNewBufferEventHandle, &bufferData, &eventSize, m_Param.m_timeOut));
	}
	catch (...) { return CAM_STATUS::TIME_OUT; }
	
	int bufferId = reinterpret_cast<intptr_t>(bufferData.pUserPointer); 
	//std::cout << "Trispector::_getImageData, get bufferid=" << bufferId << std::endl; // todo : debbug ---------
	std::vector<GenTL::BUFFER_HANDLE> bufferHandles = m_device->getBufferHandles();
	//std::cout << "Trispector::_getImageData, get bufferHandles size=" << bufferHandles.size() << ", address=" << bufferHandles[0] << std::endl; // todo : debbug ---------

	uint8_t* buffer = static_cast<uint8_t*>(m_device->getBufferData()[bufferId]);
	const size_t dataSize = m_info.m_RI.cols*m_info.m_RI.rows;

	//// raw data
	uint16_t *	rawRangePtr = (uint16_t*)(buffer + sizeof(uint8_t)*dataSize);
	uint8_t *	calReflePtr = buffer;


	//// calibration data
	float32_t* pInfo = (float32_t*)(buffer + (m_payloadSize - 140));
	float32_t scaleX = *(pInfo + 3);
	float32_t scaleY = *(pInfo + 4);
	float32_t scaleZ = *(pInfo + 5);
	float32_t offsetX = *(pInfo + 6);
	float32_t offsetY = 0.0;
	float32_t offsetZ = *(pInfo + 8);
	float32_t minZ = 0.0;
	float32_t maxZ = *(pInfo + 11);
	uint32_t frameId = *(uint32_t*)(pInfo + 20);

	imgTable.setCaliInfo(
		m_info.m_RI.cols,
		m_info.m_RI.rows,
		static_cast<double>(offsetX),
		0.0, 0.0,
		static_cast<double>(scaleX),
		static_cast<double>(scaleY),1.0,
		static_cast<double>(offsetX),
		static_cast<double>(offsetX + scaleX*m_info.m_RI.cols),
		static_cast<double>(minZ),
		static_cast<double>(maxZ),

		scaleX,
		offsetX,
		scaleZ,
		offsetZ
	);

	float *		calRangePtr = new float[dataSize];

#pragma omp parallel for
	for (int i=0; i< dataSize; ++i)
	{
		calRangePtr[i] = scaleZ * rawRangePtr[i];
	}

	imgTable.insertDataCopy(SickCam::RAN_CAL, (uint8_t *)calRangePtr, frameId);
	imgTable.insertDataCopy(SickCam::REF_CAL, (uint8_t *)calReflePtr, frameId);

	delete[] calRangePtr;
	// Re-queue buffer
	CC(tl, tl->DSQueueBuffer(m_device->mDataStreamHandle, bufferHandles[bufferId]));

	return CAM_STATUS::All_OK;
}

CAM_STATUS
Trispector::_disconnectCamera()
{
	if (m_status != CAM_STATUS::CAM_IS_DISCONNECTED)
	{
		if (m_device->isAcquisitionRunning())
			_stopAcquisition();
		
		m_device->closeDataStream();
		m_device->closeDevice();
	}

	*m_log << CustomerLog::time() << "Device disconnected!" << endl;
	return CAM_STATUS::All_OK;
}

CAM_STATUS
Trispector::_startAcquisition()
{
	if (!m_device->isAcquisitionRunning())
	{
		// init buffer
		{
			// Setup a sufficient amount of buffers
			GenApi::CIntegerPtr payload = m_deviceNodeMap._GetNode("PayloadSize");
			m_payloadSize = static_cast<size_t>(payload->GetValue());
			m_device->initializeBuffers(m_Param.m_buffersCount, m_payloadSize);
			size_t totalAllocatedMemory = m_Param.m_buffersCount * m_payloadSize;

			*m_log << CustomerLog::time() << "Total memory used for buffers: " << (totalAllocatedMemory / 1024 / 1024) << " MB\n" << "Init buffer done. \n" << endl;
		}

		// register event
		m_device->registerNewBufferEvent();
		*m_log << CustomerLog::time() << "register New Buffer Event done. " << endl;

		// Lock all parameters before starting
		GenApi::CIntegerPtr paramLock = m_deviceNodeMap._GetNode("TLParamsLocked");
		paramLock->SetValue(1);

		// Start acquisition
		auto tl = m_pR3S->getConsumer()->tl();
		CC(tl, tl->DSStartAcquisition(m_device->mDataStreamHandle, GenTL::ACQ_START_FLAGS_DEFAULT, GENTL_INFINITE));
		m_device->startAcquisition();
		*m_log << CustomerLog::time() << "start Acquisition. " << endl;
		m_info.m_id = 0;
	}

	if (m_device->isAcquisitionRunning())
	{
		GenApi::CIntegerPtr height	= m_deviceNodeMap._GetNode("Height");
		GenApi::CIntegerPtr width	= m_deviceNodeMap._GetNode("Width");
		m_info.m_RI.cols			= m_info.m_RI.cols = width->GetValue();
		m_info.m_RI.rows			= m_info.m_RI.rows = height->GetValue();
		return CAM_STATUS::All_OK;
	}
	else
		return CAM_STATUS::ERROR_START_ACQUISITION;
}

CAM_STATUS
Trispector::_stopAcquisition()
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
		return CAM_STATUS::All_OK;

	if (!m_device->isAcquisitionRunning())
	{
		CC(tl, tl->DSFlushQueue(m_device->mDataStreamHandle, GenTL::ACQ_QUEUE_ALL_DISCARD));
		m_device->unregisterNewBufferEvent();
		m_device->teardownBuffers();
		return CAM_STATUS::All_OK;
	}
	else
		return CAM_STATUS::ERROR_STOP_ACQUISITION;
}


#ifdef CALLBACK_NEW

void
Trispector::_callback_run()
{
	while (true)
	{
		{
			std::unique_lock<std::mutex> lock(m_callback_require_stop_locker);
			if (m_callback_require_stop)
				break;
		}

		SickCam::ImgT img;
		try
		{
			if (CAM_STATUS::All_OK == getImageData(img) && !img.isEmpty())
			{
				{
					std::unique_lock<std::mutex> lock(m_callback_is_finish_copy_locker);
					m_callback_is_finish_copy = false;
				}
				auto _thread = std::make_shared<std::thread>(&Trispector::_callback_run_on_grabbed, this, &img);
				_thread->detach();

				while (true)
				{
					{
						std::unique_lock<std::mutex> lock(m_callback_is_finish_copy_locker);
						if (m_callback_is_finish_copy)
							break;
					}
					Sleep(0);
				}

			}
			else
				continue;
		}
		catch (...)
		{
			continue;
		}

	}
	m_callback_is_on = false;

}

void 
Trispector::_callback_run_on_grabbed(ImgT * _img)
{
	SickCam::ImgT img = (*_img);
	{
		std::unique_lock<std::mutex> lock(m_callback_is_finish_copy_locker);
		m_callback_is_finish_copy = true;
	}

	auto _thread = std::make_shared<std::thread>(m_callBack_function, &img, m_callBack_inputs);
	_thread->join(); // wait callback done, and free the img.

}

#endif



}
