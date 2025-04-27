#pragma once
#include "DeviceInfo.h"
#include <Windows.h>
#include <mutex>
#include <pdh.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace _Kits
{

class WindowsInfo : public DeviceInfo
{
    class InfoDetails
    {
      public:
        InfoDetails() = default;
        InfoDetails(PDH_HQUERY query,
                    const std::wstring &title,
                    const std::wstring &counterName);
        ~InfoDetails();
        std::string title();
        double value();

      private:
        std::string m_title;
        PDH_HCOUNTER m_counter = NULL;
        double m_value = 0.0f;
    };

    enum class TypeOccupation
    {
        cpu_total = 0,
        cpu_exe,
        memory_total,
        memory_exe,
        disk_total,
        disk_exe,
        network_send_total,
        network_send_exe,
        network_recv_total,
        network_recv_exe
    };

  public:
    explicit WindowsInfo();
    virtual ~WindowsInfo() noexcept;
    WindowsInfo(const WindowsInfo &) = delete;
    WindowsInfo &operator=(const WindowsInfo &) = delete;
    virtual bool start() override;
    virtual void generate() override;
    virtual QVariantMap deviceInfo() override;
    virtual QVariantMap logicalDriveInfo() override;

  protected:
    std::vector<std::wstring> getProcesses();
    std::wstring getOccupation(TypeOccupation type,
                               const std::wstring &processName);
    void primaryCounter();
    void resetCounter();
    static std::string fromWString(const std::wstring &wstr);

  private:
    // PDH 性能计数器相关变量
    PDH_HQUERY m_query = NULL; // 查询句柄
    std::mutex m_mtxCount;
    std::vector<InfoDetails> m_counterPrimary; // 主要计数器（常驻的不变的）
    std::vector<InfoDetails> m_counterCPU;     // CPU 占用率计数器
    std::vector<InfoDetails> m_counterMem;     // 内存 占用率计数器
    std::vector<InfoDetails> m_counterDisk;    // 磁盘 占用率计数器
    std::vector<InfoDetails> m_counterNetRecv; // 网络 占用率计数器
    std::vector<InfoDetails> m_counterNetSend; // 网络 占用率计数器
    std::unordered_map<TypeOccupation, std::wstring> m_mapCounterName;
};

} // namespace _Kits
