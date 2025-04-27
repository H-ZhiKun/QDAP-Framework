#include "Version.h"

namespace SickCam
{
std::string VER::version_number = "V" + std::to_string(R3_VERSION_1) +"." + std::to_string(R3_VERSION_2) + "." + std::to_string(R3_VERSION_3) + "." + std::to_string(R3_VERSION_4);
std::string VER::version_time = R3_VERSION_TIME;

std::string VER::m_version_linux_aarch64 = std::string("Current:\n")
	+ "SICK GenICam SDK " + version_number + ", " + version_time + ", Linux, aarch64";

std::string VER::m_version_linux_x64 = std::string("Current:\n")
	+ "SICK GenICam SDK " + version_number + ", " + version_time + ", Linux, x64";

std::string VER::m_version_trispector = std::string("Current:\n")
	+ "SICK GenICam SDK " + version_number + ", " + version_time + ", Windows 10, x64";

std::string VER::m_version =
	std::string("Current:\n")
	+ "SICK GenICam SDK " + version_number +", "+ version_time + ", Windows 10, x64";

void mark_Obsolete(SPtr<CustomerLog> plog, const std::string& deviceName, const std::string& fname_old, const std::string& fname_new)
{
	*plog << CustomerLog::time() << "[" << deviceName << "][" << fname_old << "]: 不建议再继续使用 / Obsolete function! 请使用代替函数 / Replaceed by: " << fname_new << "\n";
	*plog << CustomerLog::time() << "[" << deviceName << "][" << fname_old << "]: 不建议再继续使用 / Obsolete function! 请使用代替函数 / Replaceed by: " << fname_new << "\n";
	*plog << CustomerLog::time() << "[" << deviceName << "][" << fname_old << "]: 不建议再继续使用 / Obsolete function! 请使用代替函数 / Replaceed by: " << fname_new << "\n";
}

bool isEnvironmentVariableExisted(const std::string& name)
{
	char* value = getenv(name.c_str());
	if (value == nullptr)
	{
		return false;
	}
	return true;
}



}
