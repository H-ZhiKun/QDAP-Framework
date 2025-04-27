#include "DeviceInfoController.h"
#include "kits/required/log/CRossLogger.h"
#include "service/AppFramework.h"
#include "tis_global/Function.h"
#include <qglobal.h>
#include <qlogging.h>
#include <qvariant.h>

using namespace _Controllers;
using namespace _Kits;
using namespace _Service;
void DeviceInfoController::doDiskInfo(const QVariant &data)
{
    _Kits::LogInfo("recv doDiskInfo");
    auto res = App().invokeModuleAsync(TIS_Info::QmlPrivateEngine::testArgs, (int)0, std::string("123"), (float)4);
    auto str = App().invokeModuleSync<std::string>(TIS_Info::QmlPrivateEngine::testArgs, (int)0, std::string("789"), (float)4);
    _Kits::LogInfo("recv doDiskInfo end = {},{}", res, str.c_str());
}
