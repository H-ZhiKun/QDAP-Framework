/**	@file	Ranger3.h
*
*	@brief	Class Ranger3 Head File.
*
*	@attention
*	This is the Ranger3 GenIcam API wrapper.
*
*
*	@copyright	Copyright 2016-2023 SICK AG. All rights reserved.
*	@author		Vision Lab, SICK GCN
*
*
*   @notes    Parameter example of FW 2.6
*
<parameters format="Ranger3ParameterCSV">
	#Version,1
	#Root
	#DeviceControl
		DeviceScanType,Linescan3D
		DeviceLinkThroughputLimit,118750000
		DeviceLinkHeartbeatTimeout,3000
	#ImageFormatControl
		ExposureTime_RegionSelector_Region0,100
		ExposureTime_RegionSelector_Region1,100
		Width_RegionSelector_Region1,2560
		Height_RegionSelector_Region1,672
		Height_RegionSelector_Scan3dExtraction1,1000
		OffsetX_RegionSelector_Region1,0
		OffsetY_RegionSelector_Region1,60
		ComponentEnable_ComponentSelector_Reflectance_RegionSelector_Scan3dExtraction1,0
		ComponentEnable_ComponentSelector_Scatter_RegionSelector_Scan3dExtraction1,0
		PixelFormat_ComponentSelector_Range_RegionSelector_Scan3dExtraction1,Coord3D_C16
		PixelFormat_ComponentSelector_Scatter_RegionSelector_Scan3dExtraction1,Mono16
		TestPattern_ComponentSelector_Intensity_RegionSelector_Region0,Off
		TestPattern_ComponentSelector_Range_RegionSelector_Scan3dExtraction1,Off
	#Scan3dControl
		Scan3dExtractionMethod_Scan3dExtractionSelector_Scan3dExtraction1,Hi3D
		WamSize_Scan3dExtractionSelector_Scan3dExtraction1,Normal
		SearchMode3D_Scan3dExtractionSelector_Scan3dExtraction1,GlobalMax
		RangeAxis_Scan3dExtractionSelector_Scan3dExtraction1,Reverse
		DetectionThreshold_Scan3dExtractionSelector_Scan3dExtraction1,20
		SearchDirection_Scan3dExtractionSelector_Scan3dExtraction1,Standard
		ReflectanceFilter_Scan3dExtractionSelector_Scan3dExtraction1,1
		ScatterMode_Scan3dExtractionSelector_Scan3dExtraction1,SymmetricSideBand
		ScatterOffset_Scan3dExtractionSelector_Scan3dExtraction1,6
		ScatterWidth_Scan3dExtractionSelector_Scan3dExtraction1,4
		ScatterGain_Scan3dExtractionSelector_Scan3dExtraction1,4
		ScatterReductionCurve_Scan3dExtractionSelector_Scan3dExtraction1,Linear
		ScatterReductionLowerLimit_Scan3dExtractionSelector_Scan3dExtraction1,0
		ScatterReductionUpperLimit_Scan3dExtractionSelector_Scan3dExtraction1,1500
		ScatterReductionKnee1Input_Scan3dExtractionSelector_Scan3dExtraction1,22
		ScatterReductionKnee1Output_Scan3dExtractionSelector_Scan3dExtraction1,12
		ScatterReductionKnee2Input_Scan3dExtractionSelector_Scan3dExtraction1,54
		ScatterReductionKnee2Output_Scan3dExtractionSelector_Scan3dExtraction1,39
		Scan3dOutputMode_Scan3dExtractionSelector_Scan3dExtraction1,UncalibratedC
	#AcquisitionControl
		AcquisitionMode,Continuous
		AcquisitionFrameRate,20
		AcquisitionLineRate,1000
		TriggerMode_TriggerSelector_LineStart,Off
		TriggerMode_TriggerSelector_FrameStart,Off
		TriggerMode_TriggerSelector_AreascanFrameStart,Off
		TriggerSource_TriggerSelector_LineStart,Encoder
		TriggerSource_TriggerSelector_FrameStart,FrameTriggerInput
		TriggerSource_TriggerSelector_ExposureStart,LineStart
		TriggerSource_TriggerSelector_AreascanFrameStart,Encoder
		TriggerActivation_TriggerSelector_FrameStart,RisingEdge
		TriggerDelay_TriggerSelector_ExposureStart,0
		MultiSlopeMode,PresetMedium
		MultiSlopeKneePointCount,1
	#DigitalIOControl
		LineInverter_LineSelector_LaserStrobe1Output,1
		LineInverter_LineSelector_LaserStrobe2Output,1
		LineFormat_LineSelector_EncoderAInput,RS422
		LineFormat_LineSelector_EncoderBInput,RS422
	#TimerControl
		TimerDuration_TimerSelector_LaserStrobe1Timer,0
		TimerDuration_TimerSelector_LaserStrobe2Timer,0
		TimerDelay_TimerSelector_LaserStrobe1Timer,0
		TimerDelay_TimerSelector_LaserStrobe2Timer,0
		TimerTriggerSource_TimerSelector_LaserStrobe1Timer,Off
		TimerTriggerSource_TimerSelector_LaserStrobe2Timer,Off
	#EncoderControl
		EncoderResolution,0.00400000019
		EncoderMode,FourPhase
		EncoderDivider,1
		EncoderOutputMode,DirectionUp
		EncoderResetSource,Off
	#EventControl
		EventNotification_EventSelector_LogMessage,On
	#LogMessageData
	#EventTestData
	#UserSetControl
	#FileAccessControl
	#ChunkDataControl
		ChunkModeActive,0
	#TestControl
	#TransportLayerControl
	#GigEVision
		GevSCPD,0
	#Capabilities
	#ProductConfiguration
	#FirmwareUpdate
	#FrameBufferStatus
	#PartSyncStatus
	#MultipartFrontendStatus
	#DescramblerStatus
	#PostProcessingStatus
	#ImagerInterfaceStatus
	#M30Debug
	#M30RegisterPeekPoke
	#DataDumper
	#LogEventTest
	#DataTypesTest
	#ValidationTest
	#AvailableTest
</parameters>
*/

#pragma once

#include "CameraShared.h"


#ifdef CALLBACK_NEW
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>


#ifdef _WIN32
#define SICK_CALLBACK __stdcall
#else
#define SICK_CALLBACK
#endif

#endif

#ifndef DISABLE_CAL_IN_PC
typedef SiCaliWrapper::RectMethod				RectMethod;
typedef SiCaliWrapper::CalibrationWrapper		CaliWrapper;
typedef UPtr<CaliWrapper>						CalibrationPtr;
#endif

namespace SickCam
{
typedef ChunkAdapter				SiChunkAdapter;
typedef UPtr<SiChunkAdapter>		ChunkPtr;
typedef FastMetadataExtractor		ChunkDataExtractor;
typedef UPtr<PartInfoCollection>	PartInfosPtr;
typedef SPtr<DeviceConnection>		DCPtr;
typedef CameraShared				Ranger3Shared;
typedef SPtr<Ranger3Shared>			R3SPtr;

////////////////////////////////////////////////////////////////////////////////

#ifdef CALLBACK_NEW
typedef std::function<void(ImgT* img, void* any)>        CallbackEvent_GetImage;
typedef std::function<void(std::string* name, std::string* ip, std::string* mac, std::string* msg, void* any)>  CallbackEvent_HeartBeats;
#endif // CALLBACK_NEW

#ifdef CALLBACK_THREAD_POOL
class ThreadPool
{
public:
ThreadPool(size_t threads)
{
	m_writeLog = isEnvironmentVariableExisted("R3DEBUG");
	if (m_writeLog)
	{
		std::cout << "Enable log output in ThreadPool." << std::endl;
	}

	workers.resize(threads);
	threadStatus.resize(threads, true); // Initially all threads are free

	for (size_t i = 0; i < threads; ++i)
	{
		workers[i] = std::thread(
			[this, i]
			{
				for (;;)
				{
					std::function<void()> task;

					{
						std::unique_lock<std::mutex> lock(this->queue_mutex);
						this->condition.wait(lock,
							[this] { return this->stop || !this->tasks.empty(); });

						if (this->stop && this->tasks.empty())
							return;

						task = std::move(this->tasks.front());
						this->tasks.pop_front();
						this->threadStatus[i] = false; // Thread is busy
					}
					task();
					{
						std::unique_lock<std::mutex> lock(this->queue_mutex);
						this->threadStatus[i] = true; // Thread is free
					}
				}
			}
			);
	}
}

template<typename ReturnType>
auto enqueue(std::function<ReturnType()> func) -> std::future<ReturnType>
{
	auto task = std::make_shared<std::packaged_task<ReturnType()>>(func);

	std::future<ReturnType> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(queue_mutex);

		if (stop)
			throw std::runtime_error("enqueue on stopped ThreadPool");

		// Count and output the number of free threads
		if (m_writeLog)
		{
			int freeThreads = std::count(threadStatus.begin(), threadStatus.end(), true);
			std::cout << "Free threads in pool: " << freeThreads << "(all" << threadStatus.size() << ")" << std::endl;
		}

		// Check if all threads are busy
		if (std::none_of(threadStatus.begin(), threadStatus.end(), [](bool status) { return status; }))
		{
			std::cerr << "!!!!! Error !!!!! No free thread in pool! Please set more threads or enhance the hardware and try again!" << std::endl;
		}

		tasks.emplace_back([task]() { (*task)(); });
	}
	condition.notify_one();
	return res;
}

~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}
	condition.notify_all();
	for (std::thread& worker : workers)
	{
		if (worker.joinable())
			worker.join();
	}
}

private:
	std::vector<std::thread> workers;
	std::deque<std::function<void()>> tasks;
	std::vector<bool> threadStatus; // Track if each thread is free or busy

	std::mutex queue_mutex;
	std::condition_variable condition;
	bool stop = false;
	bool m_writeLog = false;
};

#endif // CALLBACK_THREAD_POOL


