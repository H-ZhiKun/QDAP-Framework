/**	@file	Trispector.h
*
*	@brief	Class Trispector Head File.
*
*	@attention
*	This is the Trispector GenIcam API wrapper.
*
*
*	@copyright	Copyright 2016-2020 SICK AG. All rights reserved.
*	@author		Vision Lab, SICK GCN
*
*/

#pragma once

#include "CameraShared.h"

#ifdef CALLBACK_NEW
#include <thread>
#include <mutex>
#endif

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#include<WINSOCK2.H>
#include<STDIO.H>


namespace SickCam
{

typedef UPtr<PartInfoCollection>	PartInfosPtr;
typedef SPtr<DeviceConnection>		DCPtr;
typedef CameraShared				TrispectorShared;

/**	
* @brief 每个 Trispector 对应应该初始化为一个 类Trispector 的对象。\n\n
* Each Trispector device corresponding to a Trispector class object.
*/																						  
class Trispector final																		  
{		
public:
	/**
	* @brief Trispector 相机专用初始化函数。 \n\n
	* For Trispector.
	*
	* @param [in] TS		智能指针shared_ptr，Trispector 的运行环境。 Shared ptr, the GenICam environment.
	* @param [in] sn		相机的 SN 号。Trispector 必须使用 SN 来指定相机。SN is the only choice.
	*/
	EXPORT_TO_DLL	Trispector		(const SPtr<TrispectorShared> & TS, cStr & sn);
	
	EXPORT_TO_DLL	~Trispector	();

	/**
	* @brief 获取初始化结果。\n\n
	* To get initialization result.
	*
	* @return
	* - ERROR_NULL_PTR_DEV	初始化失败。  Initialize failed.
	* - CAM_STATUS::All_OK				
	*/
	EXPORT_TO_DLL CAM_STATUS	isReady();

	////////////////////////////////////////////////////////////////////////////////


	/** 
	* @brief 连接相机 \n\n 
	* Connect a camera.
	*	
	* @note 连接相机的动作包括：打开设备句柄，打开数据流句柄，加载参数文件，扫描相机参数。
	* 当相机连接时，采集图像过程并没有开始。\n\n
	* It will open device handle, open datastream, loading parameters file, scan parameters in device.
	* When camera is connected, image acquition is not started.
	*
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED	相机处于 start 状态。  Camera is started.
	* - ERROR_SCAN_PARAMETERS	扫描相机参数时发生错误。 Error happened when scaning parameters in device.
	*/
	EXPORT_TO_DLL CAM_STATUS	connectCamera		();

	/**
	* @brief 开始采集 \n\n
	* Start acquisition.
	*
	* @note 开始采集的动作包括：加载标定文件，申请相机缓存，锁定相机参数，开始采集图像。\n\n
	* It will load calibration file, initialize device buffer, lock device parameter, start acquisition.
	*
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。  Camera is disconnected.
	* - ERROR_START_ACQUISITION		相机开始采集失败。  Failed to start acquisition.
	* - ERROR_CALIBRATION_PATH				使用标定文件初始化标定变量错误。 Error happened when initialzing calibration variables.
	*/
	EXPORT_TO_DLL CAM_STATUS	startCamera			();

#ifdef CALLBACK_NEW

	typedef void(__stdcall *CallbackEvent)			(ImgT* img, void* any);

	/**
	* @brief 开始采集，并启动 callback 。 \n\n
	* Start acquisition and passing callback function
	*
	* @note 开始采集的动作包括：加载标定文件，申请相机缓存，锁定相机参数，开始采集图像。\n
	* Callback 函数中的第一个参数是返回的图像。其内存由 API 管理，在 callback 函数调用结束后，API 将释放 ImgT 中的内存。\n
	* 所以，使用者在拿到数据之后，应该第一时间做“深拷贝”。\n
	* ImgT 中的操作符 = ，已经重载为深拷贝。\n
	*
	* It will load calibration file, initialize device buffer, lock device parameter, start acquisition.
	*
	* @param [in] pCallback		Callback funtion pointer;
	* @param [in] any			在 callback 中要使用的参数。 The parameters will be passed into callback funtion;
	*
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。  Camera is disconnected.
	* - ERROR_START_ACQUISITION		相机开始采集失败。  Failed to start acquisition.
	* - ERROR_CALIBRATION_PATH				使用标定文件初始化标定变量错误。 Error happened when initialzing calibration variables.
	*/
	EXPORT_TO_DLL CAM_STATUS	startCamera(CallbackEvent pCallback, void * any=nullptr);

#endif

