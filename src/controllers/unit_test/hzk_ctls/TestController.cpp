#include "TestController.h"
#include "kits/required/log/CRossLogger.h"
#include "service/AppFramework.h"
#include <qglobal.h>
#include <qlogging.h>
#include <qvariant.h>

using namespace _Controllers;
using namespace _Kits;
using namespace _Service;
void TestTaskController::testDeviceInfo(const QVariant &data)
{
    // [Unit Test0] 信号传递验证: 无耦合转发信号及数据

    QVariantMap locationAdjust;
    locationAdjust["distance"] = 250.0;
    locationAdjust["speed"] = m_index++;
    QVariantMap mapData;
    mapData["page"] = "mainPage";
    mapData["data"] = locationAdjust;
}

void TestQmlController::testPageChanged(const QVariant &data)
{
    LogInfo("onPageChanged: {}", data.toString().toStdString());
    return;
}
