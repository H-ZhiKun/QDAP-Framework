#pragma once
#include "kits/required/controller_base/ControllerBase.h"
namespace _Controllers
{
    class TestDatabase : public ControllerBase<TestDatabase>
    {
      public:
        void testCURD(const QVariant &);
        void insert();
        void select();
        TASK_LIST_BEGIN
        ASYNC_TASK_ADD(TIS_Info::DeviceManager::notifyDiskInfo, TestDatabase::testCURD);
        TASK_LIST_END
    };
} // namespace _Controllers