/**	
* @brief 每个 Ranger3 对应应该初始化为一个 类Ranger3 的对象。\n\n
* Each Ranger3 device corresponding to a Ranger3 class object.
*/
class Ranger3 final
{
public:
	/** 
	* @brief 使用 IP 或者 MAC 地址指定相机。 \n\n
	* Specify a device with IP or MAC.
	* \n
	* @param [in] RS		智能指针shared_ptr，Ranger3 的运行环境。 Shared ptr, the GenICam environment. 
	* @param [in] ip_mac	IP 或者 MAC 地址。IP地址可以通过Ranger3Studio 或者 MFC demo 中找到。请看 R3S::scanDevice() 中展示的扫描过程。
	*	IP 示例："192.168.0.12"。Mac 地址可以在设备的标签上找到。Mac 示例（大小写不敏感）："00:06:77:0c:69:e1"。 \n
	*	IP / Mac address. IP can be found by Ranger3Studio or MFC demo. Please see R3S::scanDevice(). 
	*	IP example: "192.168.0.12". Mac address can be found on the surface of device. Mac example （Lower）: "00:06:77:0c:69:e1"
	* @param [in] useIP		置为 true 表示使用 IP 地址。  Set true to use IP.
	*/
	EXPORT_TO_DLL	Ranger3		(const SPtr<Ranger3Shared> & RS, cStr & ip_mac, const bool useIP);
	
	EXPORT_TO_DLL	~Ranger3	();

	/**
	* @brief 获取初始化结果。\n\n
	* To get initialization result.
	* \n
	* @return
	* - CAM_STATUS::ERROR_NULL_PTR_DEV	初始化失败。  Initialize failed.
	* - CAM_STATUS::All_OK				
	*/
	EXPORT_TO_DLL CAM_STATUS	isReady();

	////////////////////////////////////////////////////////////////////////////////


	/** 
	* @brief 连接相机 \n\n 
	* Connect a camera.
	* \n
	* @note 连接相机的动作包括：打开设备句柄，打开数据流句柄，加载参数文件，扫描相机参数。
	* 当相机连接时，采集图像过程并没有开始。\n\n
	* It will open device handle, open datastream, loading parameters file, scan parameters in device.
	* When camera is connected, image acquition is not started.
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED	相机处于 start 状态。  Camera is started.
	* - CAM_STATUS::ERROR_SCAN_PARAMETERS	扫描相机参数时发生错误。 Error happened when scanning parameters in device.
	*/
    EXPORT_TO_DLL CAM_STATUS	connectCamera		();


	/**
	* @brief 开始采集 \n\n
	* Start acquisition.
	* \n
	* @note 开始采集的动作包括：加载标定文件，申请相机缓存，锁定相机参数，开始采集图像。\n\n
	* It will load calibration file, initialize device buffer, lock device parameter, start acquisition.
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。  Camera is disconnected.
	* - CAM_STATUS::ERROR_START_ACQUISITION		相机开始采集失败。  Failed to start acquisition.
	* - CAM_STATUS::ERROR_CALIBRATION_PATH		使用标定文件初始化标定变量错误。 Error happened when initializing calibration variables.
	*/
	EXPORT_TO_DLL CAM_STATUS	startCamera			();

#ifdef CALLBACK_NEW

    /**
    * @brief 连接相机（设定心跳值） \n\n
    * Connect a camera.
    * \n
    * @note 连接相机的动作包括：打开设备句柄，打开数据流句柄，加载参数文件，扫描相机参数。
    * 当相机连接时，采集图像过程并没有开始。\n\n
    * It will open device handle, open datastream, loading parameters file, scan parameters in device.
    * When camera is connected, image acquisition is not started.
    * \n
    * \n
    * @param [in] pCallback		当失去心跳时，将调用由用户定义的回调函数。相应的处理可以在此函数中执行。详见示例程序
	* @param [in] microSecond	读取心跳的时间间隔，单位毫秒，推荐值为 10 000;
	* @param [in] any			在失去心跳的响应函数（CallbackEvent_HeartBeats）中，该指针将作为输入参数，由用户定义。 The parameters will be passed into [CallbackEvent_HeartBeats];
    * \n
    * @return
    * - CAM_STATUS::All_OK
    * - CAM_STATUS::ERROR_CAM_IS_STARTED	相机处于 start 状态。  Camera is started.
	* - CAM_STATUS::ERROR_SCAN_PARAMETERS	扫描相机参数时发生错误。 Error happened when scaning parameters in device.
	* - CAM_STATUS::ERROR_COMMUNICATION		相机连接发异常，导致通讯失败。 Communication problems.
	* - CAM_STATUS::UNKNOWN					捕捉到未知异常。 Unknown exception.
    */
    EXPORT_TO_DLL CAM_STATUS	connectCamera		(CallbackEvent_HeartBeats pCallback, const uint32_t& microSecond, void * any=nullptr);

	/**
	* @brief 【测试中，暂不建议使用】连接相机 \n\n
	* Connect a camera.
	* \n
	* @note 连接相机的动作包括：打开设备句柄，打开数据流句柄，加载参数文件，扫描相机参数。
	* 当相机连接时，采集图像过程并没有开始。\n\n
	* It will open device handle, open datastream, loading parameters file, scan parameters in device.
	* When camera is connected, image acquisition is not started.
	* \n
	* \n
	* @param [in] pCallback		当失去心跳时，将调用由用户定义的回调函数。相应的处理可以在此函数中执行。详见示例程序
	* @param [in] microSecond	读取心跳的时间间隔，单位毫秒;
	* @param [in] numCallbackThreadsInPool		在取图像的 callback 函数中可使用的最大线程数量，推荐为 CPU 核数的 2 倍。
	* @param [in] any			在失去心跳的响应函数（CallbackEvent_HeartBeats）中，该指针将作为输入参数，由用户定义。 The parameters will be passed into [CallbackEvent_HeartBeats];
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED	相机处于 start 状态。  Camera is started.
	* - CAM_STATUS::ERROR_SCAN_PARAMETERS	扫描相机参数时发生错误。 Error happened when scaning parameters in device.
	* - CAM_STATUS::ERROR_COMMUNICATION		相机连接发异常，导致通讯失败。 Communication problems.
	* - CAM_STATUS::UNKNOWN					捕捉到未知异常。 Unknown exception.
	*/
    EXPORT_TO_DLL CAM_STATUS	connectCamera		(CallbackEvent_HeartBeats pCallback, const uint32_t& microSecond, const size_t& numCallbackThreadsInPool, void * any=nullptr);

	/**
	* @brief 尝试重新连接相机 \n\n
	* Try to reconnect a camera.
	* \n
	* @note 当失去相机心跳时，相机的指针将暂时不可用。当重新发现相机后，可以调用此程序，恢复相机的指针。\n\n
	* \n
	* \n
	* @return
	* - CAM_STATUS::All_OK					重新连接成功。
	* - CAM_STATUS::CAM_IS_XXXXXXXXXXXX	    重新连接成功后，再调用此函数，将返回相机的状态。见 getStatus()。
	* - CAM_STATUS::CAM_IS_DISCONNECTED 	重新连接成功后，再调用此函数，将返回相机的状态。一般，相机将处于 Disconnected 状态。
	* - CAM_STATUS::ERROR_CAM_IS_LOST		丢失心跳后，相机将处于此状态。
	*/
    EXPORT_TO_DLL CAM_STATUS	reconnectCamera     ();

	/**
	* @brief 开始采集，并启动 callback 。 \n\n
	* Start acquisition and passing callback function
	* \n
	* @note 开始采集的动作包括：加载标定文件，申请相机缓存，锁定相机参数，开始采集图像。\n
	* Callback 函数中的第一个参数是返回的图像。其内存由 API 管理，在 callback 函数调用结束后，API 将释放 ImgT 中的内存。\n
	* 所以，使用者在拿到数据之后，应该第一时间做“深拷贝”。\n
	* ImgT 中的操作符 = ，已经重载为深拷贝。\n
	* \n
	* It will load calibration file, initialize device buffer, lock device parameter, start acquisition.
	* \n
	* @param [in] pCallback		Callback function pointer;
	* @param [in] any			在 callback 中要使用的参数。 The parameters will be passed into callback function;
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。			Camera is disconnected.
	* - CAM_STATUS::ERROR_START_ACQUISITION		相机开始采集失败。					Failed to start acquisition.
	* - CAM_STATUS::ERROR_CALIBRATION_PATH		使用标定文件初始化标定变量错误。	Error happened when initializing calibration variables.
	*/
	EXPORT_TO_DLL CAM_STATUS	startCamera			(CallbackEvent_GetImage pCallback, void * any=nullptr, const uint32_t& grab_sleep_time_ms=0);

#endif

	/**
	* @brief 停止采集 \n\n
	* Stop acquisition.
	* \n
	* @note 停止采集的动作包括：停止采集图像，解锁相机参数，清除相机缓存。本质上 connected 和 stopped 是一样的状态。 \n\n
	* It will stop acquisition, unlock device parameter, free device buffer. Actually, connected equals to stopped.
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。  Camera is disconnected.
	* - CAM_STATUS::ERROR_STOP_ACQUISITION		相机停止采集失败。  Failed to stop acquisition.
	*/
	EXPORT_TO_DLL CAM_STATUS	stopCamera			();

	// todo: stopCamera_WaitCallbackFinish, not finish! don't call it. -----------
	EXPORT_TO_DLL CAM_STATUS	stopCamera_WaitCallbackFinish(bool please_do_not_use_it_any_more = true);
	// todo：callback 中需要一个查找表：线程号，启动时间，状态，是否强制结束 ---------

	
	/**
	* @brief 断开相机连接 \n\n
	* Disconnect a camera.
	* \n
	* @note 断开相机连接的动作包括：关闭数据流句柄，关闭设备句柄。\n\n
	* It will close datastream, close device handle.
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED	相机处于 start 状态。  Camera is started.
	* - CAM_STATUS::UNKNOWN					未知错误。  Unknown problem.
	*/
	EXPORT_TO_DLL CAM_STATUS	disconnectCamera	();

	/**
	* @brief 【若使用 callback 采集图像，则无需调用】获取图像 \n\n
	* Get output images.
	* \n
	* @param [out] imgTable		Images data;
	* \n
	* @note 相机必须处于 start 状态时才可以采集图像。请注意，非线程安全。\n\n
	* Camera must be started! And it is not thread-safe.
	* \n
	* @return
	* - CAM_STATUS::All_OK                  成功获得图像。
	* - CAM_STATUS::TIME_OUT                超时，请重试。
	* - CAM_STATUS::ERROR_CAM_IS_STARTED	相机处于 start 状态。  Camera is started.
	*/
	EXPORT_TO_DLL CAM_STATUS	getImageData		(ImgT & imgTable);


