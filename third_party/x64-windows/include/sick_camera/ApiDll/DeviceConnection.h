/**	@file	DeviceConnection.h
*
*	@brief	Class DeviceConnection.
*
*	@attention
*		Used to Open & close Device \ Open & close Datastream \ connect \ start \ stop \ disconnect camera.
*		Call GenICam API and SICK API.
*		This class will be used by Class Ranger3Shared and Ranger3.
*
*
*	@copyright	Copyright 2016-2020 SICK AG. All rights reserved.
*	@author		Vision Lab, SICK GCN
*
*/


#pragma once


#include "Typedef.h"
#include <GenApi/ChunkAdapterGEV.h>

#include <string>
#include <memory>
#include <set>
#include <map>
#include <iomanip>

#define MAX_N_NETCARD 100


namespace SickCam 
{

typedef std::string DeviceId;
typedef std::string InterfaceId;
typedef std::vector<std::pair<InterfaceId, std::string>> InterfaceList;
typedef std::vector<std::pair<DeviceId, std::string>> DeviceList;

//////////////////////////////////////////////////////////////////////////////////////////

InterfaceList 
_getInterfaces			(const GenTL::TL_HANDLE hTL,		const SPtr<GenTLApi> mTl);

InterfaceId
_findInterfaceByIndex	(const InterfaceList & interfaces,	const uint32_t & index);

GenTL::IF_HANDLE 
_openInterfaceById		(const InterfaceId & interfaceId,	const GenTL::TL_HANDLE mTlHandle, const SPtr<GenTLApi> mTl);

void 
_closeInterface			(const GenTL::IF_HANDLE interfaceHandle, const SPtr<GenTLApi> mTl);


DeviceList
_getDevices_is_changed	(const GenTL::IF_HANDLE ifHandle, const SPtr<GenTLApi> mTl);

DeviceList 
_getDevices				(const GenTL::IF_HANDLE ifHandle,	const SPtr<GenTLApi> mTl);

GenTL::TL_HANDLE 
_TL_HANDLE_open			(const SPtr<GenTLApi> mTl);

/** From 3050, common::consumer. */
Str 
_findDeviceByIndex		(const SiDeviceList & devices,			const uint32_t & index);

/** From 3050, common::consumer. */
GDeviceH 
_openDeviceById			(const GenTL::TL_HANDLE interfaceHandle, const Str& deviceId, const SPtr<GenTLApi> mTl, GenTL::GC_ERROR &status);

/** @brief IP, SUBMASK and interfaceName. */
EXPORT_TO_DLL std::vector<std::vector<Str>>
getLocalIP				();

EXPORT_TO_DLL bool 
checkIpSegments			(const std::string & _ip1,const std::string & _submask1, const std::string & _ip2, const std::string & _submask2);
//////////////////////////////////////////////////////////////////////////////////////////


/** @brief To deal with GenIRanger and Common library directly. \n
* \n
on_lost_device : Lost device, reconnect after 100 ms.
on_lost_device : CAM_IS_DISCONNECTED
* 
* @note	Midifed based on the code in MultiCamera.cpp. Called by Ranger3Shared.
*/
class EXPORT_TO_DLL DeviceConnection
{
public:
	DeviceConnection				(const std::string & unsupportedModelName); // make it as unsupported GenICam devices, mDeviceName = ""
	DeviceConnection				(
									ConsumerPtr			consumer, 
									GenTL::IF_HANDLE	interfaceHandle,
									const SiDeviceList&	devices,
									const int64_t&		id,
									const std::string & interfaceName);

	~DeviceConnection				();

	bool openDevice					(); // necessary
	bool closeDevice				();

	bool openDataStream				();
	bool closeDataStream			();

	bool isReachable				() { return mIsReachable; };
	bool isOccupied					() { return mIsOccupied; };


	/** Allocates, announces and queues a number of buffers to the data stream */
	void initializeBuffers			(size_t buffersCount, size_t payloadSize);
	void teardownBuffers			();
    void teardownBuffersWhenLost	();

	void registerNewBufferEvent		();
	void unregisterNewBufferEvent	();

	void startAcquisition			();
	void stopAcquisition			();
	bool isAcquisitionRunning		()		const;

	void createDeviceNodeMap		(SiConsumer& consumer);
	void createDataStreamNodeMap	(SiConsumer& consumer);

	
	VecBufferHandle		&		getBufferHandles	()	{	return mBufferHandles;	}
	Vec<uint8_t*>		&		getBufferData		()	{	return mBufferData;		}

