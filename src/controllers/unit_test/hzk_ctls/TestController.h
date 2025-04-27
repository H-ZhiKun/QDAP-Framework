#pragma once
#include "kits/required/controller_base/ControllerBase.h"
#include "kits/required/controller_base/QmlControllerBase.h"
#include "tis_global/EnumClass.h"

namespace _Controllers
{
    class TestTaskController : public ControllerBase<TestTaskController>
    {
      public:
        void testDeviceInfo(const QVariant &);
        TASK_LIST_BEGIN
        ASYNC_TASK_ADD(TIS_Info::DeviceManager::notifyDiskInfo, TestTaskController::testDeviceInfo);
        TASK_LIST_END
      private:
        int m_index = 0;
    };

    class TestQmlController : public QmlControllerBase<TestQmlController>
    {
      public:
        void testPageChanged(const QVariant &);
        void testMVBTest(const QVariant &);
        static void testTaskTest(const QVariant &);
        static void testTaskControllerTest(const QVariant &);

        QML_LIST_BEGIN
        QML_ADD(TIS_Info::QmlCommunication::QmlActions::PageChange, TestQmlController::testPageChanged);
        QML_LIST_END
    };
} // namespace _Controllers