	////////////////////////////////////////////////////////////////////////////////


	/**
	* @brief 设置 Y 方向分辨率。仅对标定后数据有效。 \n\n
	* Set scale in Y axis.
	* \n
	* @param [in] value		value = (y_range_in_mm) / (profiles_number).
	* \n
	* @note
	* 点云 y 坐标 = Y_offset + Y_scale * Y_pixelValueInRange。 \n\n
	* Y value of a point cloud = Y_offset + Y_scale * Y_pixelValueInRange.
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED		相机处于 start 状态。  Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。  Camera is disconnected.
	*/
	EXPORT_TO_DLL CAM_STATUS	setScaleY		(const double	& value)
	{
		if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
		if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;
		m_Param.m_scaleY = value;							return value == getScaleY() ? CAM_STATUS::All_OK : CAM_STATUS::UNKNOWN;
	};

	/**
	* @brief 启用相机内部标定（包含 Calibration + Rectification），标定由相机完成。这个是（推荐）做法。需要提前发送标定文件到相机中（一体式相机已在出厂时放置）。 \n\n
	* Enable in-Device calibration.\n
	* \n
	* \n
	* @param [in] rectificationWidth	输出图像的宽度（最小160，最大4096，必须是32的倍数）。
	* @param [in] range_Uint16	In-Device 标定的原始输出数据是 uin16_t 类型。设置为 false，则将其转化成 float 。
	* @param [in] enableOpenMP	当 range_Uint16 设置为 false，此参数有意义。true 表示使用 OpenMP 加速计算。这会增加 CPU 使用率。
	* \n
	* \n
	* @note \n
	* 相机内标定，最终图像的宽度，由 rectificationWidth 确定，也就是参数 [Scan3dRectificationWidth_Scan3dExtractionSelector_Scan3dExtraction1]。\n
	* 如果 [rangeTypeUint16] 设置为 true，请特别注意：标定后的 Range 数据要使用 DataNames::RAN_CAL_16 表示，是 uin16_t 类型。\n
	* \n
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED		相机处于 start 状态。  Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。  Camera is disconnected.
	* - CAM_STATUS::ERROR_OPERATION_NOT_ALLOW	无法启用相机内部标定。原因可能是相机不支持此功能。详见说明文档中列举的支持相机型号。
	*/
	EXPORT_TO_DLL CAM_STATUS	setCalibration_inDevice(const uint32_t& rectificationWidth = 2560, const bool range_Uint16 = true, const bool enableOpenMP = false);

	/**
	* @brief 启用相机内部标定（只做 Calibration，不做 Rectification），标定由相机完成。需要提前发送标定文件到相机中（一体式相机已在出厂时放置）。 \n\n
	* Enable in-Device calibration.\n
	* \n
	* \n
	* @param [in] range_Uint16			         In-Device 标定输出的 Range 数据 是 uin16_t 类型。设置为 false，则将其转化成 float 。
	* @param [in] rangeX_Uint16	                 In-Device 标定输出的 RangeX 数据 是 uin16_t 类型。设置为 false，则将其转化成 float 。
	* @param [in] enableOpenMP			         当 range_Uint16 或者 rangeX_Uint16 其中至少一个设置为 false，此参数有意义。true 表示使用 OpenMP 加速计算。这会增加 CPU 使用率。
	* \n
	* \n
	* @note \n
	* 相机内标定，最终图像的宽度，由参数 [Width_RegionSelector_Region1] 控制。\n
	* 如果 [rangeTypeUint16] 设置为 true，请特别注意：标定后的 Range 数据要使用 DataNames::RAN_CAL_16 表示，是 uin16_t 类型。\n
	* \n
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED		相机处于 start 状态。  Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。  Camera is disconnected.
	* - CAM_STATUS::ERROR_OPERATION_NOT_ALLOW	无法启用相机内部标定。原因可能是相机不支持此功能。详见说明文档中列举的支持相机型号。
	*/
	EXPORT_TO_DLL CAM_STATUS	setCalibration_inDevice_CalibrateAC(const bool range_Uint16  = true, const bool rangeX_Uint16 = true, const bool enableOpenMP = false);

	/**
	* @brief 禁用标定，从相机端只获得原始数据 \n\n
	* Disable calibration.\n
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED		相机处于 start 状态。  Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。  Camera is disconnected.
	*/
	EXPORT_TO_DLL CAM_STATUS	setCalibration_No();

	/**
	* @brief 加载相机的参数文件，csv 格式 \n\n
	* import Configuretion File.
	* \n
	* @param [in] filePath		csv 文件路径。若 value 为空，则使用相机的默认参数  csv file path. If value=="", using device inner parameters.
	* \n
	* @note 相机必须处于 connected 或者 stopped 状态。csv 参数文件必须和相机的固件版本相兼容。
	* 否则会导入失败。 \n\n
	* The camera must be connected or stopped when calling this.
	* The parameters version in CSV file must match the device. If not, it might be failed.
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED		相机处于 start 状态。			Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。		Camera is disconnected.
	* - CAM_STATUS::ERROR_CSV_PATH				无效的文件路径。				Invalid file path.
	*/
	EXPORT_TO_DLL CAM_STATUS	importConfiguretionFile(const Str		& filePath);

	/**
	* @brief 导出相机的参数文件，csv 格式 \n\n
	* export Configuretion File.
	* \n
	* @param [in] filePath		csv 文件路径。   csv file path.
	* \n
	* @note 相机必须处于 connected 或者 stopped 状态。
	* The camera must be connected or stopped when calling this function.
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED		相机处于 start 状态。			Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。	Camera is disconnected.
	* - CAM_STATUS::ERROR_CSV_PATH				无效的文件路径。				Invalid file path.
	*/
	EXPORT_TO_DLL CAM_STATUS	exportConfiguretionFile(const Str		& filePath);

	/**
	* @brief 设置相机取图的最大等待时间（会直接影响 stop 的执行速度，所以建议设小一点），默认值见 DEFAULT_TIMEOUT  \n\n
	* Set timeout.The default is DEFAULT_TIMEOUT
	* \n
	* @param [in] value		相机取图的最大等待时间，单位毫秒。   Maximum waiting time when grabbing image, unit is millimeter.
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED		相机处于 start 状态。  Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。  Camera is disconnected.
	*/
	EXPORT_TO_DLL CAM_STATUS	setTimeOut		(const uint32_t	& value)	
	{
		if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
		if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;
		m_Param.m_timeOut = value;							return value == getTimeOut() ? CAM_STATUS::All_OK : CAM_STATUS::UNKNOWN;	
	};

	/**
	* @brief 设置图像缓存的张数 \n\n
	* Set the number of image buffers.
	* \n
	* @param [in] value		图像缓存的张数。  The number of image buffers.
	* \n
	* @note 默认是 20 ，最小可以设置为 1。设置越大，内存占用越多。设置太小，有丢帧风险。\n\n
	* The default is 20, available minimum value can be 1. The bigger value, the more memory will be allocated. The smaller value, the higher risk of miss a frame.
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED		相机处于 start 状态。		 Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。	 Camera is disconnected.
	*/
	EXPORT_TO_DLL CAM_STATUS	setBufferCount	(const uint32_t	& value)
	{
		if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
		if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;
		m_Param.m_buffersCount = value;						return value == getBufferCount() ? CAM_STATUS::All_OK : CAM_STATUS::UNKNOWN;	
	};

	/**
	* @brief 启动取图像线程池，用于 callback 函数。 \n\n
	* Set the number of threads in the pool.
	* 
	* @param [in] threadsNumber	线程池内的线程数，建议大约和 buffercount 一样。
	* @param [in] enable		是否启用
	* 
	* @note
	* 线程的创建和销毁会消耗 CPU 资源，采集图像过程中如果线程阻塞，，所以不可将。
	* 
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED		相机处于 start 状态。		 Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。	 Camera is disconnected.
	*/
	EXPORT_TO_DLL CAM_STATUS	enableCallbackThreadsInPool(const size_t& threadsNumber, const bool enable = false);

	/** 
	* @brief 设置 IP 地址\n\n
	* Set IP address.
	* \n
	* @param [in] IP			IP地址。  The new IP to be set.
	* @param [in] isPersistent	true 表示设置永久性的 IP。   true means to set persistent IP. 
	* \n
	* @par 详细说明 Introduction
	* 设置相机的 IP/Subnet 的过程较为复杂。请严格按照这里给出的说明操作。
	* 相机的 IP/Subnet 设置分为“永久性 IP/Subnet 设置” 与 “临时性 IP/Subnet 设置”。  \n
	* When setting IP, please follow the description here strictly;
	* The settings include "Persistent IP/Subnet setting" and "Temporary IP/Subnet setting".
	* \n
	* @par 永久性 IP/Subnet 设置  Persistent IP/Subnet setting.
	* 相机必须处于 connected 或者 stopped 状态。请将"isPersistent"设置为 true 以执行持久设置。
	* 关闭电源时，IP 将被保留。 \n
	* Camera must be connected or stopped. Please set "isPersistent" as true to perform persistent setting.
	* The IP will be kept when power off.
	* \n
	* @note 执行“永久性 IP/Subnet设置”时，设置成功后，setIP() 将返回 CAM_STATUS::ALL_OK。
	* <但是> 您必须调用 R3S::scanDevice() 以重新扫描所有设备！然后再次连接相机。\n
	* When executing "Persistent IP/Subnet setting", if setting successfully,
	* setIP() will return CAM_STATUS::ALL_OK. <BUT> You must call R3S::scanDevice()
	* to scan all device again! And after that, re-connect the device.
	* \n
	* \n
	* @par 临时性 IP/Subnet 设置  Force IP, temporary IP/Subnet setting.
	* 相机必须处于 disconnected 状态。请将"isPersistent"设置为 false。它将执行"ForcoIP"，
	* 这将设置临时 IP/Subnet。关闭电源后，新设置的 IP/Subnet 将丢失，
	* 设备将恢复成设置前的 IP/Subnet 地址。 \n
	* Camera must be disconnected. Please set "isPersistent" as false;
	* It will perform temporary IP/Subnet setting
	* which has been called "ForceIP". The new IP will be lost when power off.
	* \n
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED		相机处于 start 状态。		Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。	Camera is disconnected.
	* - CAM_STATUS::ERROR_SET_IP				相机设置失败。				Setting failed.
	* \n
	* @note 相机处于 started 状态时，不可执行以上任何一种设置。\n\n
	* When camera is started, neither settings is avaliable.
	*/
	EXPORT_TO_DLL CAM_STATUS	setIp					(cStr& IP, bool isPersistent = false);

