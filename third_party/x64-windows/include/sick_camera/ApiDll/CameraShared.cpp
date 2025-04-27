#include "CameraShared.h"
#ifdef __linux__
#include <sched.h>
#include <errno.h>
#endif

namespace SickCam
{

//////////////// CameraShared function definitions //////////////////

bool 
CameraShared::is_Inited = false; // for Singleton 

EXPORT_TO_DLL 
CameraShared::CameraShared(	cStr& logPath,
							cStr& ctiPath,
							const bool enShow,
							const bool enWrite)
	: m_isCtiFound(false)
	, m_isDevFound(false)
	, m_enableLogOutput(enShow)
	, m_enableLogWriteToFile(enWrite)
{
	if (is_Inited) 
	{
		std::cout << CustomerLog::time() << "[CameraShared()]: Had inited shared env." << "\n";
		return;
	}
	
	is_Inited = true;
	_setProcessPriority();

	// init log

	//auto logFilePath = logPath;
	//m_enableLogWriteToFile = enWrite;
	//if(!enWrite)
	//	logFilePath = "";
	is_Inited = _initLog(enWrite? logPath : "");

	// load cti
	if (CAM_STATUS::All_OK == loadCtiFile(ctiPath))
	{
		int numberDeviceNewFound = 0;
		_scan_Device(numberDeviceNewFound);
	}
	else
		is_Inited = false;

}

EXPORT_TO_DLL
CameraShared::~CameraShared()
{
	_clearR3S();
#ifdef __linux__
    if (m_pTl != nullptr)
	{
    	m_pconsumer->close();
        m_pTl = nullptr;
	}
#endif
#ifdef _WIN32
    if (m_sTl != nullptr)
	{
		CC(m_sTl, m_sTl->TLClose(m_tlHandle));
		CC(m_sTl, m_sTl->GCCloseLib());
		m_sTl = nullptr;
	}
#endif
}

EXPORT_TO_DLL const deviceList	
CameraShared::getConDevList(bool available_device_only)	const 
{
	deviceList avaiDevices;
	for (auto sub : m_connectedDevices)
	{
		if (sub.second->mDeviceName == "")
			continue;

		if (available_device_only)
		{
			if (!sub.second->mIsOpenDataStream)
				avaiDevices.insert(sub);
			else
				continue;
		}
		else
			avaiDevices.insert(sub);
		*m_log << CustomerLog::time() << "[CameraShared::getConDevList]: Select: " << sub.second->mDeviceName << " - " << sub.second->getIp() << " - " << sub.second->getMac() << "\n";
	}
	return avaiDevices;
}

EXPORT_TO_DLL const deviceList	
CameraShared::getConDevListIP(bool available_device_only)	const 
{ 
	deviceList avaiDevices = {};
	for (auto sub : m_connectedDevices)
	{
		if (sub.second->mDeviceName == "")
            continue;
        

		if (available_device_only)
		{
			if (!sub.second->mIsOpenDataStream)
				avaiDevices.insert({ sub.second->getIp(), sub.second });
			else
				continue;
		}
		else
			avaiDevices.insert({ sub.second->getIp(), sub.second });
		
		*m_log << CustomerLog::time() << "[CameraShared::getConDevListIP]: Select: " << sub.second->mDeviceName << " - " << sub.second->getIp() << " - " << sub.second->getMac() << "\n";
	}
	return avaiDevices;
}

EXPORT_TO_DLL const deviceList	
CameraShared::getConDevListMAC(bool available_device_only)	const
{
	deviceList avaiDevices = {};
	for (auto sub : m_connectedDevices)
	{
		if (sub.second->mDeviceName == "")
			continue;

		if (available_device_only)
		{
			if (!sub.second->mIsOpenDataStream)
				avaiDevices.insert({ sub.second->getMac(), sub.second });
			else
				continue;
		}
		else
			avaiDevices.insert({ sub.second->getMac(), sub.second });
		*m_log << CustomerLog::time() << "[CameraShared::getConDevListMAC]: Select: " << sub.second->mDeviceName << " - " << sub.second->getIp() << " - " << sub.second->getMac() << "\n";
	}
	return avaiDevices;
}

EXPORT_TO_DLL const deviceList	
CameraShared::getConDevListSN(bool available_device_only)	const
{ 
	deviceList avaiDevices = {};
	for (auto sub : m_connectedDevices)
	{
		if (sub.second->mDeviceName == "")
			continue;

		if (available_device_only)
		{
			if (!sub.second->mIsOpenDataStream)
			{
				//avaiDevices.insert({ sub.second->mDeviceName.substr(21, 8), sub.second }); // trispector : SICKTriSpectorTL_DEV_18140009
				avaiDevices.insert({ sub.second->getSN(), sub.second}); // trispector : SICKTriSpectorTL_DEV_18140009
			}
			else
				continue;
		}
		else
		{
			//avaiDevices.insert({ sub.second->mDeviceName.substr(21, 8), sub.second });
			avaiDevices.insert({ sub.second->getSN(), sub.second });
		}
		*m_log << CustomerLog::time() << "[CameraShared::getConDevListSN]: Select: " << sub.second->mDeviceName << "\n";
	}
	return avaiDevices;
}

EXPORT_TO_DLL CAM_STATUS 
CameraShared::scanDevice(int& numberDeviceNewFound)
{
	try {
		return _scan_Device(numberDeviceNewFound);
	}
	catch(std::exception& e)
	{
		*m_log << CustomerLog::time() << "Error: [CameraShared::scanDevice]: \n        - " << e.what() << "\n";
		return CAM_STATUS::UNKNOWN;
	}
	catch (...)
	{
		*m_log << CustomerLog::time() << "Error: [CameraShared::scanDevice]: \n        - " << "Uncatched error, please try again later!" << "\n";
		return CAM_STATUS::UNKNOWN;
	}
}

EXPORT_TO_DLL CAM_STATUS 
CameraShared::scanDevice()
{
	int numberDeviceNewFound;
	return scanDevice(numberDeviceNewFound);
}

EXPORT_TO_DLL CAM_STATUS
CameraShared::loadCtiFile(cStr & ctiPath)
{
	Str ctiFile = ctiPath;
	if (ctiFile.empty())
	{
		if (ctiFile.empty())
		{
			*m_log << CustomerLog::time() << "Error: [CameraShared::loadCtiFile]: cti file no exist!\n";
			return CAM_STATUS::ERROR_CTI_NOT_FOUND;
		}
	}

	if(!_fileExists(ctiFile))
	{
		*m_log << CustomerLog::time() << "Error: [CameraShared::loadCtiFile]: cti file no exist! please check: " << ctiFile << "\n";
		return CAM_STATUS::ERROR_CTI_NOT_FOUND;
	}
	*m_log << CustomerLog::time() << "[CameraShared::loadCtiFile]: cti file exist! The path is : " << ctiFile << "\n";
	
	try
	{
#ifdef __linux__
		m_pconsumer = std::make_unique<SiConsumer>(ctiFile);
		m_tlHandle = m_pconsumer->open();
		m_pTl = m_pconsumer->tl();


        // m_sTl will be used in Ranger3, but wil lead to double-free ------
        //m_sTl = std::make_shared<GenTLApi>(m_pTl->mModule);
        m_pTl = m_pconsumer->tl();
        //m_sTl.reset(_Tl);// todo: will crash ----

        // will crash
        ///m_sTl = std::make_shared<GenTLApi>(m_pTl);
        ///m_sTl.reset(m_pTl);
        ///auto m_Tl = m_pTl;

#endif
#ifdef _WIN32
        m_pconsumer = SiConsumer::load(ctiFile);
		m_sTl = m_pconsumer->tl();
		m_tlHandle = _TL_HANDLE_open(m_sTl);
		m_pTl = m_sTl.get();
		auto m_Tl = m_sTl;
#endif
		if (m_pTl != GENTL_INVALID_HANDLE)
		{
			*m_log << CustomerLog::time() << "[CameraShared::loadCtiFile]: common::Consumer open ok!\n";
			m_isCtiFound = true;
			return CAM_STATUS::All_OK;
		}
		else
			return CAM_STATUS::ERROR_OPEN_CONSUMER;
	}
	catch (std::exception& e)
	{
		*m_log << CustomerLog::time() << "Error:[CameraShared::loadCtiFile]: common::Consumer open failed! Ecode: "<< e.what() << "\n";
		return CAM_STATUS::ERROR_OPEN_CONSUMER;
	}
	catch (...)
	{
		return CAM_STATUS::ERROR_OPEN_CONSUMER;
	}
}

CAM_STATUS
CameraShared::_scan_Device(int& numberDeviceNewFound)
{
#ifdef CALLBACK_NEW
	std::unique_lock<std::mutex> lock(m_mutex_scan);
#endif
	*m_log << CustomerLog::time() << "[CameraShared::scan_Device]: scan_Device() start \n";
	if (m_tlHandle == GENTL_INVALID_HANDLE)	return	CAM_STATUS::ERROR_OPEN_TL_HANDLE;


	// new found interface
#ifdef __linux__
	auto interfaces = m_pconsumer->getInterfaces(m_tlHandle);
#endif
#ifdef _WIN32
	auto interfaces = _getInterfaces(m_tlHandle, m_sTl);
#endif


	*m_log << CustomerLog::time() << "[CameraShared::scan_Device]: interfaces.size() = " << interfaces.size() << "\n";
	for (size_t i = 0; i < interfaces.size(); ++i)
	{
		auto interfaceName = interfaces[i].second;
		GenTL::TL_HANDLE interfaceHandle = GENTL_INVALID_HANDLE;
		if (m_Interfaces_Map.count(interfaceName) == 1)
		{
			interfaceHandle = m_Interfaces_Map[interfaceName]; // has been saved in pervious scan
		}
		else
		{
#ifdef __linux__
			interfaceHandle = m_pconsumer->openInterfaceById(_findInterfaceByIndex(interfaces, i));
#endif
#ifdef _WIN32
			interfaceHandle = _openInterfaceById(_findInterfaceByIndex(interfaces, i), m_tlHandle, m_sTl); // interface new found
#endif
		}
		if (interfaceHandle == GENTL_INVALID_HANDLE)
			continue;

		*m_log << CustomerLog::time() << "[CameraShared::scan_Device]: Open Interface : " << interfaceName << "\n";
		_scan_interface(interfaceName, interfaceHandle, numberDeviceNewFound);
	}


	*m_log << CustomerLog::time() << "[CameraShared::scan_Device]: The number of available interfaces: " << m_Device_inInterface_Map.size() << "\n";
	for (auto sub : m_Device_inInterface_Map)
	{
		*m_log << CustomerLog::time() << "[CameraShared::scan_Device]:      Interface: " << sub.first << ": \n";
		for (auto sub_second : sub.second)
		{
			*m_log << CustomerLog::time() << "[CameraShared::scan_Device]:             Dev - " << sub_second << "\n";
		}
	}

    m_isDevFound = false;
	for(auto sub:m_connectedDevices)
    {
	    if(sub.second->mDeviceName != "")
        {
            m_isDevFound = true;
            break;
        }
    }

	//m_isDevFound = !m_connectedDevices.empty();


	// final check
	if (numberDeviceNewFound > 0)
	{
		*m_log << CustomerLog::time() << "[CameraShared::scan_Device]: After scanning all interfaces, " << numberDeviceNewFound << " new device(s) found!\n\n\n";
		return CAM_STATUS::All_OK;
	}

	*m_log << CustomerLog::time() << "[CameraShared::scan_Device]: After scanning all interfaces, no new device found!\n\n\n";
	return CAM_STATUS::All_OK;
}

void
CameraShared::_clear_invalid_device(cStr& name)
{
	m_connectedDevices.erase(name);

    // clear device
    for(auto devs = m_Device_inInterface_Map.begin(); devs!= m_Device_inInterface_Map.end();)
    {
		*m_log << CustomerLog::time() << "[CameraShared::_clear_invalid_device]: Try to close device: " << name << " in NetCad: " << devs->first << "\n";
		if (devs->second.empty())
		{
			m_Interfaces_Map.erase(devs->first);
			m_Device_inInterface_Map.erase(devs++);
			*m_log << CustomerLog::time() << "[CameraShared::_clear_invalid_device]: Not found device: " << name << " in NetCad: " << devs->first << "\n";
			++devs;
			continue;
		}

		if (devs->second.count(name) == 1)
		{
			devs->second.erase(name);
			*m_log << CustomerLog::time() << "[CameraShared::_clear_invalid_device]: Close device: " << name << " in NetCad: " << devs->first << "\n";
		}

        if(devs->second.empty())
        {
			*m_log << CustomerLog::time() << "[CameraShared::_clear_invalid_device]: Interface is empty in NetCad: " << devs->first << "\n";
#ifdef __linux__
            m_pconsumer->closeInterface(m_Interfaces_Map[devs->first]);
#endif
#ifdef _WIN32
            CC(m_sTl, m_sTl->IFClose(m_Interfaces_Map[devs->first]));
#endif
            *m_log << CustomerLog::time() << "[CameraShared::_clear_invalid_device]: Close Interface: " << devs->first << "\n";
            m_Interfaces_Map.erase(devs->first);
            m_Device_inInterface_Map.erase(devs++);
        }
		else
			++devs;
    }
}

void
CameraShared::_scan_interface(cStr&               interfaceName,
                              GenTL::IF_HANDLE	  interfaceHandle,
                              int &               nDevices)
{
#ifdef __linux__
    auto devices = m_pconsumer->getDevices_is_changed(interfaceHandle);
#else
    auto devices = _getDevices_is_changed(interfaceHandle, m_sTl);
#endif
    if (!devices.empty())
    {
        if(m_Interfaces_Map.count(interfaceName) == 0)
            m_Interfaces_Map.insert({interfaceName, interfaceHandle});

        if(m_Device_inInterface_Map.count(interfaceName) == 0)
            m_Device_inInterface_Map.insert({interfaceName, std::set<std::string>()});

		*m_log << CustomerLog::time() << "    [CameraShared::_scan_interface]: # Interface: " << interfaceName << " has GenICam device(s)! \n";
		*m_log << CustomerLog::time() << "    [CameraShared::_scan_interface]: # Find total " << devices.size() << " GenICam device(s) in Interface. \n";
		
		nDevices = 0;
        for (size_t j = 0; j < devices.size(); ++j)
            _add_device(interfaceName, interfaceHandle, devices, j, nDevices);
    }
    else
	{
        if (m_Interfaces_Map.count(interfaceName) == 0)
		{
#ifdef __linux__
            m_pconsumer->closeInterface(interfaceHandle);
#else
            _closeInterface(interfaceHandle, m_sTl);
#endif
            *m_log << CustomerLog::time() << "        Warning: [CameraShared::_scan_interface]: No device in Interface: " << interfaceName << ").\n";
        }
    }
}

void
CameraShared::_add_device(	cStr&               interfaceName,
							GenTL::IF_HANDLE	interfaceHandle,
							const SiDeviceList&	devices,
							const int64_t&		id,
							int & nDevices)
{
	if (m_Device_inInterface_Map.count(interfaceName) != 1)
	{
		*m_log << CustomerLog::time() << "            Error: [CameraShared::_add_device]: interfaceName: " << interfaceName << " not found!\n";
		return;
	}

	// [done]: add check to ignore the devices connected ---------------
	auto it = devices.begin();
	std::advance(it, id);
	auto device_name = it->second;

	// device: unsupported
	if (device_name == "")
	{
		*m_log << CustomerLog::time() << "        [CameraShared::_add_device]:     [x ] Unsupported   device: " << it->first << " \n";
		if(m_connectedDevices.count(it->first) == 0)
			m_connectedDevices.insert({ it->first, std::make_shared<DeviceConnection>(it->first)});
		return;
	}

	// device: already found
	if (m_connectedDevices.count(device_name) == 1)
	{
		if(m_connectedDevices[device_name]->mDeviceName != "")
		*m_log << CustomerLog::time() << "        [CameraShared::_add_device]:     [! ] Ignore [SICK] device: " << device_name << ", already found! \n";
		return;
	}
	
	auto pDev = std::make_shared<DeviceConnection>(	m_pconsumer,
													interfaceHandle,
													devices,
													id,
													interfaceName);

	// device: occupied or unreachable
	if (pDev->isOccupied())
	{
		*m_log << CustomerLog::time() << "        [CameraShared::_add_device]:     [! ] Error  [SICK] device: " << device_name << ", occupied, device is busy! \n";
		return;
	}
	if (!pDev->isReachable())
	{
		*m_log << CustomerLog::time() << "        [CameraShared::_add_device]:     [! ] Error  [SICK] device: " << device_name << ", unreachable, device ip = "<<pDev->getIp() << ", please check computer ip! \n";
		return;
	}

	// device is available
	++nDevices;
    m_connectedDevices.insert({pDev->mDeviceName, pDev});
    m_Device_inInterface_Map[interfaceName].insert(pDev->mDeviceName);
		*m_log << CustomerLog::time() << "        [CameraShared::_add_device]:     [ok] Add    [SICK] device: " << pDev->mDeviceName << ", successfully! \n";
}

std::string			
CameraShared::getLastErrorMessage()
{
	return m_log->getLastLog();
}

std::string
CameraShared::getPathToRanger3Producer()
{
#ifdef __linux__
    return "/usr/local/lib/sickGenTLProducer.cti";
#endif
#ifdef _WIN32
	std::string path;
	char* p = nullptr;
	p = getenv("SICK_GENICAM_CTI"); // Load from environment path 
	if(p != nullptr)
		path = std::string(p);
	else
	{
		char pathToExe[FILENAME_MAX]; // Try to load from exe-file path 
		if (!GetModuleFileName(nullptr, pathToExe, sizeof(pathToExe))) {
			std::cerr << "Error: [CameraShared::getPathToRanger3Producer] Could not find cti-file" << std::endl;
			return "";
		}
		path = std::string(pathToExe);
	}
	
	std::string pathToCti = path + "\\SICKGigEVisionTL.cti";
	if (!_fileExists(pathToCti))
	{
		std::cerr
			<< "Error: [CameraShared::getPathToRanger3Producer] Could not find SICKGigEVisionTL.cti.\n"
			<< "Please make sure to place the .cti-file in here:\n    * " << pathToCti << std::endl;
		return "";
	}
	
	return pathToCti;
	
#endif
}

#ifdef _WIN32
std::string
CameraShared::getPathToTrispectorProducer()
{
	std::string path;
	auto p = getenv("SICK_GENICAM_CTI");// Load from environment path 
	if (p != nullptr)
		path = std::string(p);
	else 
	{
		char pathToExe[FILENAME_MAX];
		if (!GetModuleFileName(nullptr, pathToExe, sizeof(pathToExe)))	{
			std::cerr << "Error: [CameraShared::getPathToTrispectorProducer] Could not find cti-file" << std::endl;
			return "";
		}
		path = std::string(pathToExe);
	}

	std::string pathToCti = std::string(path) + "\\SICKTrispectorTL64.cti";
	if (!_fileExists(pathToCti))
	{
		std::cerr
			<< "Error: [CameraShared::getPathToTrispectorProducer] Could not find SICKTrispectorTL64.cti." << std::endl
			<< "Please make sure to place the .cti-file in the same directory "
			<< "as the built sample executable." << std::endl
			<< pathToCti << std::endl;
		return "";
	}

	return pathToCti;
}
#endif

void
CameraShared::_setProcessPriority()
{
#ifdef _WIN32
	if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
	{
		*m_log << CustomerLog::time() << "Error: [CameraShared::_setProcessPriority]: Failed to set priority class REALTIME!\n";
		return;
	}

	if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST))
	{
		*m_log << CustomerLog::time() << "Error: [CameraShared::_setProcessPriority]: Failed to set thread priority TIME CRITICAL!\n";
		return;
	}

	// We have seen situations where it is necessary to set ES_DISPLAY_REQUIRED to
	// make sure that buffer recording is not disturbed. In addition we also need
	// to ensure that the processor does not enter a lower power state.
	if (!SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED | ES_DISPLAY_REQUIRED))
	{
		*m_log << CustomerLog::time() << "Error: [CameraShared::_setProcessPriority]: Failed to disable various power-saving modes!\n";
		return;
	}