	void updateIp					();
	void updateSubnet				();
	Str getIp						() const { return mDeviceIP; };
	Str getSubnet					() const { return mDeviceSubnet; };
	Str getMac						() const { return mDeviceMac; };
	Str getSN						() const { return mDeviceSN; };

protected:
	void _queryIP();
	void _querySubnet();
	void _queryMAC();
	void _querySN();

private:
	bool						mAcquisitionRunning;	//! < Is started
	bool						mIsReachable;
	bool						mIsOccupied;
	bool						mIsTrispector;
	Str							mDeviceIP;
	Str							mDeviceSubnet;
	Str							mDeviceMac;
	Str							mDeviceSN;
	ConsumerPtr					mpConsumer;
	Vec<GBufferH>				mBufferHandles;
	Vec<uint8_t*>				mBufferData; // need delete[]
	SPtr<SiGenTLPort>			mDevicePort;
	SPtr<SiGenTLPort>			mDataStreamPort;
	GenApi::CIntegerPtr			mDeviceSelector;


public:
	int64_t						mId;					//! < id of current device, marked in Ranger3Shard::scan(), will be used in queryIP(). 
	Str							mDeviceId;				//! < string id of current device, take place of mId.
	Str							mDeviceName;			//! < device name to open the device. 
	Str							mUnsupportedDeviceModel; //! < the model name of unsupported device. Make it as a mark.
	std::stringstream			mLastMessage;			//! < Error message when some functions return false; 
	SPtr<SiGenTLPort>			mInterfacePort;
	GenTL::TL_HANDLE			mInterfaceHandle;
    std::string                 mInterfaceName;
	GDeviceH					mDeviceHandle;
	GenTL::DS_HANDLE			mDataStreamHandle;
	GenTL::EVENT_HANDLE			mNewBufferEventHandle;
	GNodeMap					mDeviceNodeMap;			//! < Node map for reading/setting device parameters
	GNodeMap					mDataStreamNodeMap;		//! < Node map for data stream module to boost receiver thread priority
	GNodeMap					mInterfaceNodeMap;
    bool						mIsInitialized;			//! < Is initialized, supported 
	bool						mIsOpenDataStream;		//! < Is connected (4 status in Ranger3 class)
#ifndef __linux__
	NodeMap						mDeviceNode;			//! < Node map for reading/setting device parameters
	NodeMap						mDataStreamNode;		//! < Node map for data stream module to boost receiver thread priority
	NodeMap						mInterfaceNode;
	SPtr<GenTLApi>				mTl;
#else
	GenTLApi *                  mTl;
#endif

    };


//////////////////////////////////////////////////////////////////////////////////////////


/** @brief Copy from 3050 code \n
* 
* 
* @note Here is the original comments \n
* Wrapper class for a GigE Vision chunk adapter. Creating an instance\n
* of this class and attaching it to a node map and a buffer will make\n
* sure you can access chunk meta data via the node map.\n
*/
class ChunkAdapter
{
public:
    ChunkAdapter(GenTLApi* tl, GenTL::DS_HANDLE dataStreamHandle);
    ChunkAdapter(SPtr<GenTLApi> tl, GenTL::DS_HANDLE dataStreamHandle);

	/** Attach a nodemap to the adapter to allow accessing the metadata
	via the node map.
	*/
	void attachNodeMap(GenApi::INodeMap* nodeMap);

	/** Detach the nodemap from the adapter. */
	void detachNodeMap();

	/** Attach the chunk adapter to a specific buffer. This allows
	access to the chunk metadata carried in the buffer.
	*/
	void attachBuffer(GenTL::BUFFER_HANDLE handle, uint8_t* buffer);
	/** Detach the from the buffer when done with it. */
	void detachBuffer();

private:
	/**
	Get the actual chunk payload size from a buffer. This is needed to
	allow the chunk adapter to find the chunk trailer information when
	attaching.
	*/
	size_t getChunkPayloadSize(GenTL::BUFFER_HANDLE handle);

private:
	SPtr<GenTLApi> mTl;
    GenTLApi *mTl_rawPtr;
	GenTL::DS_HANDLE mDataStreamHandle;
	std::unique_ptr<GenApi::CChunkAdapterGEV> mAdapter;
};


//////////////////////////////////////////////////////////////////////////////////////////

class ChunkScanLineValueAccessor
{
public:
	ChunkScanLineValueAccessor(GenApi::CIntegerPtr chunkScanLineSelector,
		GenApi::CIntegerPtr node);

	uint64_t getValue64(const uint8_t* buffer, size_t lineIndex);
	uint32_t getValue32(const uint8_t* buffer, size_t lineIndex);

private:
	inline static uint64_t createBitMask64(size_t lsb, size_t msb)
	{
		assert(lsb < 64);
		assert(msb <= lsb);
		return (0xffffffffffffffff >> (63 - lsb + msb)) << (63 - lsb);
	}

	inline static uint32_t createBitMask32(size_t lsb, size_t msb)
	{
		assert(lsb < 32);
		assert(msb <= lsb);
		return (0xffffffff >> (31 - lsb + msb)) << (31 - lsb);
	}

	inline uint64_t getRange64(uint64_t number)
	{
		return (mMask & number) >> (63 - mLsb);
	}