	/**
	* @brief 设置 Subnet 地址\n\n
	* Set Subnet address. \n
	* \n
	* @param [in] Su			IP地址。  The new IP to be set.
	* @param [in] isPersistent	true 表示设置永久性的 IP。   true means to set persistent IP.
	* \n
	* @par 详细说明 Introduction
	* 设置相机的 IP/Subnet 的过程较为复杂。请严格按照这里给出的说明操作。
	* 相机的 IP/Subnet 设置分为“永久性 IP/Subnet 设置” 与 “临时性 IP/Subnet 设置”。  \n
	* When setting IP, please follow the description here strictly;
	* The settings include "Persistent IP/Subnet setting" and "Temporary IP/Subnet setting".
	* \n
	* @par 永久性 IP/Subnet 设置  Persistent IP/Subnet setting.
	* 相机必须处于 connected 或者 stopped 状态。请将"isPersistent"设置为 true 以执行持久设置。
	* 关闭电源时，IP 将被保留。 \n
	* Camera must be connected or stopped. Please set "isPersistent" as true to perform persistent setting.
	* The IP will be kept when power off.
	* \n
	* @note 执行“永久性 IP/Subnet设置”时，设置成功后，setIP() 将返回 CAM_STATUS::ALL_OK。
	* <但是> 您必须调用 R3S::scanDevice() 以重新扫描所有设备！然后再次连接相机。\n
	* When executing "Persistent IP/Subnet setting", if setting successfully,
	* setIP() will return CAM_STATUS::ALL_OK. <BUT> You must call R3S::scanDevice()
	* to scan all device again! And after that, re-connect the device.
	* \n
	* \n
	* @par 临时性 IP/Subnet 设置  Force IP, temporary IP/Subnet setting.
	* 相机必须处于 disconnected 状态。请将"isPersistent"设置为 false。它将执行"ForcoIP"，
	* 这将设置临时 IP/Subnet。关闭电源后，新设置的 IP/Subnet 将丢失，
	* 设备将恢复成设置前的 IP/Subnet 地址。 \n
	* Camera must be disconnected. Please set "isPersistent" as false; 
	* It will perform temporary IP/Subnet setting
	* which has been called "ForceIP". The new IP will be lost when power off.
	* \n
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED		相机处于 start 状态。		Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。	Camera is disconnected.
	* - CAM_STATUS::ERROR_SET_SUBNET			相机设置失败。				Setting failed.
	* \n
	* @note 相机处于 started 状态时，不可执行以上任何一种设置。\n\n
	* When camera is started, neither settings is avaliable.
	*/
	EXPORT_TO_DLL CAM_STATUS	setSubnet				(cStr& Su, bool isPersistent = false);

	/** 
	* @brief 设置设备参数，即 csv 文件中的参数。【不支持 started 时设置参数】\n\n
	* Set the device parameters, which are shown in csv file.
	* \n
	* @param [in] ParamterName		在csv文件的参数名。 Name shown in csv file.
	* @param [in] value				The new value to be set.
	* \n
	* @note 设置参数时，相机必须处于 connected 或者 stopped。\n\n
	* 如果需要设置其他参数，请见 3.3.0.4 之后的新增接口：setParameter() \n
	* The camera must be connected or stopped when setting parameters. \n
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED			相机处于 start 状态。			Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED		相机处于 disconnect 状态。	Camera is disconnected.
	* - CAM_STATUS::ERROR_PARAMETER_VALUE_INVALID	设置值非法。					Invalid setting value.
	* - CAM_STATUS::ERROR_PARAMETER_INVALID			设置失败。					Setting failed.
	*/
	EXPORT_TO_DLL CAM_STATUS	setParameterValue		(cStr& ParamterName, cStr& value);

	/**
	* @brief 【完全兼容 setParameterValue ，不支持 started 时设置参数】设置设备参数（不仅包含 csv 文件中的参数，还可以是其他有多级 selector 的参数）。但是，需要注意“可写”的参数一般都在 csv 文件中，所以这里主要考虑的是一些“指令”参数，比如 xxxReset 之类。\n\n
	* Set the device parameters.
	* \n
	* @param [in] ParamterName		在csv文件的参数名。 Name shown in csv file.
	* @param [in] value				The new value to be set.
	* \n
	* @note 设置参数时，相机必须处于 connected 或者 stopped【不支持 started 时设置参数】。\n\n
	* 注意，Ranger3Studio 中的参数，根据功能，可以分为 3 种：这个函数支持 1 和 2。\n
	* - 1 数值型：整数、小数、字符串；Ranger3Studio 上体现为【输入框 / Editbox】。\n
	* - 2 使能型：布尔（使用 0，1 表示）；Ranger3Studio 上体现为【勾选框 / CheckBox】。\n
	* - 3 命令型：Ranger3Studio 上体现为【按钮 / Button】。如需使用此类型参数，请调用 excuteCommand()\n
	* The camera must be connected or stopped when setting parameters.\n
	* \n
	* @note 相机参数名的推导。\n\n
	* 例子一：\n
	* 在 Ranger3Studio 中的多级参数结构中，如果有如下的结构。\n
	* 1. 选择页面 ImageFormatControl\n
	* 2. 选择 RegionSelector 的下拉框，再选择 ComponentSelector 的下拉框，然后找到 ComponentEnable 勾选框（0/1）。\n
	* RegionSelector_Scan3dExtraction1 \n
	*                                |____ComponentSelector_Reflectance \n
	*                                                                  |____ComponentEnable \n
	* 3. 仿造下面的结果，可以推导出适当的参数名。\n
	* 上例中，推导的参数名为：ComponentEnable_ComponentSelector_Reflectance_RegionSelector_Scan3dExtraction1 \n
	* 并且，勾选框的参数可选值为 0/1。\n
	* 
	* 例子二：\n
	* 在 Ranger3Studio 中的多级参数结构中，如果有如下的结构。\n
	* 1. 选择页面 DeviceControl \n
	* 2. 选择 DeviceScanType 下拉框。 \n
	* 3. 上例中，推导的参数名为：DeviceScanType。这里没有多级的 selector 。 \n
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED			相机处于 start 状态。			Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED		相机处于 disconnect 状态。	Camera is disconnected.
	* - CAM_STATUS::ERROR_PARAMETER_VALUE_INVALID	设置值非法。					Invalid setting value.
	* - CAM_STATUS::ERROR_PARAMETER_INVALID			设置失败。					Setting failed.
	*/
	EXPORT_TO_DLL CAM_STATUS	setParameter			(cStr& ParamterName, cStr& value); // the second variable should be "" when excute command.


	////////////////////////////////////////////////////////////////////////////////

	//	Get the user parameters.

	/**
	* @brief 读取 Y 方向分辨率 \n\n
	* Get scale in Y axis.
	* \n
	* @note
	* 点云 y 坐标 = Y_offset + Y_scale * Y_pixelValueInRange。 \n\n
	* Y value of a pointcloud = Y_offset + Y_scale * Y_pixelValueInRange.
	* \n
	* @note 建议在相机处于 connected 或者 Stopped 状态时，执行该操作。\n\n
	* Camera should be connected or stopped.
	*/
	EXPORT_TO_DLL double		getScaleY				()	const	{	return m_Param.m_scaleY;	};
	
	/**
	* @brief 【不建议继续使用，功能冗余。读取空数据的像素值，可以通过解析图像数据来获得。】 \n\n
	* Get missing data.
	* \n
	* @note MissingData 表示用于填补空数据的像素值，默认 -1.0e6，见宏 MISSING_DATA。  \n\n
	* The value is to fill in the empty pixels, the default is -1.0e6, see MISSING_DATA.
	* \n
	* @note 建议在相机处于 connected 或者 Stopped 状态时，执行该操作。\n\n
	* Camera should be connected or stopped.
	*/
	EXPORT_TO_DLL float			getMissingData			()	const	{	return m_Param.m_missingData;	};
	
	/**
	* @brief 【不建议再继续使用】读取使用 PC 标定时，设定的参数文件路径 \n\n
	* Get parameters file path.
	* \n
	* @return
	* - "" 空字符串表示未导入参数文件。  Empty string. Mo parameters file.
	* \n
	* @note 建议在相机处于 connected 或者 Stopped 状态时，执行该操作。\n\n
	* Camera should be connected or stopped.
	*/
	EXPORT_TO_DLL std::string	getParametPath			()	const	{	return m_Param.m_ParaPath;	};
	
	/**
	* @brief 读取相机取图的最大等待时间，单位毫秒，默认值见 DEFAULT_TIMEOUT \n\n
	* Get maximum waiting time when grabbing image, unit is millimeter. The defalut is DEFAULT_TIMEOUT.
	* \n
	* @note 建议在相机处于 connected 或者 Stopped 状态时，执行该操作。\n\n
	* Camera shoudl be connected or stopped.
	*/
	EXPORT_TO_DLL uint32_t		getTimeOut				()	const	{	return m_Param.m_timeOut;	};
	
	/**
	* @brief 读取图像缓存的张数 \n\n
	* Get the number of image buffers.
	* \n
	* @note 默认是 20 ，最小可以设置为 1。设置越大，内存占用越多。设置太小，有丢帧风险。
	* 建议在相机处于 connected 或者 Stopped 状态时，执行该操作。\n\n
	* The default is 20, avaliable minimum value can be 1. The bigger value, the more memory will be allocated. The smaller value, the higher risk of miss a frame.
	* Camera shoudl be connected or stopped.
	*/
	EXPORT_TO_DLL uint32_t		getBufferCount			()	const	{	return m_Param.m_buffersCount;	};
	
