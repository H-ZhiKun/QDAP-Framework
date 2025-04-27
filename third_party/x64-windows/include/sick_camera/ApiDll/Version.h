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
*/

#pragma once

#include "Typedef.h"
#include "CustomerLog.h"

#define R3_VERSION_1 3  // Major version
#define R3_VERSION_2 4  // Minor version
#define R3_VERSION_3 1  // Patch version
#define R3_VERSION_4 8  // Build number // 请同步更新 rc 中的版本、 CalibrationWrapper、 SampleApiDll_CS、 C# SampleApiDllCSharpWrapper、release notes
#define R3_VERSION_TIME "2023.10" // Build Time


namespace SickCam
{

/** 
* @brief 显示 API 的版本号和更新。 \n\n
* Show the version number and updates of APIs.
*/
EXPORT_TO_DLL class VER
{
public:
	VER	() = default;
	~VER() = default;

	EXPORT_TO_DLL static std::string version_number;
	EXPORT_TO_DLL static std::string version_time;

	/** 
	* @brief 以字符串形式返回版本信息【Ranger3】。 \n\n
	* Return version number and updates as string.
	*/
	EXPORT_TO_DLL static std::string & __version__()			{
#ifdef __linux__
#ifdef __arm__
		return m_version_linux_aarch64;
#else
		return m_version_linux_x64;
#endif // __arm__
#endif // __linux__

#ifdef _WIN32
		return m_version;
#endif // _WIN32
	}

	/**
	* @brief 以字符串形式返回版本信息【Trispector】。 \n\n
	* Return version number and updates as string.
	*/
	EXPORT_TO_DLL static std::string & __version_Trispector()	{ return m_version_trispector; }

private:
	static std::string m_version;
    static std::string m_version_trispector;
	static std::string m_version_linux_x64;
	static std::string m_version_linux_aarch64;
};

EXPORT_TO_DLL void mark_Obsolete(SPtr<CustomerLog> plog, const std::string& deviceName, const std::string& fname_old, const std::string& fname_new);

EXPORT_TO_DLL bool isEnvironmentVariableExisted(const std::string& name);



}