	inline uint32_t getRange32(uint32_t number)
	{
		return (static_cast<uint32_t>(mMask) & number) >> (31 - mLsb);
	}

private:
	size_t mBase;
	size_t mIncrement;
	size_t mRegisterSize;
	uint64_t mMask;
	size_t mLsb;
};

/** @brief Copy from 3050 code \n
* \n
* @note Here is the original comments \n
* Extracts metadata from chunk features given a NodeMap.\n
* \n
* This implementation doesn't use the GenApi standard way to iterate through\n
* all the scan lines, by setting the ChunkScanLineSelector for every line.\n
* \n
* Instead this class will calculate the base address and offset for every\n
* metadata feature, and assume that the offset is constant between every line.\n
* This is fairly safe assumption for these features since they are selected\n
* by a single selector.\n
* \n
* The registers will then be accessed directly via the ChunkPort, thereby\n
* skipping the node map, selectors and swissknife calculations.\n
*/
class FastMetadataExtractor
{
public:
	FastMetadataExtractor(GenApi::CNodeMapRef device);
	std::vector<Ranger3LineMetadata> extract();

private:
	GenApi::CIntegerPtr mChunkScanLineSelector;
	ChunkScanLineValueAccessor mTimestamp;
	ChunkScanLineValueAccessor mEncoderValue;
	ChunkScanLineValueAccessor mOvertriggerCount;
	ChunkScanLineValueAccessor mFrameTriggerActive;
	ChunkScanLineValueAccessor mLineTriggerActive;
	ChunkScanLineValueAccessor mEncoderResetActive;
	ChunkScanLineValueAccessor mEncoderA;
	ChunkScanLineValueAccessor mEncoderB;
	GenApi::CChunkPortPtr mChunkPort;
};


//////////////////////////////////////////////////////////////////////////////////////////

typedef uint32_t PixelFormat;

class BufferInfo
{
public:
    BufferInfo(
#ifdef __linux__
		GenTLApi* tl,
#else
		SPtr<GenTLApi> tl,
#endif
		GenTL::DS_HANDLE dataStreamHandle, GenTL::BUFFER_HANDLE bufferHandle);

public:
	bool8_t		mBufferIncomplete;
	size_t		mBufferFilledSize;
	uint64_t	mBufferFrameID;
	uint8_t*	mDataPointer;
	uint64_t	mDataFormat;
	size_t		mBufferDataSize;
	size_t		mDataWidth;
	size_t		mDataHeight;
	size_t		mBufferPayloadType;
	uint32_t	mPartCount;
	uint64_t	mDeliveredLineCount = 0; // VLD-21
};

class BufferPartInfo
{
public:
    BufferPartInfo(
#ifdef __linux__
		GenTLApi* tl,
#else
		SPtr<GenTLApi> tl,
#endif
		GenTL::DS_HANDLE dataStreamHandle, GenTL::BUFFER_HANDLE bufferHandle, uint32_t partNumber);

public:
	uint32_t	mPartNumber;
	uint8_t*	mPartDataPointer;
	size_t		mPartDataType;
	uint64_t	mPartDataFormat;
	size_t		mPartDataSize;
	size_t		mPartDataWidth;
	size_t		mPartDataHeight;
	uint64_t	mRegionId;
	uint64_t	mPurposeId;

};

class PartInfoCollection
{
public:

	/** @brief Create a BufferPartInfoCollection with the parts in a buffer.
	* @param tl The GenTLApi
	* @param dataStreamHandle Handle to the data stream
	* @param bufferHandle Handle to the buffer to get the parts from
	* @param bufferPartCount The number of parts available in the buffer
	*/
	static PartInfoCollection fromBuffer(
#ifdef __linux__
		GenTLApi* tl,
#else
		SPtr<GenTLApi> tl,
#endif	
		GenTL::DS_HANDLE dataStreamHandle,
										GenTL::BUFFER_HANDLE bufferHandle, uint32_t bufferPartCount);

	/** Find the part with a specific component in a specific region.
	\param regionId The Region which the Component belongs to
	\param componentId The Component to find the part for
	*/
	const BufferPartInfo& findPart(size_t regionId, size_t componentId) const;

	/** Check if there is a part with a specific component in a specific region.
	\param regionId The Region which the Component belongs to
	\param componentId The Component to look for
	*/
	bool hasPart(size_t regionId, size_t componentId) const;

private:
	PartInfoCollection() {}
	std::vector<BufferPartInfo>::const_iterator	findPartIterator(size_t regionId, size_t componentId) const;

public:
	std::vector<BufferPartInfo> mParts;
};

/** A class to map selector values to names and vice versa. */
class EnumSelectorEntries
{
public:
	/** Finds all the name, value pair entries for an enum selector and
	keeps them for easy look up.
	*/
	EnumSelectorEntries(GenApi::CNodeMapRef& device,
		const std::string& selectorName);

	/** Get value from enumerator name. */
	uint64_t value(const std::string& name) const;

	/** Get enumerator name from value. */
	const std::string& name(uint64_t value) const;

private:
	std::map<std::string, uint64_t> mValueFromName;
};

//////////////////////////////////////////////////////////////////////////////////////////

EXPORT_TO_DLL std::string
CAM_STATUS_str(const CAM_STATUS & e);

//////////////////////////////////////////////////////////////////////////////////////////






};