	/**
	* @brief 从设备上重新读取 IP 地址，该算子会和相机进行通讯。 \n\n
	* Get IP address from device.
	* \n
	* @param [in] isPersistent	true 表示读取永久性的 IP。   true means to get persistent IP.
	* \n
	* @return
	* -  "" 错误返回空字符串。  return empty string when error happened.
	* \n
	* @par 详细说明 Introduction
	* 读取相机的 IP/Subnet 时，请严格按照这里给出的说明操作。
	* 相机的 IP/Subnet 分为“永久性 IP/Subnet ” 与 “临时性 IP/Subnet ”。  \n
	* When getting IP, please follow the description here strictly;
	* The IP/Subnet include "Persistent IP/Subnet" and "Temporary IP/Subnet".
	* \n
	* @par 永久性 IP/Subnet   Persisitent IP/Subnet.
	* 相机必须处于 connected 或者 stopped 状态。请将"isPersistent"设置为 true 以执行读取“永久性 IP”。 \n
	* Camera must be connected or stopped. Please set "isPersistent" as true to get persistent IP.
	* \n
	* @par 临时性 IP/Subnet  Force IP, temporary IP/Subnet.
	* 相机必须处于 disconnected 状态。请将"isPersistent"设置为 false。它将执行"ForceIP" \n
	* Camera must be disconnected. Please set "isPersistent" as false;
	* It will get temporary IP/Subnet
	* \n
	* @note 相机处于 started 状态时，不可执行以上任何一种设置。\n\n
	* When camera is started, neither settings is available.
	*/
	EXPORT_TO_DLL Str			getIp					(bool isPersistent = false);

	/**
	* @brief 读取 IP 地址，IP 地址是实现保存在成员变量中的，不会和相机通讯。\n\n
	* Get IP address.
	* \n
	* \n
	* @return
	* -  "" 错误返回空字符串。  return empty string when error happened.
	* @note 相机处于 started 状态时，不可执行以上任何一种设置。\n\n
	* When camera is started, neither settings is available.
	*/
	EXPORT_TO_DLL Str			get_DeviceIP()	const { return m_DeviceIP; };

#ifdef CALLBACK_NEW
	/**
	* @brief 读心跳计数。当心跳停止的时候，这个值不会在变化。 \n\n
	* Get Heart beats count.
	* \n
	* \n
	* @return
	* -  "" 错误返回空字符串。  return empty string when error happened.
	* @note 相机处于 started 状态时，不可执行以上任何一种设置。\n\n
	* When camera is started, neither settings is available.
	*/
	EXPORT_TO_DLL uint64_t		get_Heartbeat_Count()	const { return m_heartbeat_count; };
#endif
	/**
	* @brief 读取 Subnet \n\n
	* Get Subnet.
	* \n
	* @param [in] isPersistent	true 表示读取永久性的 Subnet。   true means to get persistent Subnet.
	* \n
	* @return
	* - "" 错误返回空字符串。  return empty string when error happened.
	* \n
	* @par 详细说明 Introduction
	* 读取相机的 IP/Subnet 时，请严格按照这里给出的说明操作。
	* 相机的 IP/Subnet 分为“永久性 IP/Subnet ” 与 “临时性 IP/Subnet ”。  \n
	* When getting IP, please follow the description here strictly;
	* The IP/Subnet include "Persistent IP/Subnet" and "Temporary IP/Subnet".
	* \n
	* @par 永久性 IP/Subnet   Persistent IP/Subnet.
	* 相机必须处于 connected 或者 stopped 状态。请将"isPersistent"设置为 true 以执行读取“永久性 IP”。 \n
	* Camera must be connected or stopped. Please set "isPersistent" as true to get persistent IP.
	* \n
	* @par 临时性 IP/Subnet  Force IP, temporary IP/Subnet.
	* 相机必须处于 disconnected 状态。请将"isPersistent"设置为 false。它将执行"ForcoIP" \n
	* Camera must be disconnected. Please set "isPersistent" as false;
	* It will get temporary IP/Subnet
	* \n
	* @note 相机处于 started 状态时，不可执行以上任何一种设置。\n\n
	* When camera is started, neither settings is avaliable.
	*/
	EXPORT_TO_DLL Str			getSubNet				(bool isPersistent = false) const;
	
	/**	
	* @brief MAC 地址支持任何相机状态下读取。MAC 地址不可修改，出厂固定。支持 disconnect 读取，网络断开读取。 \n\n
	* MAC. can be queried in any camera status. MAC. is unchangeable.
	* \n
	* @return
	* - "" 错误返回空字符串。  return "" when error happened.
	*/
	EXPORT_TO_DLL cStr			getMac					()	const	{	return m_device->getMac();	};
	
	/**
	* @brief 设备名格式： "SICKGigEVisionTL_DEV_" + mac + "_Ranger3" \n\n
	* Device name format : "SICKGigEVisionTL_DEV_" + mac + "_Ranger3" 
	* \n
	* @return
	* - 以字符串形式输出结果。
	* - "" 错误返回空字符串。  return "" when error happened.
	* \n
	* @note 支持任何相机状态下读取。\n\n
	* It can be queried in any camera status..
	*/
	EXPORT_TO_DLL cStr			getDeviceName			()	const	{	return m_DeviceName;		}

	/**
	* @brief 【不建议再继续使用，请使用代替函数 getParameter("DeviceTemperature") 】读取相机温度，可以通过 getParameter 读取参数 【DeviceTemperature】 \n\n
	* Read device temperature.
	* \n
	* @return
	* - 以字符串形式输出结果。
	* - "" 错误返回空字符串。  return "" when error happened.
	* \n
	* @note 支持任何相机状态下读取。\n\n
	* It can be queried in any camera status..
	*/
	EXPORT_TO_DLL cStr			getTemperature()	const;

	/** 
	* @brief 读取 csv 文件中的相机参数，相机不可处于 started。【如需要在 started 时读取参数，可使用 getParameter 】 \n\n
	* Get the device parameters shown in csv file.
	* \n
	* @param [in] ParamterName		参数名，可以在 csv 文件中找到。  Parameter name. It can be found in csv file.
	*  \n
	* @return
	* - "" 错误，返回空字符串。  return empty string when error happened.
	* \n
	* @note 相机必须处于 connected 或者 stopped 状态。\n\n
	* The camera must be connected or stopped when calling this.
	*/
	EXPORT_TO_DLL cStr			getParameterValue		(cStr& ParameterName);

	/**
	* @brief 读取 csv 文件中的相机参数、Ranger3Studio 中的只读参数、指令等。支持包含多级 selector 的参数。【相机可以处于 started。如果遇到不支持读取的参数会抛出异常】 。 \n\n
	* Get the device parameters.
	* \n
	* @param [in] ParamterName		参数名，可以在 csv 文件中找到，或者根据 Ranger3Studio 中的所处位置进行推断。  Parameter name. It can be found in csv file or Ranger3Studio.
	*  \n
	* @return
	* - "" 错误，返回空字符串。  return empty string when error happened.
	* \n
	* @note 相机必须处于 connected 状态。\n\n
	* 特别地，这个函数支持在 started 状态下读取某些参数。某一参数是否支持在 started 状态读写，请根据实际情况测试为准。\n
	* 注意，Ranger3Studio 中的参数，根据功能，可以分为 3 种：这个函数支持 1 和 2。\n
	* - 1 数值型：整数、小数、字符串；Ranger3Studio 上体现为【输入框 / Editbox】。\n
	* - 2 使能型：布尔（使用 0，1 表示）；Ranger3Studio 上体现为【勾选框 / CheckBox】。\n
	* - 3 命令型：Ranger3Studio 上体现为【按钮 / Button】。如需使用此类型参数，请调用 excuteCommand()\n
	* The camera must be connected when calling this.
	* \n
	* @note 相机参数名的推导。\n\n
	* 例子一：\n
	* 在 Ranger3Studio 中的多级参数结构中，如果有如下的结构。\n
	* 1. 选择页面 ImageFormatControl\n
	* 2. 选择 RegionSelector 的下拉框，再选择 ComponentSelector 的下拉框，然后找到 ComponentEnable 勾选框（0/1）。\n
	* RegionSelector_Scan3dExtraction1 \n
	*                                |____ComponentSelector_Reflectance \n
	*                                                                  |____ComponentEnable \n
	* 3. 仿造下面的结果，可以推导出适当的参数名。\n
	* 上例中，推导的参数名为：ComponentEnable_ComponentSelector_Reflectance_RegionSelector_Scan3dExtraction1 \n
	* 并且，勾选框的参数可选值为 0/1。\n
	* \n
	* 例子二：\n
	* 在 Ranger3Studio 中的多级参数结构中，如果有如下的结构。\n
	* 1. 选择页面 DeviceControl \n
	* 2. 选择 DeviceScanType 下拉框。 \n
	* 3. 上例中，推导的参数名为：DeviceScanType。这里没有多级的 selector 。 \n
	*/
	EXPORT_TO_DLL cStr			getParameter			(cStr& ParameterName);
	

	/** 
	*@brief 读取所有参数 \n\n
	* Get all parameter.
	* \n
	* @param [out] Info 返回std::map<csv 文件中的参数名, Parameter_Ptr>。 Return std::map<name_in_csv_file, Parameter_Ptr> 
	* @param [in] forceUpdate 强制重新获取相机的参数列表。   Force to reload parameters from device. 
	* \n
	* @note 相机必须处于 connected 或者 stopped 状态。\n\n
	* The camera must be connected or stopped when calling it.
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED		相机处于 start 状态。		Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。	Camera is disconnected.
	* - CAM_STATUS::ERROR_PARAMETERS_EMPTY		参数列表为空。				Empty parameters .
	*/
	EXPORT_TO_DLL CAM_STATUS	getAllParameterInfo		(AllParams & Info, const bool forceUpdate=false);
	
	/**
	*@brief 按分类读取所有参数 \n\n
	* Get all parameter sorted by categories.
	* \n
	* @param [out] cate 参数的分类名。  name vector of categories of parameters.  
	* @param [out] para 分类名及其所包含的参数名。 std::map<Categories_name, Vec_parameter_ptr_in_a_category>. 
	* @param [in] forceUpdate 强制重新获取相机的参数列表。   Force to reload parameters from device.
	* \n
	* @note 相机必须处于 connected 或者 stopped 状态。\n\n
	* The camera must be connected or stopped when calling this.
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED		相机处于 start 状态。		Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。	Camera is disconnected.
	* - CAM_STATUS::ERROR_PARAMETERS_EMPTY		参数列表为空。				Empty parameters .
	*/
	EXPORT_TO_DLL CAM_STATUS	getAllParameterInfo		(Categories & cate, Params & para, const bool forceUpdate = false);

	/** @brief Get string of all parameters. */
	EXPORT_TO_DLL Str			getParametersAsStructureString		() const { return m_Param.printParameters();};
	