	/**
	* @brief 停止采集 \n\n
	* Stop acquisition.
	*
	* @note 停止采集的动作包括：停止采集图像，解锁相机参数，清除相机缓存。本质上 connected 和 stopped 是一样的状态。 \n\n
	* It will stop acquisition, unlock device parameter, free device buffer. Actually, connected equals to stopped.
	*
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_DISCONNECTED	相机处于 disconnect 状态。  Camera is disconnected.
	* - ERROR_STOP_ACQUISITION		相机停止采集失败。  Failed to stop acquisition.
	*/
	EXPORT_TO_DLL CAM_STATUS	stopCamera			();
	
	/**
	* @brief 断开相机连接 \n\n
	* Disconnect a camera.
	*
	* @note 断开相机连接的动作包括：关闭数据流句柄，关闭设备句柄。\n\n
	* It will close datastream, close device handle.
	*
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED	相机处于 start 状态。  Camera is started.
	* - UNKNOW					未知错误。  Unknown problem.
	*/
	EXPORT_TO_DLL CAM_STATUS	disconnectCamera	();

	/**
	* @brief 获取图像 \n\n
	* Get output images.
	*
	* @param [out] imgTable		Images data;
	*
	* @note 相机必须处于 start 状态时才可以采集图像。请注意，非线程安全。\n\n
	* Camera must be started! And it is not thread-safe.
	*
	* @return
	* - CAM_STATUS::All_OK
	* - CAM_STATUS::ERROR_CAM_IS_STARTED	相机处于 start 状态。  Camera is started.
	*/
	EXPORT_TO_DLL CAM_STATUS	getImageData		(ImgT & imgTable);


	////////////////////////////////////////////////////////////////////////////////

	/**
	* @brief 设置 Y 方向分辨率 \n\n
	* Set scale in Y axis.
	*
	* @param [in] value		value = (y_range_in_mm) / (profiles_number).
	*	
	* @note
	* 点云 y 坐标 = Y_offset + Y_scale * Y_pixelValueInRange。 \n\n
	* Y value of a pointcloud = Y_offset + Y_scale * Y_pixelValueInRange.
	*
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
	* @brief 设置相机取图的最大等待时间，默认值见 DEFAULT_TIMEOUT  \n\n
	* Set timeout.The defalut is DEFAULT_TIMEOUT 
	*
	* @param [in] value		相机取图的最大等待时间，单位毫秒。   Maximum waiting time when grabbing image, unit is millimeter.
	*
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
	*
	* @param [in] value		图像缓存的张数。  The number of image buffers.
	*
	* @note 默认是 20 ，最小可以设置为 1。设置越大，内存占用越多。设置太小，有丢帧风险。\n\n
	* The default is 20, avaliable minimum value can be 1. The bigger value, the more memory will be allocated. The smaller value, the higher risk of miss a frame.
	*
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

	////////////////////////////////////////////////////////////////////////////////

	//	Get the user parameters.

	/**
	* @brief 读取 Y 方向分辨率 \n\n
	* Get scale in Y axis.
	*
	* @note
	* 点云 y 坐标 = Y_offset + Y_scale * Y_pixelValueInRange。 \n\n
	* Y value of a pointcloud = Y_offset + Y_scale * Y_pixelValueInRange.
	*
	* @note 建议在相机处于 connected 或者 Stopped 状态时，执行该操作。\n\n
	* Camera shoudl be connected or stopped.
	*/
	EXPORT_TO_DLL double		getScaleY				()	const	{	return m_Param.m_scaleY;	};
	
	/**
	* @brief 读取相机取图的最大等待时间，单位毫秒，默认值见 DEFAULT_TIMEOUT \n\n
	* Get maximum waiting time when grabbing image, unit is millimeter. The defalut is DEFAULT_TIMEOUT.
	*
	* @note 建议在相机处于 connected 或者 Stopped 状态时，执行该操作。\n\n
	* Camera shoudl be connected or stopped.
	*/
	EXPORT_TO_DLL uint32_t		getTimeOut				()	const	{	return m_Param.m_timeOut;	};
	
	/**
	* @brief 读取图像缓存的张数 \n\n
	* Get the number of image buffers.
	*
	* @note 默认是 20 ，最小可以设置为 1。设置越大，内存占用越多。设置太小，有丢帧风险。
	* 建议在相机处于 connected 或者 Stopped 状态时，执行该操作。\n\n
	* The default is 20, avaliable minimum value can be 1. The bigger value, the more memory will be allocated. The smaller value, the higher risk of miss a frame.
	* Camera shoudl be connected or stopped.
	*/
	EXPORT_TO_DLL uint32_t		getBufferCount			()	const	{	return m_Param.m_buffersCount;	};
	
