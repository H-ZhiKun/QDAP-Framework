#include "DeviceConnection.h"

#ifdef __linux__
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#endif
#ifdef _WIN32
#include <winsock2.h>
#include <Iphlpapi.h>
#pragma comment(lib, "Iphlpapi.lib")
#endif

#include <memory>
#include <algorithm>

#include <omp.h>

namespace SickCam
{

////////////////////////////////////////////////////////////////////////////////

#pragma region Consumer

InterfaceList
_getInterfaces(const GenTL::TL_HANDLE hTL, const SPtr<GenTLApi> mTl)
{
	bool8_t changed = false;
	uint64_t timeout = 100; // Timeout in milliseconds
	CC(mTl, mTl->TLUpdateInterfaceList(hTL, &changed, timeout));

	uint32_t numCandidates = 0;
	CC(mTl, mTl->TLGetNumInterfaces(hTL, &numCandidates));

	InterfaceList interfaceList;

	for (uint32_t i = 0; i < numCandidates; ++i)
	{
		char interfaceId[1024];
		size_t interfaceIdSize = sizeof(interfaceId);
		CC(mTl, mTl->TLGetInterfaceID(hTL, i, interfaceId, &interfaceIdSize));

		GenTL::INFO_DATATYPE dataType;
		char displayName[1024];
		size_t displayNameSize = sizeof(displayName);
		CC(mTl, mTl->TLGetInterfaceInfo(hTL,
			interfaceId,
			GenTL::INTERFACE_INFO_DISPLAYNAME,
			&dataType,
			displayName,
			&displayNameSize));

		interfaceList.push_back(std::make_pair(interfaceId, displayName));
	}

	return interfaceList;
}

InterfaceId
_findInterfaceByIndex(const InterfaceList & interfaces, const uint32_t & index)
{
	if (index > interfaces.size())
	{
		std::cerr << "    Cannot select interface with index: " << index << std::endl;
		return InterfaceId();
	}

	auto it = interfaces.begin();
	std::advance(it, index);

	return it->first;
}

GenTL::IF_HANDLE
_openInterfaceById(const InterfaceId& interfaceId, const GenTL::TL_HANDLE mTlHandle, const SPtr<GenTLApi> mTl)
{
	if (interfaceId.empty())
	{
		std::cerr << "    Cannot find interface with id " << interfaceId << std::endl;
		return GENTL_INVALID_HANDLE;
	}

	GenTL::IF_HANDLE interfaceHandle = NULL;
	auto err = mTl->TLOpenInterface(mTlHandle, interfaceId.c_str(), &interfaceHandle);

	if (interfaceHandle == GENTL_INVALID_HANDLE)
	{
		std::cerr << "    Interface: " << interfaceId << " cannot be opened."
			<< std::endl;
	}

	if (err == GenTL::GC_ERROR_LIST::GC_ERR_RESOURCE_IN_USE)
	{
		std::cerr << "    Interface: " << interfaceId << " is been used."
			<< std::endl;
	}

	return interfaceHandle;
}

void
_closeInterface(const GenTL::IF_HANDLE interfaceHandle, const SPtr<GenTLApi> mTl)
{
	try
	{
		CC(mTl, mTl->IFClose(interfaceHandle));
	}
	catch(...)
	{
		return;
	}
}

DeviceList
_getDevices_is_changed(const GenTL::IF_HANDLE ifHandle, const SPtr<GenTLApi> mTl)
{
	// to return all GenICam device and ignore status.
	DeviceList devices;
	bool8_t changed = false;
	CC(mTl, mTl->IFUpdateDeviceList(ifHandle, &changed, 500));

	//if (!changed)
	//	return DeviceList();

	uint32_t numDevices;
	CC(mTl, mTl->IFGetNumDevices(ifHandle, &numDevices));

	for (size_t i = 0; i < numDevices; ++i)
	{
		char deviceId[1024];
		size_t deviceIdSize = sizeof(deviceId);
		CC(mTl, mTl->IFGetDeviceID(ifHandle, static_cast<uint32_t>(i), deviceId, &deviceIdSize));

		GenTL::INFO_DATATYPE dataType;

		//{
		//	// check if device is ready
		//	GenTL::DEVICE_ACCESS_STATUS dev_status = 888;
		//	size_t cmdSize = sizeof(dev_status);
		//	GenTL::GC_ERROR ret = mTl->IFGetDeviceInfo(	ifHandle,
		//												deviceId,
		//												GenTL::DEVICE_INFO_ACCESS_STATUS,
		//												&dataType,
		//												&dev_status,
		//												&cmdSize);
		//
		//	if (GenTL::DEVICE_ACCESS_STATUS_READWRITE != dev_status)
		//		continue;
		//}


		// add model name check here
		{
			// Keep Ranger3 and Trispector only
			char modelName[1024];
			size_t modelNameSize = sizeof(modelName);
			CC(mTl, mTl->IFGetDeviceInfo(ifHandle, deviceId, GenTL::DEVICE_INFO_MODEL, &dataType, modelName, &modelNameSize));

			std::string modelName_str(modelName);
			std::transform(modelName_str.begin(), modelName_str.end(), modelName_str.begin(), ::tolower);
			
			bool is_SICK = modelName_str.find("range") < modelName_str.size()
				|| modelName_str.find("trispector") < modelName_str.size()
				|| modelName_str.find("rule") < modelName_str.size()
				|| modelName_str.find("sick") < modelName_str.size();

			if (!is_SICK)
			{
				char sn[1024];
				size_t snSize = sizeof(sn);
				CC(mTl, mTl->IFGetDeviceInfo(ifHandle, deviceId, GenTL::DEVICE_INFO_SERIAL_NUMBER, &dataType, sn, &snSize));

				devices.push_back(std::make_pair(modelName_str + "(" + sn + ")", "")); // make unsupported device (modelName + "(" + sn + ")", "")
				continue;
			}
		}

		// add supported device
		char deviceUserId[1024];
		size_t deviceUserIdSize = sizeof(deviceUserId);
		GenTL::GC_ERROR status = mTl->IFGetDeviceInfo(ifHandle, deviceId, GenTL::DEVICE_INFO_USER_DEFINED_NAME, &dataType, deviceUserId, &deviceUserIdSize);
		devices.push_back(std::make_pair(deviceId, std::string(deviceId) + "_" + deviceUserId));
	}
	return devices;
}

DeviceList
_getDevices(const GenTL::IF_HANDLE ifHandle, const SPtr<GenTLApi> mTl)
{
	DeviceList devices;
	bool8_t changed = false;
	CC(mTl, mTl->IFUpdateDeviceList(ifHandle, &changed, 500));

	uint32_t numDevices;
	CC(mTl, mTl->IFGetNumDevices(ifHandle, &numDevices));

	for (size_t i = 0; i < numDevices; ++i)
	{
		char deviceId[1024];
		size_t deviceIdSize = sizeof(deviceId);

		CC(mTl, mTl->IFGetDeviceID(
			ifHandle, static_cast<uint32_t>(i), deviceId, &deviceIdSize));

		GenTL::INFO_DATATYPE dataType;
		char modelName[1024];
		size_t modelNameSize = sizeof(modelName);
		CC(mTl, mTl->IFGetDeviceInfo(ifHandle,
			deviceId,
			GenTL::DEVICE_INFO_MODEL,
			&dataType,
			modelName,
			&modelNameSize));

		// Append user defined name if possible
		char deviceUserId[1024];
		size_t deviceUserIdSize = sizeof(deviceUserId);
		GenTL::GC_ERROR status =
			mTl->IFGetDeviceInfo(ifHandle,
				deviceId,
				GenTL::DEVICE_INFO_USER_DEFINED_NAME,
				&dataType,
				deviceUserId,
				&deviceUserIdSize);

		std::stringstream deviceName;
		deviceName << modelName;
		if (status == GenTL::GC_ERR_SUCCESS)
		{
			deviceName << ": " << deviceUserId;
		}

		devices.push_back(std::make_pair(deviceId, deviceName.str()));
	}
	return devices;
}

GenTL::TL_HANDLE
_TL_HANDLE_open(const SPtr<GenTLApi> mTl)
{
	GenTL::TL_HANDLE mTlHandle;
	//CC(mTl, mTl->GCInitLib());
	CC(mTl, mTl->TLOpen(&mTlHandle));
	return mTlHandle;
}

Str
_findDeviceByIndex(const SiDeviceList & devices, const uint32_t & index)
{
	if (index >= devices.size())
	{
		std::cerr << "    Cannot select device with index " << index << std::endl;
		return Str();
	}

	auto it = devices.begin();
	std::advance(it, index);

	return it->first;
}

GDeviceH
_openDeviceById(const GenTL::TL_HANDLE interfaceHandle, const Str & deviceId, const SPtr<GenTLApi> mTl, GenTL::GC_ERROR& status)
{
	if (deviceId.empty())
	{
		//std::cerr << "    Device id string was empty." << std::endl;
		return GENTL_INVALID_HANDLE;
	}

	GenTL::DEVICE_ACCESS_FLAGS deviceAccess = GenTL::DEVICE_ACCESS_CONTROL; 
	GenTL::DEV_HANDLE deviceHandle;
	//GenTL::GC_ERROR status;
	status = mTl->IFOpenDevice(interfaceHandle, deviceId.c_str(), deviceAccess, &deviceHandle);
	if (deviceHandle == GENTL_INVALID_HANDLE || status != GenTL::GC_ERR_SUCCESS)
	{
		//std::cerr << "    Could not open device. " << std::endl;
		return GENTL_INVALID_HANDLE;
	}

	return deviceHandle;
}

#ifndef __linux__
std::vector<std::vector<Str>>
getLocalIP()
{
	std::vector<std::vector<Str>> vecIpSms(0);

	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO[10];
	unsigned long stSize = sizeof(IP_ADAPTER_INFO) * 10;
	int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	if (ERROR_BUFFER_OVERFLOW == nRel) {
		if (pIpAdapterInfo != NULL)
			delete[] pIpAdapterInfo;
		return vecIpSms;
	}

	PIP_ADAPTER_INFO cur = pIpAdapterInfo;
	std::string ip, sm;
	int i = 0;
	//for (int i = 0; i<1000; ++i)
	while (cur)
	{
		++i;
		if (i > MAX_N_NETCARD)
			break;

		switch (cur->Type) 
		{
		case MIB_IF_TYPE_OTHER:
			break;
		case MIB_IF_TYPE_ETHERNET:
		{
			IP_ADDR_STRING *pIpAddrString = &(cur->IpAddressList);
			ip = std::string(pIpAddrString->IpAddress.String, 16);
			if (ip.substr(0,7) == "0.0.0.0")
				continue;

			vecIpSms.push_back({
				std::string(pIpAddrString->IpAddress.String, 16),
				std::string(pIpAddrString->IpMask.String, 16),
				//std::string(cur->AdapterName),
				std::string(cur->Description),
				});
		}
		break;
		case MIB_IF_TYPE_TOKENRING:
			break;
		case MIB_IF_TYPE_FDDI:
			break;
		case MIB_IF_TYPE_PPP:
			break;
		case MIB_IF_TYPE_LOOPBACK:
			break;
		case MIB_IF_TYPE_SLIP:
			break;
		default://Wireless
		{
			IP_ADDR_STRING *pIpAddrString = &(cur->IpAddressList);
			ip = std::string(pIpAddrString->IpAddress.String, 16);
			if (ip.substr(0, 7) == "0.0.0.0")
			
			vecIpSms.push_back({
				std::string(pIpAddrString->IpAddress.String, 16),
				std::string(pIpAddrString->IpMask.String, 16),
				std::string(cur->AdapterName),
				});
		}
		break;
		}
		cur = cur->Next;
	}

	if (pIpAdapterInfo != NULL)
		delete[] pIpAdapterInfo;

	return vecIpSms;
}
#else
    std::vector<std::vector<Str>>
    getLocalIP()
    {
        std::vector<std::vector<Str>> vecIpSms(0);

        struct sockaddr_in *sin = NULL;
        struct ifaddrs *ifa = NULL, *ifList;
        std::string name[10];
        if (getifaddrs(&ifList) < 0)
        {
            return vecIpSms;
        }
        int i = 0;
        //找到所有网卡，并打印网卡相关信息
        for (ifa = ifList; ifa != NULL; ifa = ifa->ifa_next,++i)
        {
            if(ifa->ifa_addr->sa_family == AF_INET)
            {
                std::string ifname(ifa->ifa_name);
                //name[i]=ifname;
                //i++;
                //printf("interfaceName: %s\n", ifa->ifa_name);

                sin = (struct sockaddr_in *)ifa->ifa_addr;
                //printf("ipAddress: %s\n", inet_ntoa(sin->sin_addr));

                std::string ip = inet_ntoa(sin->sin_addr);

                //sin = (struct sockaddr_in *)ifa->ifa_dstaddr;
                //printf("broadcast: %s\n", inet_ntoa(sin->sin_addr));

                sin = (struct sockaddr_in *)ifa->ifa_netmask;
                //printf("subnetMask: %s\n\n", inet_ntoa(sin->sin_addr));

                std::string submask = inet_ntoa(sin->sin_addr);

                vecIpSms.push_back({ip, submask, ifname});
            }
        }
        freeifaddrs(ifList);
    }
#endif