	/** @brief Get string of device info. \n\n
	* \n
	* @note 请在 connected 或者 stopped 状态下读取。\n\n
	* 其中设备基本参数，也可以使用下面的函数读取，\n
	* this->getParameterValue()\n
	* \n
	* 参见例程：Demo_R3_05_read_device_information.cpp\n
	* \n
	*/
	EXPORT_TO_DLL Str			getDeviceInfoAsStructureString		() const;


	////////////////////////////////////////////////////////////////////////////////

	/**
	*@brief 读取相机状态。  \n\n
	* Get camera status.
	* \n
	* @return 
	* - CAM_STATUS::CAM_IS_CONNECTED		相机已连接 Connected
	* - CAM_STATUS::CAM_IS_STARTED			相机开始采集 Start acquisition
	* - CAM_STATUS::CAM_IS_STOPPED			相机停止采集 Stop acquisition
	* - CAM_STATUS::CAM_IS_DISCONNECTED		相机未连接 Disconnected
	* - CAM_STATUS::ERROR_NULL_PTR_DEV		类 Ranger3 的对象未初始化完成。 Class Ranger3 initialization undone.
	*/
	EXPORT_TO_DLL CAM_STATUS	getStatus				()	const	{ return m_status; };

	/**
	*@brief 读取相机类型。  \n\n
	* Get camera type.
	* \n
	* @return
	* - _RANGER3
	*/
	EXPORT_TO_DLL CamType		getCamType				()	const { return m_camType; };

	/** 
	* @brief 【不建议再继续使用。可通过解析图像数据获得，这样更加合理】读取图像序号，也是当前取到的图像总数，stop 后清理。  \n\n
	* Get image ID. The value is also the number of grabbed image. Reset to 0 when stopped.
	* \n
	* @note 0初始。相机必须处于 connected 或者 stopped 状态。\n\n
	* Start from 0. The camera must be connected or stopped when calling this.
	*/
	EXPORT_TO_DLL uint64_t		getImgID				()	const	{ return m_info.m_id; };

	////////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	/**
	* @brief 【为防止误删，请谨慎使用此函数】发送文件到相机上的 100K 存储空间。默认用于存储标定文件。请客户在确定要更新标定文件后，再使用该函数。 \n\n
	* Sends a file to the device.
	* \n
	* @note 相机必须处于 connected 或者 stopped 状态。\n\n
	* The camera must be connected or stopped when calling this.\n
	* \n
	* @note 
	* 该 100K 空间，默认用来存放标定文件。\n\n
	* 100K 空间只可以存放一个文件。后放的文件将覆盖之前的文件。该区域将始终只有一个文件。\n
	* \n
	* @return
	* - CAM_STATUS::ALL_OK							操作成功。
	* - CAM_STATUS::ERROR_USER_FILE_NOT_FOUND_IN_PC 操作失败，无效的文件输入。
	* - CAM_STATUS::ERROR_USER_FILE_SEND_FAILED		操作失败，发送过程失败。
	*/
	EXPORT_TO_DLL CAM_STATUS	sendFileToCamera(std::istream & contents);
	/** Overload	*/
	EXPORT_TO_DLL CAM_STATUS	sendFileToCamera(const std::string & filePath); 

	/** 
	* @brief 获取相机 100K 存储空间内的文件。默认该文件是相机的标定文件。  \n\n
	* Retrive a file from the device.
	* \n
	* @note 相机必须处于 connected 或者 stopped 状态。\n\n
	* The camera must be connected or stopped when calling this.
	* \n
	* @return 
	* - 返回文本的内容。
	*/
	EXPORT_TO_DLL std::string	retrieveFileFromCamera();

	/**
	* @brief 【为防止误删，请谨慎使用此函数】删除 100K 存储空间内的文件。  \n\n
	* Delete a file in the device.
	* \n
	* @note 相机必须处于 connected 或者 stopped 状态。\n\n
	* The camera must be connected or stopped when calling this.
	* \n
	* @return
	* - CAM_STATUS::All_OK						成功删除
	* - CAM_STATUS::ERROR_USER_FILE_NOT_EXIST	删除失败，100K 空间内不存在文件
	*/
	EXPORT_TO_DLL CAM_STATUS	deleteFileFromCamera();

	////////////////////////////////////////////////////////////////////////////////

	/**
	* @brief 读取相机的参数池。相机开放了 5 组参数设置，用户可以预设，然后在必要的时候使能。  \n\n
	* Get user set.
	* \n
	* @param[in] id				可设置值为 1-5
	* @param[out] is_All_OK		成功返回 ALL_OK
	* \n
	* @note 相机必须处于 connected 或者 stopped 状态。\n\n
	* The camera must be connected or stopped when calling this.
	* \n
	* @return
	* - UserSet	返回包含 UserSet 操作的类。若操作失败，将返回默认的 UserSet。 
	*/
	EXPORT_TO_DLL UserSet		getUserSet(const UserSetId & id, CAM_STATUS & is_All_OK);
	/** Overload	*/
	EXPORT_TO_DLL UserSet		getUserSet(const UserSetId & id);
	
	/**
	* @brief 加载（使能）相机开放的 5 组参数中的一个。  \n\n
	* \n
	* @param[in] id	 可设置值为 1-5
	* \n
	* @note 相机必须处于 connected 或者 stopped 状态。\n\n
	* The camera must be connected or stopped when calling this.
	* \n
	* @return
	* - CAM_STATUS::ALL_OK						操作成功。
	* - CAM_STATUS::ERROR_OPERATION_NOT_ALLOW	非法操作，一般为 id 给定的值超出1-5，或者 userSet 没有预设。
	*/
	EXPORT_TO_DLL CAM_STATUS	userSet_load(const int & id);
	
	/**
	* @brief 保存当前设置到，相机开放的 5 组参数中的一个。  \n\n
	* \n
	* @param[in] id					可设置值为 1-5
	* \n
	* @note 相机必须处于 connected 或者 stopped 状态。\n\n
	* The camera must be connected or stopped when calling this.
	* \n
	* @return
	* - CAM_STATUS::ALL_OK						操作成功。
	* - CAM_STATUS::ERROR_OPERATION_NOT_ALLOW	非法操作，一般为 id 给定的值超出1-5，或者 userSet 没有预设。
	*/
	EXPORT_TO_DLL CAM_STATUS	userSet_save(const int & id);

	/**
	* @brief 将相机开放的 5 组参数中的一个，设置为默认启动加载项。  \n\n
	* \n
	* @param[in] id					可设置值为 1-5
	* \n
	* @note 相机必须处于 connected 或者 stopped 状态。\n\n
	* The camera must be connected or stopped when calling this.
	* \n
	* @return
	* - CAM_STATUS::ALL_OK						操作成功。
	* - CAM_STATUS::ERROR_OPERATION_NOT_ALLOW	非法操作，一般为 id 给定的值超出1-5，或者 userSet 没有预设。
	*/
	EXPORT_TO_DLL CAM_STATUS	userSet_set_default(const int & id);

	/**
	* @brief 为相机开放的 5 组参数中的一个，添加描述。  \n\n
	* \n
	* @param[in] id					可设置值为 1-5
	* @param[in] description		参数组的描述
	* \n
	* @note 相机必须处于 connected 或者 stopped 状态。\n\n
	* The camera must be connected or stopped when calling this.
	* \n
	* @return
	* - CAM_STATUS::ALL_OK						操作成功。
	* - CAM_STATUS::ERROR_OPERATION_NOT_ALLOW	非法操作，一般为 id 给定的值超出1-5，或者 userSet 没有预设。
	*/
	EXPORT_TO_DLL CAM_STATUS	userSet_set_description(const int & id, const std::string & description);

	/**
	* @brief 读取，相机开放的 5 组参数中的一个的描述。  \n\n
	* \n
	* @param[in] id					可设置值为 1-5
	* \n
	* @note 相机必须处于 connected 或者 stopped 状态。\n\n
	* The camera must be connected or stopped when calling this.
	* \n
	*/
	EXPORT_TO_DLL std::string	userSet_get_description(const int & id);
#endif

	/**
	* @brief 更新相机固件。此函数将会阻塞，请运行后重新扫描相机。 \n\n
	* Excute device command.
	* \n
	* @param [in] firmwarePackagePath		指令名。  firmware package path.
	*  \n
	* @Note \n
	* 过程 1: Link/Data 闪烁，表示发送固件文件到相机。 \n
	* 过程 2: State 闪烁，表示正在执行固件升级，请不要断开相机电源。\n
	*/
	EXPORT_TO_DLL bool updateFirmware(const std::string& firmwarePackagePath);

////////////////////////////////////////////////////////////////////////////////

#pragma region not_use_anymore

	// disable calibration in pc function as default.
#ifndef DISABLE_CAL_IN_PC
	/**
	* @brief 【不建议再继续使用，请使用In-Device标定。】读取标定的矫正方法 \n\n
	* Get rectification methods.
	* \n
	* @note 详见标定过程的介绍。\n\n
	* More details please see the user documents.
	* \n
	* @note 建议在相机处于 connected 或者 Stopped 状态时，执行该操作。\n\n
	* Camera should be connected or stopped.
	*/
	EXPORT_TO_DLL RectMethod	getRectMethos(bool do_not_use_it____inPC_Calibration_will_be_replaced_by_inDevice = true)	const { return m_Param.m_RectMethod; };

	/**
	* @brief 启用 PC 标定，从相机端只获得原始数据，标定过程在PC端完成，需要设置标定文件。标定文件支持 json 和 xml。推荐使用 json \n\n
	* Enable in-PC calibration, calibration file required.\n
	* \n
	* @param [in] filePath			   若 filePath 为空，则输出未标定的原始数据据。  If filePath=="", output uncalibrated image.
	* @param [in] toDoRectify		   控制是否进行矫正。  To rectify or not.
	* @param [in] rectificationWidth   矫正宽度。在PC上执行的情况下，支持设置范围（0-10000）。一般而言，该值越大，X方向的分辨率越高。但是算法得到的虚拟点也会影响真值，需要根据实际应用情况权衡。  Rectification Width.
	* @param [in] threads_number_X64Only 设置标定函数将使用的线程数（仅 X64 有效）。如果设置为-1，则设置为 CPU 的逻辑核数，如此，则一般会达到 100% 的CPU使用率。如果希望降低 CPU 使用率，可以减少线程数，这样也会增加标定处理的使用时间。
	* @param [in] runMode			   定义 CPU 的优化模式。如果本机不支持 AVX ，必须设置为 0；-1 表示默认值。如果遇到 sheet_of_light.dll 报错，请先使用 cpu-z 查看 CPU 是否支持 AVX。
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED		相机处于 start 状态。  Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。  Camera is disconnected.
	* - CAM_STATUS::ERROR_CALIBRATION_PATH		标定参数文件的路径不正确。  Calibration file path is invalid.
	*/
	EXPORT_TO_DLL CAM_STATUS	setCalibration_inPC(const Str& filePath,
		const bool toDoRectify = true,
		const uint32_t& rectificationWidth = RECTIFICATION_WIDTH,
		const int threadsNumber_X64Only = -1,
		const int runMode = -1);

