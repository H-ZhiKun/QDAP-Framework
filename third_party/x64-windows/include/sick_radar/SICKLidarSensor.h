#pragma once

#ifdef _WIN32
#ifdef  SICKLSAPI
#define SICKLSAPI  _declspec(dllexport)
#else
#define SICKLSAPI  _declspec(dllimport)
#endif
#else
#define SICKLSAPI __attribute__((visibility("default")))
#endif

#include <string>  
#include <queue>
#include <thread>
#include "SICKLidarSensorData.h"

namespace SICK
{
	extern "C" class SICKLSAPI  SICKLidarSensor
	{
		/*=======================常用功能==========================*/
	public:
		/// <summary>
		/// 连接传感器，并执行初始化工作
		/// </summary>
		/// <param name="err">错误代码</param>
		/// <param name="lmsIPAddr">传感器IP地址</param>
		/// <returns>true:初始化成功，false:初始化失败</returns>
		bool init(CommonError& err, const std::string& lmsIPAddr);

		/// <summary>
		/// 断开连接，并注销资源
		/// </summary>
		void destory();

		/// <summary>
		/// 获取单帧轮廓数据
		/// 注：当Echo Filter设置为First或All时，该接口获取的为首次回波的数据；当Echo Filter设置为Last时，该接口获取的为最后一次回波的数据。
		/// </summary>
		/// <param name="singleProfile">单帧轮廓数据</param>
		/// <param name="err">错误代码</param>
		/// <returns>
		///返回1表示：成功获取LMS的单帧轮廓数据
		///返回0表示：无可用的点云数据
		///返回 -1表示：网络通讯异常
		/// </returns>
		int getSingleProfile(std::vector<Point3D>& singleProfile, CommonError& err);
		int getSingleProfile(ProfileInformation& singleProfile, CommonError& err);
		int getSingleProfileBlock(std::vector<Point3D>& singleProfile, CommonError& err, const unsigned int &waitTime=1000);  //阻塞模式，有数据之后返回，waitTime为最大阻塞时间,ms
		int getSingleProfileBlock(ProfileInformation& singleProfile, CommonError& err, const unsigned int &waitTime=1000);  //阻塞模式，有数据之后返回，waitTime为最大阻塞时间,ms

		/// <summary>
		/// 获取最后一次回波的单帧轮廓数据
		/// 注：仅当Echo Filter设置为All时，且需要获取最后一次回波数据时调用。当Echo Filter设置为Fisrt或Last时，该接口不生效。
		/// </summary>
		/// <param name="singleProfile">单帧轮廓数据</param>
		/// <param name="err">错误代码</param>
		/// <returns>
		///返回1表示：成功获取LMS的单帧轮廓数据
		///返回0表示：无可用的点云数据
		///返回-1表示：网络通讯异常
		///返回-2表示：还未开启多次回波，需要先调用setEchoFilter(1)开启多次回波
		/// </returns>
		int getSingleProfileLastEcho(std::vector<Point3D>& singleProfile, CommonError& err);
		int getSingleProfileLastEcho(ProfileInformation& singleProfile, CommonError& err);
		int getSingleProfileLastEchoBlock(std::vector<Point3D>& singleProfile, CommonError& err, const unsigned int& waitTime = 1000);   //阻塞模式，有数据之后返回，waitTime为最大阻塞时间,ms
		int getSingleProfileLastEchoBlock(ProfileInformation& singleProfile, CommonError& err, const unsigned int& waitTime = 1000);   //阻塞模式，有数据之后返回，waitTime为最大阻塞时间,ms

		/// <summary>
		/// 获取所有回波的单帧轮廓数据
		/// 注：仅当Echo Filter设置为All时，且需要获取全部回波数据时调用。当Echo Filter设置为Fisrt或Last时，该接口不生效。
		/// </summary>
		/// <param name="singleProfile">单帧轮廓数据</param>
		/// <param name="err">错误代码</param>
		/// <returns>
		///返回1表示：成功获取LMS的单帧轮廓数据
		///返回0表示：无可用的点云数据
		///返回-1表示：网络通讯异常
		///返回-2表示：还未开启多次回波，需要先调用setEchoFilter(1)开启多次回波
		/// </returns>
		int getSingleProfileAllEchoBlock(std::vector<ProfileInformation>& singleProfiles, CommonError& err, const unsigned int& waitTime = 1000);

		/// <summary>
		/// 模块内部是否均在正常运行，如果不正常，需要重新init
		/// </summary>
		/// <returns>true:正常，false:异常</returns>
		bool isRunningOK();

		/// <summary>
		/// 是否移除轮廓中的无效点，默认不移除。移除无效点之后，可能会出现不同轮廓之间数据点数量不一样的情况
		/// </summary>
		/// <param name="flag">false:不移除， true:移除</param>
		/// <returns></returns>
		bool setRemoveZeroPoints(bool flag = false);

		/// <summary>
		/// 是否开启数据永久输出，仅连接LMS4000时调用。开启后可以获取轮廓数据，但无法配置和获取雷达参数。若需要获取和配置参数，需要先关闭该模式
		/// </summary>
		/// <param name="flag">false:关闭， true:启用</param>
		/// <returns>
		/// 返回1表示：设置成功
		/// 返回0表示：设备未连接
		/// 返回-1表示：数据格式错误
		/// 返回-2表示：设置失败
		/// 返回-3表示：当前雷达不支持该功能
		/// </returns>
		int setSendDataPermanently(bool flag);

		/*=======================获取传感器的状态信息==========================*/
	public:
		/// <summary>
		/// 获取传感器产品序列号
		/// </summary>
		/// <param name="serialNo">序列号</param>
		/// <returns>true:成功，false:失败</returns>
		bool getSerialNo(unsigned int& serialNo);

		/// <summary>
		/// 获取传感器内部温度
		/// </summary>
		/// <param name="temperature">温度值</param>
		/// <returns>true:成功，false:失败</returns>
		bool getTemperature(float& temperature);

		/// <summary>
		/// 获取设备类型
		/// </summary>
		/// <param name="deviceType">设备类型</param>
		/// <returns>true:成功，false:失败</returns>
		bool getDeviceType(SensorType& deviceType);

		/// <summary>
        /// 获取LMS系列雷达的污染状态
		/// </summary>
		/// <param name="level">污染等级指数，0：无污染，1：污染警告，2：污染报警，3：污染检测功能异常</param>
		/// <param name="err">错误代码</param>
		/// <returns>		
		///返回1表示：成功
		///返回-1表示：连接异常
		///返回-2表示：当前激光雷达型号不支持该功能
		///返回-3表示：无法读取污染状态
		///</returns>
		int getLMSContaminationStatus(int& level, CommonError& err);

        /// <summary>
        /// 获取雷达的污染状态（支持LRS4000，picoscan）
        /// </summary>
        /// <param name="conData">每个扇区的污染数据，0：无污染，1：污染警告，2：污染报警，3：污染检测功能异常或未激</param>
        /// <param name="err">错误代码</param>
        /// <returns>
        ///返回1表示：成功
        ///返回-1表示：连接异常
        ///返回-2表示：当前激光雷达型号不支持该功能
        ///返回-3表示：无法读取污染状态
        ///</returns>
        int getContaminationStatus(std::vector<int>& conData, CommonError& err);

        /// <summary>
        /// 获取雷达的工作距离范围
        /// </summary>
        /// <param name="minDist">最远距离，mm</param>
        /// <param name="maxDist">最近距离,mm</param>
        /// <returns>
        ///返回1表示：成功
        ///返回-1表示：连接异常
        ///返回-2表示：当前激光雷达型号不支持该功能
        ///返回-3表示：无法读取污染状态
        ///</returns>
        int getDistanceRange(float& minDist,float& maxDist);

		/*=======================设置/读取传感器的配置（修改配置会进去配置模式，不能获取轮廓数据。修改之后需要永久保存）==========================*/
	public:
		/// <summary>
		/// 永久保存雷达的参数，保存后断电不会丢失 。注：LMS系列保存参数会需要比较多的耗时
		/// </summary>
		/// <returns>
		/// 返回1表示：成功
		/// 返回0表示：设备未连接
		/// 返回-1表示：保存失败
		/// </returns>
		int saveSettingsPermanently();

		/// <summary>
		/// 重启设备。   注：重启之后SDK会断开连接，需要重新init
		/// </summary>
		/// <returns>true:成功，false:失败</returns>
		bool rebootDevice();

		/// <summary>
		/// 修改IP地址，修改成功后，设备会自动重启
		/// </summary>
		/// <param name="LMSIP">新的扫描仪IP</param>
		/// <returns>
		/// 返回1表示：修改成功
		/// 返回0表示：设备未连接
		/// 返回-1表示：IP地址格式错误
		/// 返回-2表示：修改IP地址失败
		/// 返回-3表示：当前雷达不支持该功能
		/// </returns>
		int modifyIPAdress(std::string LMSIP);

		///<summary>
		/// 修改和读取子网掩码，修改成功后，设备需要重启
		/// </summary>		
		/// <param name="subnetMask">扫描仪子网掩码</param>
		/// <returns>
		///返回1表示：修改成功
		///返回0表示：设备未连接
		///返回-1表示：子网掩码格式错误
		///返回-2表示：设置/读取出错
		///返回-3表示：当前雷达不支持该功能
		///</returns>
		int setSubnetMask(std::string subnetMask);
		int getSubnetMask(std::string& subnetMask);

		///<summary>
		/// 修改和读取默认网关，修改成功后，设备需要重启
		/// </summary>		
		/// <param name="gateway">扫描仪默认网关</param>
		/// <returns>
		///返回1表示：修改成功
		///返回0表示：设备未连接
		///返回-1表示：默认网关格式错误
		///返回-2表示：设置/读取出错
		///返回-3表示：当前雷达不支持该功能
		///</returns>
		int setDefaultGateway(std::string gateway);
		int getDefaultGateway(std::string& gateway);

		///<summary>
		/// 设置和读取数据流输出的以太网配置。当搭载的雷达为multiscan或picoscan时支持
		/// </summary>
		/// <param name="protocal"> 数据传输方式，1：UDP，2：TCP。 注：SDK采用UDP的方式，若改为TCP，会导致连接异常</param>
		/// <param name="destinationIP"> 数据输出的目标IP地址</param>
		/// <param name="destinationPort"> 数据输出的目标端口号</param>
		/// <returns>
		///返回1表示：设置/读取成功
		///返回0表示：设备未连接
		///返回-1表示：输入参数有误
		///返回-2表示：设置/读取出错
		///返回-3表示：当前雷达不支持该功能
		///</returns>
		int setStreamingEthernetSettings(unsigned short protocal, std::string destinationIP, unsigned short destinationPort);
		int getStreamingEthernetSettings(unsigned short& protocal, std::string& destinationIP, unsigned short& destinationPort);

		/// <summary>
		/// 设置和读取激光雷达的回波方式。注：设置期间无法扫描获取数据；LMS系列保存参数会需要比较多的耗时
		/// </summary>
		/// <param name="echoFlag">回波标志，0:First echo, 1:All echoes, 2:Last echo</param>
		///<returns>
		///返回1表示：设置/读取成功
		///返回0表示：设备未连接
		///返回-1表示：输入参数有误
		///返回-2表示：设置/读取出错
		///返回-3表示：当前雷达不支持该功能
		///</returns>
		int setEchoFilter(unsigned int echoFlag);
		int getEchoFilter(unsigned int& echoFlag);

		///<summary>
		/// 设置/读取是否开启数据流输出。当搭载的雷达为multiscan或picoscan时支持
		/// </summary>
		/// <param name="isOpen"> 开启标志位，0: 关闭，1:打开</param>
		/// <returns>
		///返回1表示：设置/读取成功
		///返回0表示：设备未连接
		///返回-1表示：输入参数有误
		///返回-2表示：设置/读取出错
		///返回-3表示：当前雷达不支持该功能
		///</returns>
		int setScanDataEnable(unsigned int isOpen);
		int getScanDataEnable(unsigned int& isOpen);

		///<summary>
		///设置/读取数据流输出的格式。当搭载的雷达为multiscan或picoscan时支持
		/// </summary>
		/// <param name="isOpen"> 数据格式标志位，1:MSGPACK,  2:Compact。  注：SDK采用Compact的方式，若改为MSGPACK，会导致异常</param>
		/// <returns>
		///返回1表示：设置/读取成功
		///返回0表示：设备未连接
		///返回-1表示：输入参数有误
		///返回-2表示：设置/读取出错
		///返回-3表示：当前雷达不支持该功能
		///</returns>
		int setScanDataFormat(int dataFormat);
		int getScanDataFormat(int& dataFormat);

		///<summary>
		/// 设置/读取雷达的输出角度范围
		/// </summary>
		/// <param name="startAngle"> 起始角度范围，单位：角度</param>
		/// <param name="stopAngle"> 结束角度范围，单位：角度</param>
		/// <returns>
		///返回1表示：设置/读取成功
		///返回0表示：设备未连接
		///返回-1表示：输入参数有误
		///返回-2表示：设置/读取出错
		///返回-3表示：当前雷达不支持该功能
		///</returns>
		int setAngleRange(float startAngle, float stopAngle);
		int getAngleRange(float& startAngle, float& stopAngle);

		///<summary>
		/// 设置/读取雷达的扫描频率和角度分辨率
		/// </summary>
		/// <param name="frequency">扫描频率</param>
		/// <param name="resolution"> 角度分辨率，单位：角度</param>
		/// <returns>
		///返回1表示：设置/读取成功
		///返回0表示：设备未连接
		///返回-1表示：输入参数有误
		///返回-2表示：设置/读取出错
		///返回-3表示：当前雷达不支持该功能
		///</returns>
		int setFrequencyAndResolution(float frequency, float resolution);
		int getFrequencyAndResolution(float& frequency, float& resolution);

		///<summary>
		/// 设置/读取数据输出内容的配置
		/// </summary>
		/// <param name="rssi">是否输出RSSI，0：否，1：是</param>
		/// <param name="rssiType"> RSSI值类型，0：8bit，1：16bit（针对LRS4000)</param>
		/// <param name="encoder">是否输出编码器值，0：否，1：是</param>
		/// <param name="deviceName">是否输出设备名称，0：否，1：是</param>
		/// <param name="timeStamp">是否输出时间戳，0：否，1：是</param>
		/// <param name="outputinterval">输出的轮廓间隔</param>
		/// <returns>
		///返回1表示：设置/读取成功
		///返回0表示：设备未连接
		///返回-1表示：输入参数有误
		///返回-2表示：设置/读取出错
		///返回-3表示：当前雷达不支持该功能
		///</returns>
		int setDataContentConfigure(int rssi, int rssiType, int encoder, int deviceName, int timeStamp, int outputinterval = 1);
		int getDataContentConfigure(int& rssi, int& rssiType, int& encoder, int& deviceName, int& timeStamp, int& outputinterval);

		///<summary>
		/// 设置/读取雾滤波是否启用
		/// </summary>
		/// <param name="isActive"> 开启标志位，1:开启, 0:关闭</param>
		/// <param name="sensitivityLevel">雾过滤等级，取值范围：1~6，该参数仅当雷达为LMS511时生效，其余型号雷达无需设置</param>
		/// <returns>
		///返回1表示：设置/读取成功
		///返回0表示：设备未连接
		///返回-1表示：输入参数有误
		///返回-2表示：设置/读取出错
		///返回-3表示：当前雷达不支持该功能
		///</returns>
		int setFogFilter(unsigned short isActive, unsigned short sensitivityLevel = 3);
		int getFogFilter(unsigned short& isActive, unsigned short& sensitivityLevel);

		///<summary>
		///   设置/读取粒子滤波是否启用
		/// </summary>		
		/// <param name="isActive"> 开启标志位，1:开启, 0:关闭</param>
		/// <returns>
		///返回1表示：设置/读取成功
		///返回0表示：设备未连接
		///返回-1表示：输入参数有误
		///返回-2表示：设置/读取出错
		///返回-3表示：当前雷达不支持该功能
		///</returns>
		int setParticleFilter(unsigned short isActive);
		int getParticleFilter(unsigned short& isActive);

		///<summary>
		///   开始/停止测量。建议连接LMS4000时使用。停止测量后，雷达停止测量数据的输出。LMS4000雷达停止测量后雷达内部电机会停转。
		/// </summary>		
		/// <returns>
		///返回1表示：设置/读取成功
		///返回0表示：设备未连接
		///返回-1表示：输入参数有误
		///返回-2表示：设置/读取出错
		///返回-3表示：当前雷达不支持该功能
		///</returns>
		int startMeasurement();
		int stopMeasurement();

#ifdef USE_ROS
		void transformPoints(bool flag);
		int getSingleProfileBlockROS(ProfileInformation& profileInfo,std::vector<float>& distances, std::vector<float>& intensities, std::vector<float>& reflector, CommonError& err, const unsigned int& waitTime = 1000);
#endif

	public:
		SICKLidarSensor();
		~SICKLidarSensor();

	private:
		void	clearResource();

		friend void LMSThreadCallbk(void* para);
		friend void CompactDataThreadCallbk(void* Para);
		friend void SendPermanentlyThreadCallbk(void* Para);

	private:
		void* m_lidarSensor = nullptr;
		void* m_lidarCompactDataRecipient = nullptr;
		bool m_isInitOk;
		bool m_isChildThreadRunning;
		bool m_isCalculatePointOK;

		bool m_useZeroPointFlag = true;

		SensorType m_deviceType;
		unsigned int m_lmsSerialNo;
		std::string m_lmsIP;
		std::string m_lmsSubnetMask;
		std::string m_lmsDefaultGateway;
		std::string m_lmsUdpIP;
		unsigned short m_lmsUdpPort;
		unsigned short m_lmsProtocal;

		unsigned int m_lmsScanDataEnableFlag;
		int m_lmsDataFormat;
		unsigned int m_lmsEchoFlag;
		float m_lmsFrequency;
		float m_lmsResolution;
		float m_lmsStartAngle;
		float m_lmsStopAngle;

		float m_lmsTemperature;
		int m_lmsRSSI;
		int m_lmsRSSIType;
		int m_lmsEncoder;
		int m_lmsDeviceName;
		int m_lmsTimeStamp;
		int m_lmsOutputInterval;
		int m_lmsScaleFactor;

		int m_lmsIncrementSource;
		int m_lmsEncoderSetting;

		void* m_mutex;
		Single3DPointVec m_sLmsSingleProfile;
		Single3DPointVec m_singleProfileEcho2;
		ProfileInformation m_proileInfo;
		std::vector< Single3DPointVec> m_middleEchoProfiles;
		void* m_ringBuffer=nullptr;

#ifdef USE_ROS
		bool m_isTransform=false;
		std::vector<float> m_distances, m_intensities, m_reflectors;
#endif

#ifdef _WIN32
		void* m_laserScanThread;
#else
		std::thread* m_laserScanThread;
#endif
		unsigned long m_laserScanThreadID;
	};
};
