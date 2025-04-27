#pragma once
#include "kits/required/module_base/ModuleBase.h"
#include <qvariant.h>
#include <string_view>
#include <utility>

namespace _Service
{
    struct NotifyType
    {
        static constexpr std::string_view Task = "Task";
        static constexpr std::string_view UniqueTime = "UniqueTime";
    };

    class AppFramework
    {

      public:
        virtual ~AppFramework() noexcept = default;
        /**
         * @brief 获取应用程序框架的唯一实例
         * @details
         * 该函数返回应用程序框架的唯一实例，是单例模式的实现。该函数不接收任何参数。
         * @return
         * 返回AppFramework类型的引用，表示应用程序框架的唯一实例。
         */
        [[nodiscard]] static AppFramework &instance() noexcept;
        virtual int run(int argc, char *argv[]) = 0;
        virtual void stop() noexcept = 0;
        template <typename Ret, typename... Args>
        Ret invokeModuleSync(const std::string &funcRoute, Args &&...args);
        template <typename... Args>
        bool invokeModuleAsync(const std::string &funcRoute, Args &&...args);
        /**
         * @brief 通知所有模块
         *
         * 该函数用于向所有已注册的模块发送通知。每个模块需要实现相应的槽函数来处理该通知。
         *
         * 示例：
         * 如果通知类型为 NotifyType::Task，则子类应当实现：
         * @code
         * private slots: void notifyTask(const QVariant &);
         * @endcode
         *
         * @param type 通知的类型
         * @param value 传递的参数数据
         */
        virtual void notify(const std::string_view &type, const QVariant &) noexcept = 0;

      protected:
        AppFramework() = default;
        AppFramework(const AppFramework &) noexcept = delete;
        AppFramework &operator=(const AppFramework &) = delete;
        AppFramework(AppFramework &&) = delete;
        AppFramework &operator=(AppFramework &&) = delete;
        std::unordered_map<std::string,
                           std::unique_ptr<_Kits::ModuleBase>> m_plugins; // 使用类型索引存储模块
    };

    template <typename Ret, typename... Args>
    inline Ret AppFramework::invokeModuleSync(const std::string &funcRoute, Args &&...args)
    {
        auto pos = funcRoute.find("::");
        if (pos == std::string::npos)
        {
            return Ret{};
        }
        std::string className = funcRoute.substr(0, pos);
        std::string route = funcRoute.substr(pos + 2);
        auto modFinder = m_plugins.find(className);
        if (modFinder == m_plugins.end())
        {
            return Ret{};
        }
        auto &module = modFinder->second;

        return module->invokeSync<Ret>(route, std::forward<Args>(args)...);
    }
    template <typename... Args>
    bool AppFramework::invokeModuleAsync(const std::string &funcRoute, Args &&...args)
    {
        auto pos = funcRoute.find("::");
        if (pos == std::string::npos)
        {
            return false;
        }
        std::string className = funcRoute.substr(0, pos);
        std::string route = funcRoute.substr(pos + 2);
        auto modFinder = m_plugins.find(className);
        if (modFinder == m_plugins.end())
        {
            return false;
        }
        auto &module = modFinder->second;
        return module->invokeAsync(route, std::forward<Args>(args)...);
    }
    [[nodiscard]] inline AppFramework &App() noexcept
    {
        return AppFramework::instance();
    }
} // namespace _Service
