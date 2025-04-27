#pragma once
#include <QVariantMap>

namespace _Kits
{

class DeviceInfo
{
  public:
    explicit DeviceInfo() = default;
    virtual ~DeviceInfo() noexcept = default;
    DeviceInfo(const DeviceInfo &) = delete;
    DeviceInfo &operator=(const DeviceInfo &) = delete;
    virtual bool start() = 0;
    virtual void generate() = 0;
    virtual QVariantMap deviceInfo() = 0;
    virtual QVariantMap logicalDriveInfo() = 0;
};

} // namespace _Kits
