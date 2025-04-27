#include "WindowsInfo.h"
#include "DeviceInfo.h"
#include "kits/required/log/CRossLogger.h"
#include <QDebug>
#include <algorithm>
#include <codecvt>
#include <fmt/format.h>
#include <json/json.h>
#include <json/value.h>
#include <qdebug.h>
#include <string>
#include <tlhelp32.h>
#include <unordered_map>
#include <vector>

namespace _Kits
{

#pragma comment(lib, "pdh.lib")
    WindowsInfo::InfoDetails::InfoDetails(PDH_HQUERY query,
                                          const std::wstring &title,
                                          const std::wstring &counterName)
    {

        m_title = WindowsInfo::fromWString(title);
        PdhAddEnglishCounterW(query, counterName.c_str(), 0, &m_counter);
    }
    WindowsInfo::InfoDetails::~InfoDetails()
    {
        if (m_counter != NULL)
        {
            PdhRemoveCounter(m_counter);
            m_counter = NULL;
        }
    }
    std::string WindowsInfo::InfoDetails::title()
    {
        return m_title;
    }
    double WindowsInfo::InfoDetails::value()
    {
        // 获取格式化的计数器值
        PDH_FMT_COUNTERVALUE counterValue; // 格式化的计数器值
        if (PdhGetFormattedCounterValue(
                m_counter, PDH_FMT_DOUBLE, nullptr, &counterValue) ==
            ERROR_SUCCESS)
        {
            m_value = counterValue.doubleValue;
        }
        else
        {
            m_value = 0.0f;
        }
        return m_value;
    }

    WindowsInfo::WindowsInfo()
    {
    }

    WindowsInfo::~WindowsInfo() noexcept
    {
        if (m_query != NULL)
        {
            PdhCloseQuery(m_query);
        }
    }

    bool WindowsInfo::start()
    {
        if (PdhOpenQueryW(NULL, 0, &m_query) != ERROR_SUCCESS)
        {
            qDebug() << "无法打开性能查询";
            return false;
        }
        m_mapCounterName = {
            {TypeOccupation::cpu_total,
             L"\\Processor(_Total)\\% Processor Time"},
            {TypeOccupation::cpu_exe, L"\\Processor({})\\% Processor Time"}};
        return true;
    }

    void WindowsInfo::resetCounter()
    {
        // auto behavior = [this](std::vector<InfoDetails> &vec,
        //                        TypeOccupation type) mutable -> void {
        //     const std::unordered_map<TypeOccupation, std::wstring> mapTotal =
        //     {
        //         {TypeOccupation::cpu_total, L"cpu_total"}};
        //     vec.resize(0);
        //     vec.push_back(
        //         InfoDetails(m_query, mapTotal.at(type),
        //         m_mapCounterName.at(type)));
        // };
        m_counterCPU.resize(0);
        m_counterCPU.emplace_back(
            InfoDetails(m_query,
                        L"cpu_total",
                        m_mapCounterName.at(TypeOccupation::cpu_total)));
        // behavior(m_counterCPU, TypeOccupation::cpu_total);
        // behavior(m_counterDisk, TypeOccupation::disk_total);
        // behavior(m_counterMem, TypeOccupation::memory_total);
        // behavior(m_counterNetRecv, TypeOccupation::network_recv_total);
        // behavior(m_counterNetSend, TypeOccupation::network_send_total);
    }
    void WindowsInfo::primaryCounter()
    {
        m_counterPrimary.emplace_back(InfoDetails(
            m_query, L"cpu_total", L"\\Processor(_Total)\\% Processor Time"));
    }
    void WindowsInfo::generate()
    {
        primaryCounter();
    }

    QVariantMap WindowsInfo::deviceInfo()
    {
        return {};
    }

    QVariantMap WindowsInfo::logicalDriveInfo()
    {
        wchar_t buffer[256] = {0};
        QVariantMap jsValue;
        DWORD length = GetLogicalDriveStringsW(sizeof(buffer), buffer);

        if (length == 0)
        {
            qDebug() << "Failed to get logical drive strings. Error: {}"
                     << GetLastError();
            return {};
        }

        wchar_t *drive = buffer;
        while (*drive)
        {
            ULARGE_INTEGER freeBytesAvailable = {}, totalBytes = {},
                           totalFreeBytes = {};

            if (GetDiskFreeSpaceExW(
                    drive, &freeBytesAvailable, &totalBytes, &totalFreeBytes))
            {
                auto wstrDisk = std::wstring(drive);
                if (wstrDisk.back() == L'\\')
                {
                    wstrDisk.pop_back();
                }
                QVariantMap itemDisk;
                itemDisk["drive"] = fromWString(std::wstring(wstrDisk)).c_str();
                itemDisk["total_size"] =
                    totalBytes.QuadPart / (1024 * 1024 * 1024);
                itemDisk["free_space"] =
                    totalFreeBytes.QuadPart / (1024 * 1024 * 1024);
                jsValue["disk_info"] = itemDisk;
            }
            else
            {
                qDebug() << "  Failed to get space for drive: " << drive
                         << ". Error: " << GetLastError();
                return {};
            }

            drive += wcslen(drive) + 1; // 下一个盘符
        }
        return jsValue;
    }
    std::vector<std::wstring> WindowsInfo::getProcesses()
    {
        std::vector<std::wstring> vecProcess;
        // 创建一个快照，获取所有进程的信息
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnap == INVALID_HANDLE_VALUE)
        {
            LogError("Failed to create snapshot");
            return vecProcess;
        }

        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(hSnap, &pe32))
        {
            do
            {
                vecProcess.push_back(pe32.szExeFile);
            } while (Process32NextW(hSnap, &pe32));
        }
        else
        {
            LogError("Failed to get process information");
        }

        CloseHandle(hSnap);
        return vecProcess;
    }
    std::string WindowsInfo::fromWString(const std::wstring &wstr)
    {
        // 获取转换后的字符所需的缓冲区大小
        int bufferSize = WideCharToMultiByte(
            CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);

        // 分配足够的空间来存储转换后的字符串
        std::string str(bufferSize - 1, '\0'); // 去掉 null 字符
        WideCharToMultiByte(CP_UTF8,
                            0,
                            wstr.c_str(),
                            -1,
                            &str[0],
                            bufferSize,
                            nullptr,
                            nullptr);
        return str;
    }
} // namespace _Kits