#endif
#if 0
#ifdef __linux__
	pid_t pid = getpid();
	sched_param param;
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	if (param.sched_priority == -1)
	{
		std::cout << "Error: [CameraShared::_setProcessPriority]: Failed to call sched_get_priority_max()!\n";
		return;
	}

	auto exe_code = sched_setscheduler(pid, SCHED_FIFO, &param);
	if (exe_code != 0)
	{
		std::cerr << "\n\nError: [CameraShared::_setProcessPriority]: Failed to call sched_setscheduler("<< pid << ", SCHED_FIFO, " << param.sched_priority <<"), return " << exe_code << ", "<< strerror(errno) << " !\n";
		std::cerr << "\nThere is a protential solution, please check that: \n";
		std::cerr << "    1. Run: \"sudo cat /sys/fs/cgroup/cpu/user.slice/cpu.rt_runtime_us\"\n";
		std::cerr << "    2. If the value is Zero, pleasee change it to 950000\n";
		std::cerr << "\nHere is the refelerance page in nvidia website:\n";
		std::cerr << "    https://forums.developer.nvidia.com/t/operation-not-permitted-by-using-sched-setscheduler/70114/2\n\n";
		return;
	}
	exe_code = pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
	if (exe_code != 0)
	{
		std::cerr << "Error: [CameraShared::_setProcessPriority]: Failed to call pthread_setschedparam(), return " << exe_code  << ", "<< strerror(errno) << " !\n";
		std::cerr << "\nThere is a protential solution, please check that: \n";
		std::cerr << "    1. Run: \"sudo cat /sys/fs/cgroup/cpu/user.slice/cpu.rt_runtime_us\"\n";
		std::cerr << "    2. If the value is Zero, pleasee change it to 950000\n";
		std::cerr << "\nHere is the refelerance page in nvidia website:\n";
		std::cerr << "    https://forums.developer.nvidia.com/t/operation-not-permitted-by-using-sched-setscheduler/70114/2\n\n";
		return;
	}