	/**
	* @brief 设置标定的矫正方法（PC 标定）。如果要设置 in-device 标定的校正方法，请设置参数 【Scan3dRectificationMethod_Scan3dExtractionSelector_Scan3dExtraction1】 \n\n
	* Set rectification methods.
	* \n
	* @param [in] value		Top(default) / Mean / Bottom.
	* \n
	* @note 详见标定过程的介绍。\n\n
	* More details please see the user documents.
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED		相机处于 start 状态。  Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。  Camera is disconnected.
	*/
	EXPORT_TO_DLL CAM_STATUS	setRectMethos_inPC(const RectMethod& value, bool do_not_use_it____inPC_Calibration_will_be_replaced_by_inDevice = true)
	{
		if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
		if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;
		m_Param.m_RectMethod = value;					return value == getRectMethos() ? CAM_STATUS::All_OK : CAM_STATUS::UNKNOWN;
	};
	/** @brief 【不建议再继续使用，请使用代替函数 setRectMethos_inPC */
	EXPORT_TO_DLL CAM_STATUS	setRectMethos(const RectMethod& value)
	{
		return setRectMethos_inPC(value);
	};

	/**
	* @brief 【不建议再继续使用，该参数是使用 xml 标定文件用的，目前已经不起作用】设置启用 SSE ，来加速标定过程的处理速度。 \n\n
	* Set to enable SSE or not.
	* \n
	* @param [in] value		控制是否启用 SSE。  To enable SSE or not.
	* \n
	* @note 临时保留，建议设置为 true。\n\n
	* temporary parameter, set to true.
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED		相机处于 start 状态。			Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。	Camera is disconnected.
	*/
	EXPORT_TO_DLL CAM_STATUS	setEnableSSE(const bool	      value = true, bool do_not_use_it_any_more = true)
	{
		if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
		if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;
		m_Param.m_enableSSE = value;						return CAM_STATUS::All_OK;
	};


	/**
	* @brief 【不建议再继续使用，请使用In-Device标定。】设置标定的矫正系数（PC 标定）。如果要设置 in-device 标定的校正系数，请设置参数 【Scan3dRectificationSpread_Scan3dExtractionSelector_Scan3dExtraction1】 \n\n
	* Set rectification spread.
	* \n
	* @param [in] value		默认 1.2
	* \n
	* @note 详见标定过程的介绍。这个值越大，图像上的 missing data 越少。\n\n
	* More details please see the user documents.
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED		相机处于 start 状态。  Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。  Camera is disconnected.
	*/
	EXPORT_TO_DLL CAM_STATUS	setRectSpread_inPC(const double& value, bool do_not_use_it____inPC_Calibration_will_be_replaced_by_inDevice = true)
	{
		if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
		if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;
		m_Param.m_RectSpread = value;					return value == getRectSpread() ? CAM_STATUS::All_OK : CAM_STATUS::UNKNOWN;
	};

	/** @brief 【不建议再继续使用，请使用代替函数 setRectSpread_inPC */
	EXPORT_TO_DLL CAM_STATUS	setRectSpread(const double& value, bool please_do_not_use_it_any_more = true)
	{
		if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
		if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;
		m_Param.m_RectSpread = value;					return value == getRectSpread() ? CAM_STATUS::All_OK : CAM_STATUS::UNKNOWN;
	};

	/**
	* @brief 【不建议再继续使用，请使用In-Device标定。】设置标定后数据中的无效数据的值（PC 标定）。 \n\n
	* Set missing data.
	* \n
	* @param [in] value		用于填补空数据的像素值，默认 -1.0e6f，见宏 MISSING_DATA。  The value to fillin the empty pixels, the default is -1.0e6f, see MISSING_DATA.
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED		相机处于 start 状态。 Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。 Camera is disconnected.
	*/
	EXPORT_TO_DLL CAM_STATUS	setMissingData_inPC(const float& value, bool do_not_use_it____inPC_Calibration_will_be_replaced_by_inDevice = true)
	{
		if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
		if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;
		m_Param.m_missingData = value;						return CAM_STATUS::All_OK;
	};
	
	/** @brief 【不建议再继续使用，请使用代替函数 setMissingData_inPC */
	EXPORT_TO_DLL CAM_STATUS	setMissingData(const float& value, bool please_do_not_use_it_any_more = true)
	{
		return setMissingData_inPC(value);
	};

	/**
	* @brief 【不建议再继续使用，请使用In-Device标定。】设置是否进行矫正，如需矫正，则请指定校正后的图像宽度（PC 标定）。in-device 标定暂不支持 \n\n
	* Set to rectify or not.
	* \n
	* @param [in] toDoRectify		   控制是否进行矫正。  To rectify or not.
	* @param [in] rectificationWidth   矫正宽度。在PC上执行的情况下，支持设置范围（0-10000）。一般而言，该值越大，X方向的分辨率越高，算法插值得到的虚拟点也越多，请根据实际应用情况权衡。  Rectification Width.
	* \n
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED		相机处于 start 状态。			Camera is started.
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。	Camera is disconnected.
	*/
	EXPORT_TO_DLL CAM_STATUS	setDoRectify_inPC(const bool toDoRectify = true, const uint32_t& rectificationWidth = 3200, bool do_not_use_it____inPC_Calibration_will_be_replaced_by_inDevice = true)
	{
		if (getStatus() == CAM_STATUS::CAM_IS_STARTED)		return CAM_STATUS::ERROR_CAM_IS_STARTED;
		if (getStatus() == CAM_STATUS::CAM_IS_DISCONNECTED)	return CAM_STATUS::ERROR_CAM_IS_DISCONNECTED;
		m_Param.m_doRectify = toDoRectify;
		m_Param.m_RectWidth = toDoRectify ? rectificationWidth : atoi(getParameterValue("Width_RegionSelector_Region1").c_str());
		return toDoRectify == getDoRectify() ? CAM_STATUS::All_OK : CAM_STATUS::UNKNOWN;
	};

	/** @brief 【不建议再继续使用，请使用代替函数 setDoRectify_inPC */
	EXPORT_TO_DLL CAM_STATUS	setDoRectify(const bool toDoRectify = true, const uint32_t& rectificationWidth = 3200, bool do_not_use_it_any_more = true)
	{
		return setDoRectify_inPC(toDoRectify, rectificationWidth);
	};

	/**
	* @brief 【不建议再继续使用，请使用In-Device标定。】
	* 读取标定的矫正系数（PC 标定）。如果要读取 in-device 标定的校正系数，请读取参数 【Scan3dRectificationSpread_Scan3dExtractionSelector_Scan3dExtraction1】 \n\n
	* Get rectification spread.
	* \n
	* @note 详见标定过程的介绍。\n\n
	* More details please see the user documents.
	* \n
	* @note 建议在相机处于 connected 或者 Stopped 状态时，执行该操作。\n\n
	* Camera should be connected or stopped.
	*/
	EXPORT_TO_DLL double		getRectSpread_inPC(bool do_not_use_it____inPC_Calibration_will_be_replaced_by_inDevice = true)	const { return m_Param.m_RectSpread; };
	
	/** @brief 【不建议再继续使用，请使用代替函数 getRectSpread_inPC */
	EXPORT_TO_DLL double		getRectSpread(bool please_do_not_use_it_any_more = true)	const { return m_Param.m_RectSpread; };

	/**
	* @brief 【不建议再继续使用，请使用In-Device标定。标定文件在相机中，无需给额外定标定文件路径】读取标定文件的路径 \n\n
	* Get calibration file path.
	* \n
	* @return
	* - "" 空字符串表示未导入标定文件，输出未标定的原始数据据。  Empty string. No calibration file, output uncalibrated image.
	* \n
	* @note 建议在相机处于 connected 或者 Stopped 状态时，执行该操作。\n\n
	* Camera shoudl be connected or stopped.
	*/
	EXPORT_TO_DLL std::string	getCalibrationPath_inPC(bool do_not_use_it____inPC_Calibration_will_be_replaced_by_inDevice = true)	const { return m_Param.m_CaliPath; };
	
	/** @brief 【不建议再继续使用，请使用代替函数 getCalibrationPath_inPC */
	EXPORT_TO_DLL std::string	getCalibraPath(bool please_do_not_use_it_any_more = true)	const { return getCalibrationPath_inPC(); };

	/** @brief 【不建议再继续使用，请使用代替函数 setParameter("EncoderReset", "") 】重置编码器计数，等价于 excuteCommand("EncoderReset")。 	*/
	EXPORT_TO_DLL bool			resetEncoder(bool do_not_use_it_any_more = true);

	/**
	* @brief 【不建议再继续使用，请使用In-Device标定，setCalibration_inDevice_CalibrateAC()】读取是否进行矫正（PC 标定）。 in-device 暂不支持。 \n\n
	* Get to rectify or not.
	* \n
	* @note 建议在相机处于 connected 或者 Stopped 状态时，执行该操作。\n\n
	* Camera shoudl be connected or stopped.
	*/
	EXPORT_TO_DLL bool			getDoRectify_inPC(bool do_not_use_it____inPC_Calibration_will_be_replaced_by_inDevice = true)	const { return m_Param.m_doRectify; };
	
	/** @brief 【不建议再继续使用，请使用代替函数 getDoRectify_inPC */
	EXPORT_TO_DLL bool			getDoRectify(bool please_do_not_use_it_any_more = true)	const { return getDoRectify_inPC(); };

	/** @brief 【不建议再继续使用，请使用代替函数 setCalibration_No(), setCalibration_inDevice(), setCalibration_inPC() 】设置标定文件的路径 / Set calibration file path. */
	EXPORT_TO_DLL CAM_STATUS	setCalibraPath(const Str& do_not_use_it_any_more);

#endif





