#pragma once
#include "kits/required/controller_base/ControllerBase.h"
#include "tis_global/Function.h"
#include <qvariant.h>
namespace _Controllers
{
    class DeviceInfoController : public ControllerBase<DeviceInfoController>
    {
      public:
        void doDiskInfo(const QVariant &);
        TASK_LIST_BEGIN
        ASYNC_TASK_ADD(TIS_Info::DeviceManager::notifyDiskInfo, DeviceInfoController::doDiskInfo);
        TASK_LIST_END
      private:
        int m_index = 0;
    };
} // namespace _Controllers