#endif
#endif

}

bool
CameraShared::_clearR3S()
{
	try
	{
		m_connectedDevices.clear();

		for (auto sub : m_Interfaces_Map)
		{
#ifdef __linux__
			m_pconsumer->closeInterface(sub.second);
#endif
#ifdef _WIN32
			CC(m_sTl, m_sTl->IFClose(sub.second));
#endif
			*m_log << CustomerLog::time() << "[CameraShared::_clearR3S]: Close Interface: " << sub.first << "\n";
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "[CameraShared::_clearR3S]: Catch exception: " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "[CameraShared::_clearR3S]: Catch unknown exception." <<  std::endl;
	}


    *m_log << CustomerLog::time() << "[CameraShared::_clearR3S]: Close all Interfaces.\n";

    m_Interfaces_Map = {};
    m_Device_inInterface_Map = {};
    return m_Interfaces_Map.empty();
}

bool
CameraShared::_initLog(cStr & logPath)
{
	if (logPath.empty())
	{
		m_logPath = "";
		m_enableLogWriteToFile = false;
	}
	else
	{
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		time_t tt = std::chrono::system_clock::to_time_t(now);
		struct tm ltm = { 0 };
#ifdef __linux__
        localtime_r(&tt, &ltm);
#endif
#ifdef _WIN32
        localtime_s(&ltm, &tt);
#endif
		std::stringstream stm;
		stm << std::setfill('0');
		stm << std::setw(4) << (ltm.tm_year + 1900);
		stm << std::setw(2) << (ltm.tm_mon + 1);
		stm << std::setw(2) << (ltm.tm_mday);
		stm << std::setw(2) << (ltm.tm_hour);
		stm << std::setw(2) << (ltm.tm_min);
		stm << std::setw(2) << (ltm.tm_sec);

		m_logPath = logPath + "_" + stm.str() + "_log.txt";
	}

	m_log = std::make_shared<CustomerLog>(m_logPath, m_enableLogOutput, m_enableLogWriteToFile);
	*m_log << getVersion() << ", " << getVersionTime() << "\n";
	return true;
}

bool 
CameraShared::_fileExists(const std::string& pathToFile)
{
	std::ifstream file(pathToFile.c_str());
	return file.good();
}

}
