#include "AppFrameworkImpl.h"
#include "kits/required/factory/ControllerRegister.h"
#include "kits/required/factory/ModuleRegister.h"
#include "kits/required/log/CRossLogger.h"
#include "kits/required/module_base/ModuleBase.h"
#include "kits/required/thread_pool/ConcurrentPool.h"
#include <QApplication>
#include <QIcon>
#include <QMetaMethod>
#include <exception>
#include <fmt/format.h>
#include <qlogging.h>
#include <vector>


using namespace _Service;
using namespace _Kits;
AppFramework &AppFramework::instance() noexcept
{
    return AppFrameworkImpl::instance();
}
inline AppFrameworkImpl &AppFrameworkImpl::instance() noexcept
{
    static AppFrameworkImpl instance;
    return instance;
}
AppFrameworkImpl::~AppFrameworkImpl() noexcept
{
}
int AppFrameworkImpl::run(int argc, char *argv[])
{
    QApplication qtCore(argc, argv);
    qtCore.setWindowIcon(QIcon(":/infinity_station/res/icon/gw.ico")); // 设置标题栏图标
    auto config = loadConfig();
    createModules(config);
    cmdCheck();

    int ret = qtCore.exec();
    stop();
    return ret;
}
YAML::Node AppFrameworkImpl::loadConfig()
{
    YAML::Node config;
    try
    {
        std::string filePath = QCoreApplication::applicationDirPath().toStdString() + "/config/config.yaml";
        config = YAML::LoadFile(filePath);
    }
    catch (const YAML::Exception &e)
    {
        qDebug() << "Error: Loaded YAML is null (invalid or empty file).";
        std::terminate();
    }
    return config;
}
void AppFrameworkImpl::stop() noexcept
{
    for (const auto &it : m_plugins)
    {
        it.second->stop();
    }
}

void AppFrameworkImpl::createModules(const YAML::Node &yaml)
{
    std::string savePath;
    for (const auto &item : yaml)
    {
        auto className = item.first.as<std::string>();
        auto block = item.second;
        if (className == "app")
        {
            savePath = block["save_path"].as<std::string>();
        }
        isValidModule(className);

        auto module = _Kits::ModuleRegister::createModule<_Kits::ModuleBase>(className);
        if (module)
        {
            module->tidyMetaMethod();

            module->customization(_Kits::ControllerRegister::getKeyRoutes(className));
            module->start(block);
            m_plugins.emplace(className, std::move(module));
            _Kits::LogInfo("module create success: {}", className);
        }
    }
    connectModules();
}

void AppFrameworkImpl::connectModules()
{
    auto &connectMaps = _Kits::ModuleRegister::getGlobalConnections();
    const std::string separator("::");
    for (const auto &[primaryKey, primaryValues] : connectMaps)
    {
        auto signalPos = primaryKey.find(separator);
        auto signalModuleName = primaryKey.substr(0, signalPos);
        auto signalName = primaryKey.substr(signalPos + 2);
        if (m_plugins.find(signalModuleName) == m_plugins.end())
            continue;
        auto *signalModule = m_plugins.at(signalModuleName).get();
        for (const auto &slotValue : primaryValues)
        {
            auto slotPos = slotValue.find(separator);
            auto slotModuleName = slotValue.substr(0, slotPos);
            auto slotName = slotValue.substr(slotPos + 2);
            if (m_plugins.find(slotModuleName) == m_plugins.end())
                continue;
            auto *slotModule = m_plugins.at(slotModuleName).get();
            auto *signalModule = m_plugins.at(signalModuleName).get();
            ModuleBase::tisMetaConnect(signalModule, signalName, slotModule, slotName);
        }
    }
}

void AppFrameworkImpl::cmdCheck()
{
    auto findModule = m_plugins.find("QmlPrivateEngine");
    if (findModule == m_plugins.end())
    {
        LogError("QmlPrivateEngine module not find, in connectQml.");
        threadPool().runEvery(
            "check input quit",
            []() {
                QTextStream input(stdin);
                if (input.atEnd())
                {
                    return; // 如果没有输入，继续等待
                }
                QString command = input.readLine().trimmed();
                if (command == "quit")
                {
                    qDebug() << "Received quit command. Exiting...";
                    QCoreApplication::quit();
                }
            },
            1000);
        return;
    }
}

void AppFrameworkImpl::notify(const std::string_view &type, const QVariant &data) noexcept
{
    auto notifyType = std::string(type);
    std::string notifyFunc = std::string("notify") + std::string(type);
    auto finder = m_notifyModules.find(notifyType);
    if (finder == m_notifyModules.end())
    {
        std::vector<_Kits::ModuleBase *> vecModules;
        for (const auto &[_, module] : m_plugins)
        {
            if (module->invokeAsync(notifyFunc, data))
            {
                vecModules.push_back(module.get());
            }
        }
        if (vecModules.size())
        {
            m_notifyModules.emplace(notifyType, std::move(vecModules));
        }
        return;
    }
    auto modules = finder->second;
    for (const auto &mod : modules)
    {
        mod->invokeAsync(notifyFunc, data);
    }
}

void AppFrameworkImpl::isValidModule(const std::string &name)
{
    auto findModule = m_plugins.find(name);
    if (findModule != m_plugins.end())
    {
        throw std::runtime_error(fmt::format("<{}> module already existed.", name));
    }
}