    bool
    checkIpSegments(const std::string & _ip1, const std::string & _submask1, const std::string & _ip2, const std::string & _submask2)
    {
        auto ip1 = Ip4FromString(_ip1);
        auto sm1 = Ip4FromString(_submask1);
        auto ip2 = Ip4FromString(_ip2);
        auto sm2 = Ip4FromString(_submask2);

        auto sm = sm1 > sm2 ? sm1 : sm2;
        auto segment1 = ip1 & sm;
        auto segment2 = ip2 & sm;

        return segment1 == segment2;
    }


#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region DeviceConnection

DeviceConnection::DeviceConnection(const std::string & unsupportedModelName) // make it as unsupported GenICam devices, mDeviceName = ""
	: mDeviceName("")
	, mUnsupportedDeviceModel(unsupportedModelName)
	, mAcquisitionRunning(false)
	, mIsReachable(true)
	, mIsInitialized(false)
	, mIsOpenDataStream(false)
{};

DeviceConnection::DeviceConnection( ConsumerPtr			consumer, 
									GenTL::IF_HANDLE	interfaceHandle,
									const SiDeviceList&	devices, 
									const int64_t&		id,
                                    const std::string & interfaceName)
	: 
	mpConsumer(consumer),
	mAcquisitionRunning(false), 
	mIsReachable(true),
	mInterfaceHandle(interfaceHandle),
	mIsInitialized(false), 
	mIsOpenDataStream(false), 
	mId(id),
    mInterfaceName(interfaceName)
{
	mTl = mpConsumer->tl();
#ifdef __linux__
    mInterfacePort = std::make_shared<SiGenTLPort>(interfaceHandle, mTl);
    mInterfaceNodeMap = mpConsumer->getNodeMap(mInterfacePort.get(), "InterfacePort");
#else
    mInterfacePort = std::make_shared<SiGenTLPort>(mTl, interfaceHandle);
    mInterfaceNode = NodeMap(mInterfacePort, "InterfacePort");
    // get NodeMapRef
    mInterfaceNodeMap = mInterfaceNode.get();
#endif

	mDeviceId = _findDeviceByIndex(devices, mId);
	mDeviceName = "";
	if (mDeviceId.empty())
	{
		std::cerr << "    Error:[DeviceConnection::DeviceConnection]:  mDeviceId is empty! " << std::endl;
		return;
	}

	if(openDevice())
	    closeDevice();
	else
        mDeviceName = ""; // mark this device is invalid
}

DeviceConnection::~DeviceConnection()
{
	if (mDeviceName != "")
	{
		closeDataStream();
		closeDevice();

		mBufferData.clear();
		mBufferData = std::vector<uint8_t*>();
	}
}

bool
DeviceConnection::openDevice()
{
	if (mIsInitialized) return true;
	if (mDeviceId.empty()) return false;
	mDeviceName = "";


	mIsTrispector = mDeviceId.substr(0, 16) == "SICKTriSpectorTL";
	GenTL::GC_ERROR openDeviceStatus;
#ifdef __linux__
	mDeviceHandle = mpConsumer->openDeviceById(mInterfaceHandle, mDeviceId, openDeviceStatus);
#endif
#ifdef _WIN32
	mDeviceHandle = _openDeviceById(mInterfaceHandle, mDeviceId, mTl, openDeviceStatus);
#endif
	if (mDeviceHandle == GENTL_INVALID_HANDLE /*&& !is_trispector*/)
	{
		mIsInitialized = false;
		if (openDeviceStatus != GenTL::GC_ERROR_LIST::GC_ERR_RESOURCE_IN_USE)
		{
			mLastMessage.clear();
			mLastMessage << "open device return: " << openDeviceStatus;

			// for debug
			//std::cerr << "    Warning:[DeviceConnection::openDevice]: FAILED: mDeviceHandle = GENTL_INVALID_HANDLE, openDeviceStatus = " << openDeviceStatus << " \n";
			//std::cerr << "    Warning:[DeviceConnection::openDevice]: FAILED: mDeviceHandle = GENTL_INVALID_HANDLE, openDeviceStatus = " << _ENUM_STR(static_cast<GenTL::GC_ERROR_LIST>(openDeviceStatus)) << " \n";
			//std::cerr << "    Warning:[DeviceConnection::openDevice]: FAILED: deviceId = " << mDeviceId << " \n";
			//std::cerr << "    Warning:[DeviceConnection::openDevice]: FAILED: deviceList:";
			//for(auto sub: devices)
			//	std::cerr << "                (" <<sub.first << ", " << sub.second<<") \n";
			//return;
		}
	}
	else
		mIsInitialized = true;

	GenTL::INFO_DATATYPE dataType;

	// Append user defined name if possible
	char deviceUserId[1024];
	size_t deviceUserIdSize = sizeof(deviceUserId);
	auto status = mTl->IFGetDeviceInfo(mInterfaceHandle, mDeviceId.c_str(), GenTL::DEVICE_INFO_USER_DEFINED_NAME, &dataType, deviceUserId, &deviceUserIdSize);

	std::stringstream deviceName;
	deviceName << mDeviceId;
	if (status == GenTL::GC_ERR_SUCCESS)
		deviceName << "_" << deviceUserId;

	mDeviceName = deviceName.str();
	if (mIsTrispector)
		mIsInitialized = !mDeviceName.empty();
	else
	{
		_queryIP();
		_querySubnet();
		_queryMAC();
		_querySN();
	}

	if (!mIsInitialized && !mIsTrispector)
	{
		auto ip_sm = getLocalIP();
		mIsReachable = false;
		mIsOccupied = false;
		std::stringstream ss;
		for (auto sub : ip_sm)
		{
			ss << "                " << sub[0] << " | " << sub[1] << " | " << sub[2] << " ;\n";
			if(mInterfaceName == sub[2])
                mIsOccupied = mIsOccupied || checkIpSegments(sub[0], sub[1], mDeviceIP, mDeviceSubnet); // if in same segment
        }

		mLastMessage.clear();
		if (mIsOccupied)
		{
			mIsReachable = true;
			mLastMessage << mDeviceName << " is occupied! Device_IP = " << mDeviceIP;
		}
		else
		{
			mLastMessage << mDeviceName << " is unreachable! Device_IP = " << mDeviceIP << "\n                All computer IP:\n" << ss.str();
		}
	}
	else
	{
		// ok 
		mIsReachable = true;
		mIsOccupied = false;
	}

	return mIsInitialized;
}

bool
DeviceConnection::closeDevice()
{
	if (!mIsInitialized) return true;

	if(/*!mIsTrispector &&*/ isReachable())
		CC(mTl, mTl->DevClose(mDeviceHandle));
	
	mIsInitialized = false;
	return !mIsInitialized;
}

bool
DeviceConnection::openDataStream()
{
	if (mIsOpenDataStream) return true;

	char streamId[1024];
	size_t idSize = sizeof(streamId);
	CC(mTl, mTl->DevGetDataStreamID(mDeviceHandle, 0, streamId, &idSize));
	CC(mTl, mTl->DevOpenDataStream(mDeviceHandle, streamId, &mDataStreamHandle));

	mIsOpenDataStream = true;
	return mIsOpenDataStream;
}

bool
DeviceConnection::closeDataStream()
{
    if (!mIsInitialized) return true;
	if (!mIsOpenDataStream) return true;
	CC(mTl, mTl->DSClose(mDataStreamHandle));
	mIsOpenDataStream = false;
	return !mIsOpenDataStream;
}

void
DeviceConnection::initializeBuffers(size_t buffersCount, size_t payloadSize)
{
	mBufferHandles.resize(buffersCount, GENTL_INVALID_HANDLE);
	mBufferData.resize(buffersCount, nullptr);

	// test time using in initializeBuffers
	//auto d = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

#pragma omp parallel for
	for (int i = 0; i < buffersCount; ++i)
	{
		mBufferData[i] = new uint8_t[payloadSize];
		if (mBufferData[i] == nullptr)
		{
#ifdef __linux__
            throw std::exception(std::logic_error("Could not allocated enough memory for buffers"));
#else
            throw std::exception("Could not allocated enough memory for buffers");
#endif
        }
		// Store the pointer to the raw memory in the user data for sake of
		// simplicity. This makes it easy to access the memory when a buffer has
		// been received.
		CC(mTl, mTl->DSAnnounceBuffer(mDataStreamHandle,
			mBufferData[i],
			payloadSize,
			reinterpret_cast<void*>(i),
			&mBufferHandles[i]));
		CC(mTl, mTl->DSQueueBuffer(mDataStreamHandle, mBufferHandles[i]));
	}
	//auto end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

}

void
DeviceConnection::teardownBuffers()
{
	for (size_t i = 0; i < mBufferHandles.size(); i++)
	{
		// Revoke buffer from stream so that memory can be deleted safely
		CC(mTl, mTl->DSRevokeBuffer(mDataStreamHandle,
			mBufferHandles[i],
			reinterpret_cast<void**>(&mBufferData[i]),
			nullptr));
		delete[] mBufferData[i];
		mBufferData[i] = nullptr;
	}
}

void
DeviceConnection::teardownBuffersWhenLost()
{
    for (size_t i = 0; i < mBufferHandles.size(); i++)
    {
        delete[] mBufferData[i];
        mBufferData[i] = nullptr;
    }
}

void
DeviceConnection::registerNewBufferEvent()
{
	CC(mTl, mTl->GCRegisterEvent(mDataStreamHandle, GenTL::EVENT_NEW_BUFFER, &mNewBufferEventHandle));
}

void
DeviceConnection::unregisterNewBufferEvent()
{
	CC(mTl, mTl->GCUnregisterEvent(mDataStreamHandle, GenTL::EVENT_NEW_BUFFER));
}

void
DeviceConnection::startAcquisition()
{
	GenApi::CCommandPtr acquisitionStart = mDeviceNodeMap._GetNode("AcquisitionStart");
	acquisitionStart->Execute();
	mAcquisitionRunning = true;
}

void
DeviceConnection::stopAcquisition()
{
	if (mAcquisitionRunning)
	{
		GenApi::CCommandPtr acquisitionStop = mDeviceNodeMap._GetNode("AcquisitionStop");
		acquisitionStop->Execute();
		mAcquisitionRunning = false;
	}
}

bool
DeviceConnection::isAcquisitionRunning() const
{
	return mAcquisitionRunning;
}

void
DeviceConnection::createDeviceNodeMap(SiConsumer& consumer)
{
	GenTL::PORT_HANDLE devicePort;
	CC(mTl, mTl->DevGetPort(mDeviceHandle, &devicePort));
#ifdef __linux__
    mDevicePort.reset(new SiGenTLPort(devicePort, mTl));
    mDeviceNodeMap = consumer.getNodeMap(mDevicePort.get(), "Device");
#else
	mDevicePort = std::make_shared<SiGenTLPort>(mTl, devicePort);
	mDeviceNode = NodeMap(mDevicePort, "Device");
	mDeviceNodeMap = mDeviceNode.get();
#endif
}

void
DeviceConnection::createDataStreamNodeMap(SiConsumer& consumer)
{
#ifdef __linux__
    mDataStreamPort.reset(new SiGenTLPort(mDataStreamHandle, mTl));
    mDataStreamNodeMap = consumer.getNodeMap(mDataStreamPort.get(), "StreamPort");
#else
	mDataStreamPort = std::make_shared<SiGenTLPort>(mTl, mDataStreamHandle);
	mDataStreamNode = NodeMap(mDataStreamPort, "StreamPort");
	mDataStreamNodeMap = mDataStreamNode.get();
#endif
}

void
DeviceConnection::updateIp()
{
	if (mIsInitialized)
		_queryIP();
}

void
DeviceConnection::updateSubnet()
{
	if (mIsInitialized)
		_querySubnet();
}

void
DeviceConnection::_queryIP()
{
	mDeviceSelector = mInterfaceNodeMap._GetNode("DeviceSelector");
	mDeviceSelector->SetValue(mId);
	GenApi::CIntegerPtr gevDeviceIpAddress = mInterfaceNodeMap._GetNode("GevDeviceIPAddress");
	if (gevDeviceIpAddress)
	{
		uint32_t currentIp = static_cast<uint32_t>(gevDeviceIpAddress->GetValue());
		mDeviceIP = StringFromIp4(currentIp);
	}
}

void
DeviceConnection::_querySubnet()
{
	GenApi::CIntegerPtr deviceSelector = mInterfaceNodeMap._GetNode("DeviceSelector");
	deviceSelector->SetValue(mId);
	GenApi::CIntegerPtr gevDevice = mInterfaceNodeMap._GetNode("GevDeviceSubnetMask");
	if (gevDevice)
	{
		uint32_t current = static_cast<uint32_t>(gevDevice->GetValue());
		mDeviceSubnet = StringFromIp4(current);
	}
}

void
DeviceConnection::_queryMAC()
{
	GenApi::CIntegerPtr deviceSelector = mInterfaceNodeMap._GetNode("DeviceSelector");
	deviceSelector->SetValue(mId);
	GenApi::CIntegerPtr gevDeviceMacAddress = mInterfaceNodeMap._GetNode("GevDeviceMACAddress");
	if (gevDeviceMacAddress)
	{
		uint64_t currentMac = static_cast<uint64_t>(gevDeviceMacAddress->GetValue());
		mDeviceMac = StringFromMAC(currentMac);
	}
}

void DeviceConnection::_querySN()
{
	GenTL::INFO_DATATYPE dataType;
	char sn[1024];
	size_t snSize = sizeof(sn);
	CC(mTl,
	   mTl->IFGetDeviceInfo(
		   mInterfaceHandle,
		   mDeviceId.c_str(),
		   GenTL::DEVICE_INFO_SERIAL_NUMBER, 
		   &dataType,
		   sn, 
		   &snSize)
	);
	mDeviceSN = sn;
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region ChunkData

ChunkAdapter::ChunkAdapter(GenTLApi* tl, GenTL::DS_HANDLE dataStreamHandle)
        : mTl_rawPtr(tl)
        , mDataStreamHandle(dataStreamHandle)
        , mAdapter(new GenApi::CChunkAdapterGEV())
{
    // Empty
}


ChunkAdapter::ChunkAdapter(SPtr<GenTLApi> tl, GenTL::DS_HANDLE dataStreamHandle)
	: mTl(tl)
	, mDataStreamHandle(dataStreamHandle)
	, mAdapter(new GenApi::CChunkAdapterGEV())
{
	// Empty
}

void ChunkAdapter::attachNodeMap(GenApi::INodeMap* nodeMap)
{
	mAdapter->AttachNodeMap(nodeMap);
}

void ChunkAdapter::detachNodeMap()
{
	mAdapter->DetachNodeMap();
}

void ChunkAdapter::attachBuffer(GenTL::BUFFER_HANDLE handle, uint8_t* buffer)
{
	GenApi::AttachStatistics_t statistics;
	size_t chunkPayloadSize = getChunkPayloadSize(handle);
	if (!mAdapter->CheckBufferLayout(buffer, chunkPayloadSize))
	{
#ifdef __linux__
        throw std::exception(std::logic_error("Buffer has unknown chunk layout"));
#else
		throw std::exception("Buffer has unknown chunk layout");
#endif
	}
	mAdapter->AttachBuffer(buffer, chunkPayloadSize, &statistics);

	// Ranger3 uses a single chunk port for all metadata.
	if (statistics.NumChunkPorts != 1)
	{
#ifdef __linux__
        throw std::exception(std::logic_error("A single chunk port was expected"));
#else
		throw std::exception("A single chunk port was expected");
#endif
	}

	// There should be one chunk for the metadata port and one for
	// wrapping the image data.
	if (statistics.NumChunks != 2)
	{
#ifdef __linux__
        throw std::exception(std::logic_error("Two chunks were expected"));
#else
		throw std::exception("Two chunks were expected");
#endif
	}

	// Only the metadata chunk should be attached.
	if (statistics.NumAttachedChunks != 1)
	{
#ifdef __linux__
        throw std::exception(std::logic_error("A single attached chunk was expected"));
#else
		throw std::exception("A single attached chunk was expected");
#endif
	}
}

void ChunkAdapter::detachBuffer()
{
	mAdapter->DetachBuffer();
}

size_t ChunkAdapter::getChunkPayloadSize(GenTL::BUFFER_HANDLE handle)
{
	size_t result;
	size_t size = sizeof(result);
	GenTL::INFO_DATATYPE dataType;

#ifdef __linux__
	CC(mTl_rawPtr,
       mTl_rawPtr->DSGetBufferInfo(mDataStreamHandle,
			handle,
			GenTL::BUFFER_INFO_DELIVERED_CHUNKPAYLOADSIZE,
			&dataType,
			&result,
			&size));
#else
    CC(mTl,
       mTl->DSGetBufferInfo(mDataStreamHandle,
			handle,
			GenTL::BUFFER_INFO_DELIVERED_CHUNKPAYLOADSIZE,
			&dataType,
			&result,
			&size));
#endif

	return result;
}


////////////////////////////////////////////////////////////////////////////////

/** Gets the backing register of an ordinary feature. This assumes the feature
is connected to a register via <pValue>.
*/
GenApi::CRegisterPtr getRegister(GenApi::IValue& feature)
{
	GenApi::NodeList_t children;
	feature.GetNode()->GetChildren(children,
		GenApi::ELinkType::ctReadingChildren);

	assert(children.size() == 1);

	GenApi::CRegisterPtr reg(children[0]);
	// Verify that the child is an actual register
	assert(reg.IsValid());
	return reg;
}

size_t getChunkPortLength(GenApi::CChunkPortPtr chunkPort)
{
	uint64_t length;
	chunkPort->Read(&length, CHUNK_LENGTH_REGISTER, CHUNK_LENGTH_REGISTER_LEN);
	return static_cast<size_t>(length);
}


ChunkScanLineValueAccessor::ChunkScanLineValueAccessor(
	GenApi::CIntegerPtr chunkScanLineSelector,
	GenApi::CIntegerPtr feature)
{
	GenApi::CRegisterPtr registerNode = getRegister(*feature);
	chunkScanLineSelector->SetValue(0);
	mBase = registerNode->GetAddress();
	try {
		chunkScanLineSelector->SetValue(1);
		mIncrement = registerNode->GetAddress() - mBase;
	}
	catch (...)
	{
		mIncrement = 0;
	}
	
	mRegisterSize = registerNode->GetLength();
	assert(mRegisterSize == 4 || mRegisterSize == 8);

	GenICam::gcstring valueStr;
	GenICam::gcstring attributeStr;
	bool hasLsb =
		registerNode->GetNode()->GetProperty("LSB", valueStr, attributeStr);
	mLsb = hasLsb ? atoi(valueStr.c_str()) : (mRegisterSize == 4 ? 31 : 63);

	bool hasMsb =
		registerNode->GetNode()->GetProperty("MSB", valueStr, attributeStr);
	size_t msb = hasMsb ? atoi(valueStr.c_str()) : 0;
	mMask = mRegisterSize == 4 ? createBitMask32(mLsb, msb)
		: createBitMask64(mLsb, msb);
}
#ifdef __linux__
    uint64_t htonll(uint64_t val)
    {
        return (((uint64_t) htonl(val)) << 32) + htonl(val >> 32);
    }

    uint64_t ntohll(uint64_t val)
    {
        return (((uint64_t) ntohl(val)) << 32) + ntohl(val >> 32);
    }
#endif


uint64_t ChunkScanLineValueAccessor::getValue64(const uint8_t* buffer,
	size_t lineIndex)
{
	assert(mRegisterSize == 8);
	return getRange64(ntohll(*reinterpret_cast<const int64_t*>(
		&buffer[mBase + mIncrement * lineIndex])));
}

uint32_t ChunkScanLineValueAccessor::getValue32(const uint8_t* buffer,
	size_t lineIndex)
{
	assert(mRegisterSize == 4);
	return getRange32(
		static_cast<uint32_t>(ntohl(*reinterpret_cast<const uint32_t*>(
			&buffer[mBase + mIncrement * lineIndex]))));
}

FastMetadataExtractor::FastMetadataExtractor(GenApi::CNodeMapRef device)
	: mChunkScanLineSelector(device._GetNode("ChunkScanLineSelector"))
	, mTimestamp(mChunkScanLineSelector, device._GetNode("ChunkTimestamp"))
	, mEncoderValue(mChunkScanLineSelector, device._GetNode("ChunkEncoderValue"))
	, mOvertriggerCount(mChunkScanLineSelector,
		device._GetNode("ChunkOvertriggerCount"))
	, mFrameTriggerActive(mChunkScanLineSelector,
		device._GetNode("ChunkFrameTriggerActive"))
	, mLineTriggerActive(mChunkScanLineSelector,
		device._GetNode("ChunkLineTriggerActive"))
	, mEncoderResetActive(mChunkScanLineSelector,
		device._GetNode("ChunkEncoderResetActive"))
	, mEncoderA(mChunkScanLineSelector, device._GetNode("ChunkEncoderA"))
	, mEncoderB(mChunkScanLineSelector, device._GetNode("ChunkEncoderB"))
	, mChunkPort(device._GetNode("DeviceChunkPort"))
{
}

std::vector<Ranger3LineMetadata> FastMetadataExtractor::extract()
{
	std::vector<uint8_t> buffer(getChunkPortLength(mChunkPort), 0);
	mChunkPort->Read(buffer.data(), 0, buffer.size());

	std::vector<Ranger3LineMetadata> metadata;
	const auto lineMin = mChunkScanLineSelector->GetMin();
	const auto lineMax = mChunkScanLineSelector->GetMax();

	// Pre-allocate the memory and access directly by index in the loop. This
	// avoids calling push_back, since that would be the most expensive operation
	// in debug builds
	metadata.resize(lineMax - lineMin + 1);

//#pragma omp parallel for
	for (int64_t i = lineMin; i <= lineMax; ++i)
	{
		size_t index = i - lineMin;
		Ranger3LineMetadata& lineMetadata = metadata[index];
		lineMetadata.timestamp = mTimestamp.getValue64(buffer.data(), i);
		lineMetadata.encoderValue = mEncoderValue.getValue32(buffer.data(), i);
		lineMetadata.overtriggerCount =
			static_cast<uint8_t>(mOvertriggerCount.getValue32(buffer.data(), i));
		lineMetadata.frameTriggerActive =
			mFrameTriggerActive.getValue32(buffer.data(), i) ? true : false;
		lineMetadata.lineTriggerActive =
			mLineTriggerActive.getValue32(buffer.data(), i) ? true : false;
		lineMetadata.encoderResetActive =
			mEncoderResetActive.getValue32(buffer.data(), i) ? true : false;
		lineMetadata.encoderA =
			mEncoderA.getValue32(buffer.data(), i) ? true : false;
		lineMetadata.encoderB =
			mEncoderB.getValue32(buffer.data(), i) ? true : false;
	}
	return metadata;
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region BufferControl

BufferInfo::BufferInfo(	
#ifdef __linux__
						GenTLApi* tl,
#else
						SPtr<GenTLApi> tl,
#endif
						GenTL::DS_HANDLE dataStreamHandle,
						GenTL::BUFFER_HANDLE bufferHandle)
{
	GenTL::INFO_DATATYPE infoType = GenTL::INFO_DATATYPE_UNKNOWN;
	size_t infoSize = sizeof(mBufferIncomplete);
	CC(tl, tl->DSGetBufferInfo(dataStreamHandle, bufferHandle, GenTL::BUFFER_INFO_IS_INCOMPLETE, &infoType, &mBufferIncomplete, &infoSize));

	infoSize = sizeof(mBufferFilledSize);
	CC(tl, tl->DSGetBufferInfo(dataStreamHandle, bufferHandle, GenTL::BUFFER_INFO_SIZE_FILLED, &infoType, &mBufferFilledSize, &infoSize));

	infoSize = sizeof(mBufferFrameID);
	CC(tl, tl->DSGetBufferInfo(dataStreamHandle, bufferHandle, GenTL::BUFFER_INFO_FRAMEID, &infoType, &mBufferFrameID, &infoSize));

	// add 
	infoSize = static_cast<size_t>(sizeof(mDataPointer));
	CC(tl, tl->DSGetBufferInfo(dataStreamHandle, bufferHandle, GenTL::BUFFER_INFO_BASE, &infoType, &mDataPointer, &infoSize));

	infoSize = sizeof(mBufferDataSize);
	CC(tl, tl->DSGetBufferInfo(dataStreamHandle, bufferHandle, GenTL::BUFFER_INFO_DATA_SIZE, &infoType, &mBufferDataSize, &infoSize));

	infoSize = sizeof(mBufferPayloadType);
	CC(tl, tl->DSGetBufferInfo(dataStreamHandle, bufferHandle, GenTL::BUFFER_INFO_PAYLOADTYPE, &infoType, &mBufferPayloadType, &infoSize));

	if (mBufferPayloadType == GenTL::PAYLOAD_TYPE_MULTI_PART)
	{
		CC(tl, tl->DSGetNumBufferParts(dataStreamHandle, bufferHandle, &mPartCount));
	}
	else
	{
		mPartCount = 0;

		infoSize = static_cast<size_t>(sizeof(mDataFormat));
		CC(tl, tl->DSGetBufferInfo(dataStreamHandle, bufferHandle, GenTL::BUFFER_INFO_PIXELFORMAT, &infoType, &mDataFormat, &infoSize));
	}

	// VLD-21
	try {
		infoSize = sizeof(mDeliveredLineCount);
		CC(tl, tl->DSGetBufferPartInfo(dataStreamHandle, bufferHandle, 0, GenTL::BUFFER_PART_INFO_DELIVERED_IMAGEHEIGHT, &infoType, &mDeliveredLineCount, &infoSize));
	}
	catch (...) { mDeliveredLineCount = 0; }
}


BufferPartInfo::BufferPartInfo( 
#ifdef __linux__
								GenTLApi* tl,
#else
								SPtr<GenTLApi> tl,
#endif
                                GenTL::DS_HANDLE dataStreamHandle,
                                GenTL::BUFFER_HANDLE bufferHandle,
                                uint32_t partNumber)
	: mPartNumber(partNumber)
{
	GenTL::INFO_DATATYPE infoType = GenTL::INFO_DATATYPE_UNKNOWN;
	size_t infoSize = static_cast<size_t>(sizeof(mPartDataPointer));
	CC(tl, tl->DSGetBufferPartInfo(dataStreamHandle, bufferHandle, mPartNumber,
		GenTL::BUFFER_PART_INFO_BASE,
		&infoType, &mPartDataPointer, &infoSize));

	infoSize = static_cast<size_t>(sizeof(mPartDataType));
	CC(tl, tl->DSGetBufferPartInfo(dataStreamHandle, bufferHandle, mPartNumber,
		GenTL::BUFFER_PART_INFO_DATA_TYPE,
		&infoType, &mPartDataType, &infoSize));

	infoSize = static_cast<size_t>(sizeof(mPartDataFormat));
	CC(tl, tl->DSGetBufferPartInfo(dataStreamHandle, bufferHandle, mPartNumber,
		GenTL::BUFFER_PART_INFO_DATA_FORMAT,
		&infoType, &mPartDataFormat, &infoSize));

	infoSize = static_cast<size_t>(sizeof(mPartDataSize));
	CC(tl, tl->DSGetBufferPartInfo(dataStreamHandle, bufferHandle, mPartNumber,
		GenTL::BUFFER_PART_INFO_DATA_SIZE,
		&infoType, &mPartDataSize, &infoSize));

	infoSize = static_cast<size_t>(sizeof(mPartDataWidth));
	CC(tl, tl->DSGetBufferPartInfo(dataStreamHandle, bufferHandle, mPartNumber,
		GenTL::BUFFER_PART_INFO_WIDTH,
		&infoType, &mPartDataWidth, &infoSize));

	infoSize = static_cast<size_t>(sizeof(mPartDataHeight));
	CC(tl, tl->DSGetBufferPartInfo(dataStreamHandle, bufferHandle, mPartNumber,
		GenTL::BUFFER_PART_INFO_HEIGHT,
		&infoType, &mPartDataHeight, &infoSize));

	infoSize = static_cast<size_t>(sizeof(mRegionId));
	CC(tl, tl->DSGetBufferPartInfo(dataStreamHandle, bufferHandle, mPartNumber,
		IVP_BUFFER_PART_CUSTOM_INFO_REGION_ID,
		&infoType, &mRegionId, &infoSize));

	infoSize = static_cast<size_t>(sizeof(mPurposeId));
	CC(tl, tl->DSGetBufferPartInfo(dataStreamHandle, bufferHandle, mPartNumber,
		IVP_BUFFER_PART_CUSTOM_INFO_DATA_PURPOSE_ID,
		&infoType, &mPurposeId, &infoSize));
}



PartInfoCollection
PartInfoCollection::fromBuffer(
#ifdef __linux__
							GenTLApi* tl,
#else
							SPtr<GenTLApi> tl,
#endif	
							GenTL::DS_HANDLE dataStreamHandle,
							GenTL::BUFFER_HANDLE bufferHandle,
							uint32_t bufferPartCount)
{
	PartInfoCollection object;
	for (uint32_t partIndex = 0; partIndex < bufferPartCount; ++partIndex)
	{
		object.mParts.push_back(BufferPartInfo(tl, dataStreamHandle, bufferHandle, partIndex));
	}
	return object;
}


const BufferPartInfo&
PartInfoCollection::findPart(size_t regionId, size_t componentId) const
{
	auto iter = findPartIterator(regionId, componentId);
	assert(iter != mParts.end());
	return *iter;
}


bool PartInfoCollection::hasPart(size_t regionId, size_t componentId) const
{
	auto iter = findPartIterator(regionId, componentId);
	return iter != mParts.end();
}

std::vector<BufferPartInfo>::const_iterator
PartInfoCollection::findPartIterator(size_t regionId, size_t componentId) const
{
	return std::find_if(
		mParts.begin(), mParts.end(), [=](const BufferPartInfo& part) {
		return part.mRegionId == regionId && part.mPurposeId == componentId;
	});
}

EnumSelectorEntries::EnumSelectorEntries(GenApi::CNodeMapRef& device,
	const std::string& selectorName)
{
	GenApi::CEnumerationPtr selector = device._GetNode(selectorName.c_str());
	GenApi::NodeList_t entries;
	selector->GetEntries(entries);
	for (GenApi::CEnumEntryPtr entry : entries)
	{
		GenApi::CEnumEntryPtr enumEntry = entry;
		mValueFromName[enumEntry->GetSymbolic().c_str()] = enumEntry->GetValue();
	}

	// [RangeA] Alex: add this ---
	mValueFromName["RangeA"] = RANGE_A_ID; // [RangeA] No in the device, fake this in the SDK ---

}

uint64_t EnumSelectorEntries::value(const std::string& name) const
{
	return mValueFromName.at(name);
}

const std::string& EnumSelectorEntries::name(uint64_t value) const
{
	auto iter = std::find_if(
		mValueFromName.begin(),
		mValueFromName.end(),
		[=](std::pair<std::string, uint64_t> p) { return p.second == value; });
	assert(iter != mValueFromName.end());
	return iter->first;
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

EXPORT_TO_DLL std::string
CAM_STATUS_str(const CAM_STATUS & e)
{
	switch (e)
	{
	case SickCam::CAM_STATUS::All_OK:							return "All_OK"             ;		break;
	case SickCam::CAM_STATUS::CAM_IS_CONNECTED:					return "CAM_IS_CONNECTED"   ;		break;
	case SickCam::CAM_STATUS::CAM_IS_STARTED:					return "CAM_IS_STARTED"     ;		break;
	case SickCam::CAM_STATUS::CAM_IS_STOPPED:					return "CAM_IS_STOPPED"     ;		break;
	case SickCam::CAM_STATUS::CAM_IS_DISCONNECTED:				return "CAM_IS_DISCONNECTED";		break;
	case SickCam::CAM_STATUS::ERROR_OPEN_CAM:					return "ERROR_OPEN_CAM"     ;		break;
	case SickCam::CAM_STATUS::ERROR_SCAN_EMPTY_DEVICE:			return "ERROR_SCAN_EMPTY_DEVICE";	break;
	case SickCam::CAM_STATUS::ERROR_CTI_NOT_FOUND:				return "ERROR_CTI_NOT_FOUND";		break;
	case SickCam::CAM_STATUS::ERROR_OPEN_CONSUMER:				return "ERROR_OPEN_CONSUMER";		break;
	case SickCam::CAM_STATUS::ERROR_OPEN_TL_HANDLE:				return "ERROR_OPEN_TL_HANDLE";		break;
	case SickCam::CAM_STATUS::ERROR_EMPTY_IF_ID:				return "ERROR_EMPTY_IF_ID"  ;		break;
	case SickCam::CAM_STATUS::ERROR_EMPTY_IF_HANDLE:			return "ERROR_EMPTY_IF_HANDLE";		break;
	case SickCam::CAM_STATUS::ERROR_CSV_PATH:					return "ERROR_CSV_PATH"     ;		break;
	case SickCam::CAM_STATUS::ERROR_CALIBRATION_PATH:			return "ERROR_CALIBRATION_PATH";	break;
	case SickCam::CAM_STATUS::ERROR_EMPTY_PATH:					return "ERROR_EMPTY_PATH"   ;		break;
	case SickCam::CAM_STATUS::ERROR_EMPTY_IMG:					return "ERROR_EMPTY_IMG"    ;		break;
	case SickCam::CAM_STATUS::ERROR_SET_CAM:					return "ERROR_SET_CAM"      ;		break;
	case SickCam::CAM_STATUS::ERROR_CLO_CAM:					return "ERROR_CLO_CAM"      ;		break;
	case SickCam::CAM_STATUS::ERROR_GET_IMG:					return "ERROR_GET_IMG"      ;		break;
	case SickCam::CAM_STATUS::ERROR_GET_SEN_IMG:				return "ERROR_GET_SEN_IMG"  ;		break;
	case SickCam::CAM_STATUS::ERROR_RST_CAM:					return "ERROR_RST_CAM"      ;		break;
	case SickCam::CAM_STATUS::ERROR_UPDATE_PARA:				return "ERROR_UPDATE_PARA"  ;		break;
	case SickCam::CAM_STATUS::ERROR_STOP_ACQUISITION:			return "ERROR_STOP_ACQUISITION";		break;
	case SickCam::CAM_STATUS::ERROR_START_ACQUISITION:			return "ERROR_START_ACQUISITION";		break;
	case SickCam::CAM_STATUS::ERROR_PARAMETER_FORMAT_DISMATCH:	return "ERROR_PARAMETER_FORMAT_DISMATCH";break;
	case SickCam::CAM_STATUS::ERROR_PARAMETER_VALUE_DISMATCH:	return "ERROR_PARAMETER_VALUE_DISMATCH";break;
	case SickCam::CAM_STATUS::ERROR_PARAMETER_READ_ONLY:		return "ERROR_PARAMETER_READ_ONLY";		break;
	case SickCam::CAM_STATUS::ERROR_PARAMETER_INVALID:			return "ERROR_PARAMETER_INVALID";		break;
	case SickCam::CAM_STATUS::ERROR_PARAMETER_VALUE_INVALID:	return "ERROR_PARAMETER_VALUE_INVALID";	break;
	case SickCam::CAM_STATUS::ERROR_PARAMETERS_EMPTY:			return "ERROR_PARAMETERS_EMPTY";		break;
	case SickCam::CAM_STATUS::ERROR_SCAN_PARAMETERS:			return "ERROR_SCAN_PARAMETERS";		break;
	case SickCam::CAM_STATUS::ERROR_COMMUNICATION:				return "ERROR_COMMUNICATION";		break;
	case SickCam::CAM_STATUS::ERROR_SET_IP:						return "ERROR_SET_IP";				break;
	case SickCam::CAM_STATUS::ERROR_SET_SUBNET:					return "ERROR_SET_SUBNET";			break;
	case SickCam::CAM_STATUS::ERROR_NULL_PTR_DEV:				return "ERROR_NULL_PTR_DEV";		break;
	case SickCam::CAM_STATUS::ERROR_NULL_DEV_HANDLE:			return "ERROR_NULL_DEV_HANDLE";		break;
	case SickCam::CAM_STATUS::ERROR_NULL_DS_HANDLE:				return "ERROR_NULL_DS_HANDLE";		break;
	case SickCam::CAM_STATUS::ERROR_OPEN_DATASTREAM:			return "ERROR_OPEN_DATASTREAM";		break;
	case SickCam::CAM_STATUS::ERROR_CAM_NOT_CONNECT:			return "ERROR_CAM_NOT_CONNECT";		break;
	case SickCam::CAM_STATUS::ERROR_CAM_NOT_START:				return "ERROR_CAM_NOT_START";		break;
	case SickCam::CAM_STATUS::ERROR_CAM_NOT_STOP:				return "ERROR_CAM_NOT_STOP";		break;
	case SickCam::CAM_STATUS::ERROR_CAM_NOT_DISCONNECT:			return "ERROR_CAM_NOT_DISCONNECT";	break;
	case SickCam::CAM_STATUS::ERROR_CAM_IS_CONNECTED:			return "ERROR_CAM_IS_CONNECTED";	break;
	case SickCam::CAM_STATUS::ERROR_CAM_IS_STARTED:				return "ERROR_CAM_IS_STARTED";		break;
	case SickCam::CAM_STATUS::ERROR_CAM_IS_STOPPED:				return "ERROR_CAM_IS_STOPPED";		break;
	case SickCam::CAM_STATUS::ERROR_CAM_IS_DISCONNECTED:		return "ERROR_CAM_IS_DISCONNECTED";	break;
	case SickCam::CAM_STATUS::ERROR_CAM_IS_UNREACHABLE:			return "ERROR_CAM_IS_UNREACHABLE";	break;
	case SickCam::CAM_STATUS::ERROR_CAM_IS_OCCUPIED:			return "ERROR_CAM_IS_OCCUPIED";		break;
	case SickCam::CAM_STATUS::ERROR_CAM_IS_LOST:			    return "ERROR_CAM_IS_LOST";			break;
	case SickCam::CAM_STATUS::ERROR_IMAGE_NOT_READY:			return "ERROR_IMAGE_NOT_READY";		break;
	case SickCam::CAM_STATUS::ERROR_USER_FILE_NOT_EXIST:		return "ERROR_USER_FILE_NOT_EXIST";	break;
	case SickCam::CAM_STATUS::ERROR_USER_FILE_SEND_FAILED:		return "ERROR_USER_FILE_SEND_FAILED";		break;
	case SickCam::CAM_STATUS::ERROR_USER_FILE_NOT_FOUND_IN_PC:	return "ERROR_USER_FILE_NOT_FOUND_IN_PC";	break;
	case SickCam::CAM_STATUS::ERROR_USER_SET_NOT_FOUND:			return "ERROR_USER_SET_NOT_FOUND"		;	break;
	case SickCam::CAM_STATUS::ERROR_OPERATION_NOT_ALLOW:		return "ERROR_OPERATION_NOT_ALLOW";	break;
	case SickCam::CAM_STATUS::WARN_IMAGE_LOST:					return "WARN_IMAGE_LOST";			break;
	case SickCam::CAM_STATUS::TIME_OUT:							return "TIME_OUT";					break;
	case SickCam::CAM_STATUS::NO_CHANGE_IN_NETWORK:				return "NO_CHANGE_IN_NETWORK";		break;
	case SickCam::CAM_STATUS::UNKNOWN:							return "UNKNOWN";					break;
	case SickCam::CAM_STATUS::DEFAULT:							return "DEFAULT";					break;
	default:
		break;
	}
	return std::to_string(int(e));
}


}