	/**
	* @brief 设备名格式： "SICKGigEVisionTL_DEV_" + mac + "_Trispector" \n\n
	* Device name format : "SICKGigEVisionTL_DEV_" + mac + "_Trispector" 
	*
	* @return
	* - "" 错误返回空字符串。  return "" when error happened.
	*
	* @note 支持任何相机状态下读取。\n\n
	* It can be queried in any camera status..
	*/
	EXPORT_TO_DLL cStr			getDeviceName			()	const	{	return m_DeviceName;	}
	EXPORT_TO_DLL cStr			getDeviceSN				()	const	{	return m_DeviceSN;		}


	////////////////////////////////////////////////////////////////////////////////

	/**
	*@brief 读取相机状态。  \n\n
	* Get camera status.
	*
	* @return 
	* - CAM_IS_CONNECTED 相机已连接 Connected
	* - CAM_STATUS::CAM_IS_STARTED 相机开始采集 Start acquisition
	* - CAM_IS_STOPPED 相机停止采集 Stop acquisition
	* - CAM_STATUS::CAM_IS_DISCONNECTED 相机未连接 Disconnected
	* - ERROR_NULL_PTR_DEV 类 Trispector 的对象未初始化完成。 Class Trispector initialization undone.
	*/
	EXPORT_TO_DLL CAM_STATUS	getStatus				()	const	{ return m_status; };

	/**
	*@brief 读取相机类型。  \n\n
	* Get camera type.
	*
	* @return
	* - _Trispector 
	* - _TRISPECTOR
	*/
	EXPORT_TO_DLL CamType		getCamType				()	const	{ return m_camType; };
	
	/** 
	* @brief 读取图像序号，也是当前取到的图像总数，stop 后清理。  \n\n
	* Get image ID. The value is also the number of grabbed image. Reset to 0 when stopped.
	* 
	* @note 0初始。相机必须处于 connected 或者 stopped 状态。\n\n
	* Start from 0. The camera must be connected or stopped when calling this.
	*/
	EXPORT_TO_DLL uint64_t		getImgID				()	const	{ return m_info.m_id; };


	////////////////////////////////////////////////////////////////////////////////

	EXPORT_TO_DLL bool			socketInit				(const std::string& ip, const int& port);
	EXPORT_TO_DLL std::string	socketSendCommand		(const std::string & command);



protected:
	Trispector() = delete;
	Trispector(const Trispector& r) = delete;
	Trispector& operator = (const Trispector& r) = delete;

	void		_setThreadPriority				(GNodeMap & dataStreamNodeMap);
	CAM_STATUS	_connectCamera					();
	CAM_STATUS	_getImageData					(ImgT & imgTable);
	CAM_STATUS	_disconnectCamera				();
	CAM_STATUS	_startAcquisition				();
	CAM_STATUS	_stopAcquisition				();

#ifdef CALLBACK_NEW

	void	_callback_run						();
	void	_callback_run_on_grabbed			(ImgT * _img);

#endif

private:
	SPtr<TrispectorShared>	m_pR3S;				/**< shared parts.		   */
	DCPtr				m_device;				/**< the device.		   */
	ConsumerPtr			m_consumer;				/**< used in closing camera*/
	GDeviceH			m_deviceHandle;			 
	GNodeMap			m_deviceNodeMap;		 
	ParameterVector		m_Param;				/**< contain all user parameters.*/
	PartInfosPtr		m_pParts;				  
	ImgInfo				m_info;
	bool				m_ChunkModeActive;		/**< will effect Image Grab Mode*/
	bool				m_isUsingReflectance;	  
	bool				m_IsOutputSensor;		/**< false means range, true means sensor*/
	int64_t				m_buffer16Size;			 
	size_t				m_payloadSize;			  
	Str					m_DeviceName;			/**< "SICKGigEVisionTL_DEV_" + mac + "_Trispector"*/
	Str					m_DeviceSN;				
	CAM_STATUS			m_status;				  
	bool				m_canStop;				/**<  to protect ImageGrabber;*/
	SPtr<CustomerLog>	m_log;
	CamType				m_camType;
	SOCKET				m_socket;

	static std::map<std::string, DN> gDNT;

#ifdef CALLBACK_NEW
	bool				m_callback_is_on;
	CallbackEvent		m_callBack_function;
	void*				m_callBack_inputs;		/**< parameters of callback function.*/
	bool				m_callback_is_finish_copy;
	std::mutex			m_callback_is_finish_copy_locker;
	bool				m_callback_require_stop;
	std::mutex			m_callback_require_stop_locker;
#endif

};


}