	/**
	* @brief 【不建议再继续使用，请使用代替函数 setParameter("Some-Command", "") 】执行相机的某些指令。 \n\n
	* Excute device command.
	* \n
	* @param [in] CommandName		指令名。  Command name.
	*  \n
	* @return
	* - true  设置成功。
	* - false 设置失败。
	* \n
	* @note 相机必须处于 connected，或者 stopped 状态。\n\n
	* 注意，Ranger3Studio 中的参数，根据功能，可以分为 3 种：这个函数支持 3。\n
	* - 1 数值型：整数、小数、字符串；Ranger3Studio 上体现为【输入框 / Editbox】。\n
	* - 2 使能型：布尔（使用 0，1 表示）；Ranger3Studio 上体现为【勾选框 / CheckBox】。\n
	* - 3 命令型：Ranger3Studio 上体现为【按钮 / Button】。如需使用此类型参数，请调用 excuteCommand()\n
	* The camera must be connected or stopped when calling this.
	* \n
	* * @note 相机参数名的推导。\n\n
	* 例子一：\n
	* 在 Ranger3Studio 中的多级参数结构中，如果有如下的结构。\n
	* 1. 选择页面 ImageFormatControl\n
	* 2. 选择 RegionSelector 的下拉框，再选择 ComponentSelector 的下拉框，然后找到 ComponentEnable 勾选框（0/1）。\n
	* RegionSelector_Scan3dExtraction1 \n
	*                                |____ComponentSelector_Reflectance \n
	*                                                                  |____ComponentEnable \n
	* 3. 仿造下面的结果，可以推导出适当的参数名。\n
	* 上例中，推导的参数名为：ComponentEnable_ComponentSelector_Reflectance_RegionSelector_Scan3dExtraction1 \n
	* 并且，勾选框的参数可选值为 0/1。\n
	*  \n
	* 例子二：\n
	* 在 Ranger3Studio 中的多级参数结构中，如果有如下的结构。\n
	* 1. 选择页面 DeviceControl \n
	* 2. 选择 DeviceScanType 下拉框。 \n
	* 3. 上例中，推导的参数名为：DeviceScanType。这里没有多级的 selector 。 \n
	*/
	EXPORT_TO_DLL bool			excuteCommand(cStr& CommandName, bool do_not_use_it_any_more = true);

	/** @brief 【不建议再继续使用，请使用代替函数 importConfiguretionFile() 】设置参数文件路径 / Set parameters file path. */
	EXPORT_TO_DLL CAM_STATUS	setParametPath(const Str& do_not_use_it_any_more);

	/** @brief 【不建议再继续使用，请使用代替函数 exportConfiguretionFile() 】保存参数文件为 csv 格式。/ Save parameters to file. */
	EXPORT_TO_DLL CAM_STATUS	saveParameterToCSV(cStr& do_not_use_it_any_more);

	/** @brief 【不建议再继续使用，请使用代替函数 importConfiguretionFile() 】加载 csv 参数文件。/ Load parameters from file. */
	EXPORT_TO_DLL CAM_STATUS	loadParameterFrCSV(cStr& do_not_use_it_any_more);

	/** @brief 【不建议再继续使用，请使用代替函数 getParametersAsStructureString() 】加载 csv 参数文件。/ Load parameters from file. */
	EXPORT_TO_DLL Str			getAllParameterInfo(bool do_not_use_it_any_more__replaced_by_getParametersAsStructureString) const;

#pragma endregion

protected:
	Ranger3				() = delete;
	Ranger3				(const Ranger3& r) = delete;
	Ranger3& operator=	(const Ranger3& r) = delete;
	CAM_STATUS  _initDevice(const SPtr<Ranger3Shared> & RS, cStr & ip_mac, const bool useIP);
	CAM_STATUS  _freeDevice						(); /**< invoke when cam is lost */

	void		_setThreadPriority				(GNodeMap & dataStreamNodeMap);
    void		_praseData						(
#ifdef __linux__
		GenTLApi* tl,
#else
		SPtr<GenTLApi> tl,
#endif
		Vec<GBufferH>& bufferHandles, const size_t& bufferId, ImgT & imgTable);

	void _checkRangeA();

    void		_convertAndCalibrate			(ImgT & imgTable, cStr& region, cStr& component);

	CAM_STATUS	_grabSetting					(cStr & calib_path_or_json_string, 
												#ifndef DISABLE_CAL_IN_PC
														const RectMethod & method,
												#endif
														const float & missingData, const bool doRectify, const double & rectificationSpread);
	void		_chunkDataSetting				();
	void		_reflectanceCheck				(); /**< Scan3dExtraction1 */
	void		_scatterCheck					(); /**< Scan3dExtraction1 */
	bool		_inDeviceCalibrationCheck       (); /**< in Device calibration */


	CAM_STATUS	_connectCamera					();
	CAM_STATUS	_getImageData					(ImgT & imgTable);
	CAM_STATUS	_disconnectCamera				();
	CAM_STATUS	_startAcquisition				();
	CAM_STATUS	_stopAcquisition				();

	/**
	* @brief Traverse all parameters in the device.													   
	* \n
	* @note
	*	1. The parameters information wil be saved as text file named "parameter.tmp". 		   
	*	2. Parse the file to get all parameter info.
	*/
	CAM_STATUS	_scanDeviceParameter			(cStr path = "parameter.tmp");
	void        _ConvertUint16ToFloat			(uint8_t * p16In, uint8_t * pFloatOut, const size_t & s, const float& offerZ, const float& scaleZ) const;
	

#ifdef CALLBACK_NEW

	void	_callback_run						();
	void	_callback_run_on_grabbed			(ImgT * _img);

	void	_check_HeartBeats_run				();
	Str		_getDeviceInfo_when_lost			() const;
#endif

private:
	R3SPtr				m_pR3S;					/**< shared parts.		   */
	DCPtr				m_device;				/**< the device.		   */
	ConsumerPtr			m_consumer;				/**< used in closing camera*/
	ChunkPtr			m_chunkAdapter;
	GDeviceH			m_deviceHandle;
	GNodeMap			m_deviceNodeMap;

#ifndef DISABLE_CAL_IN_PC
	CalibrationPtr		m_pCalibrationWrapper;
#endif

	ParameterVector		m_Param;				/**< contain all user parameters.*/
	PartInfosPtr		m_pParts;
	ImgInfo				m_info;


#ifdef _WIN32
    NodeMap				m_deviceNode;
    std::map<UserSetId, UserSet>
                        m_userSets;
#endif
	bool				m_ChunkModeActive;		/**< will effect Image Grab Mode*/
	bool				m_isUsingReflectance;
	bool				m_isUsingScatter;
	bool				m_IsOutputSensor;		/**< false means range, true means sensor*/

	bool				m_keepCalibratedDataAsUint16 = false;	/**< In-device calibration */
	bool				m_keepCalibratedAC_DataX_AsUint16 = false;	     /**< In-device calibrationAC */
	bool				m_convCalibratedDataToFloat_enableOMP = false;	/**< In-device calibration */

	uint32_t			m_scatterSize;
	uint32_t			m_payloadSize;
	int64_t				m_buffer16Size;
	int64_t				m_previousImageID = 0;
	Str					m_DeviceName;			/**< "SICKGigEVisionTL_DEV_" + mac + "_Ranger3"*/
	std::string			m_DeviceIP;				/**< IP */
	Str					m_DeviceModelName;		/**< Parameter: DeviceModelName*/
	Str					m_DeviceVersion;		/**< Parameter: DeviceVersion*/
	Str					m_DeviceFirmwareVersion;/**< Parameter: DeviceFirmwareVersion*/
	Str					m_DeviceSerialNumber;	/**< Parameter: DeviceSerialNumber*/
	CAM_STATUS			m_status;				  
	bool				m_canStop;				/**< to protect ImageGrabber;*/
	bool				m_calibration_inDevice;	/**< Whether calibtatting in device;*/
	bool				m_calibration_inDevice_CalibratedAC=false;
	SPtr<CustomerLog>	m_log;
	CamType				m_camType;
	SPtr<ChunkDataExtractor>  m_FastExtractor;

	SPtr<EnumSelectorEntries> m_regionLookup;
	SPtr<EnumSelectorEntries> m_componentLookup;
	
#ifdef CALLBACK_NEW

	// Get Image
	bool				m_callback_is_on;
	bool				m_callback_is_finish_copy;
	bool				m_callback_require_stop;
	CallbackEvent_GetImage
						m_callBack_function;
	void*				m_callBack_inputs;		/**< parameters of callback function.*/
	std::mutex			m_callback_is_finish_copy_locker;
	std::mutex			m_callback_require_stop_locker;
	uint32_t            m_grab_interval=0;		/**< Grab sleep time, ms */

	// Get lost

    uint32_t            m_heartbeat_interval;   /**< Heartbeat checks */
    CallbackEvent_HeartBeats
                        m_on_lost_function;     /**< Heartbeat checks: callback function.*/
    void*				m_on_lost_inputs;		/**< Heartbeat checks: parameters of callback function.*/
    std::string         m_on_lost_mac;          /**< Heartbeat checks: using mac to reconnect.*/
	int                 m_heartbeat_is_on={2};  /**< Heartbeat checks: 0(require to stop) 1(is running) 2(quited)*/
    uint64_t            m_heartbeat_count;

	// Debug
	struct callbackInfo
	{
		callbackInfo(
			unsigned long	_thread_id,
			bool			_callback_is_on,
			bool			_callback_require_stop,
			std::string     _start_time
		)
		{
			m_thread_id				= _thread_id;
			m_callback_is_on		= _callback_is_on;
			m_callback_require_stop = _callback_require_stop;
			m_start_time			= _start_time;
		}

		//uint64_t		m_image_id;
		unsigned long	m_thread_id;
		bool			m_callback_is_on;
		bool			m_callback_require_stop;
		std::string     m_start_time;
	};
	std::map<uint64_t, callbackInfo> m_callbackDebug;
	std::unique_ptr<ThreadPool>		 m_callbackPool;

#endif // CALLBACK_NEW

    };


////////////////////////////////////////////////////////////////////////////////

#ifdef CALLBACK_NEW


#endif


}


typedef SickCam::Ranger3Shared	R3S;
typedef SickCam::Ranger3		R3;
typedef SickCam::CAM_STATUS		Ecode;

