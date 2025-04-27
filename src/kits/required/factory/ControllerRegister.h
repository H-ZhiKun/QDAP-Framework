#pragma once
#include <any>
#include <qvariant>
#include <string>
#include <unordered_map>
#include <yaml-cpp/yaml.h>

namespace _Kits
{
    using RegisterKey = std::variant<std::string, int>;
    // 定义工厂类，支持注册和创建模块
    class ControllerRegister
    {
      public:
        template <typename HttpHandler>
        static void registerHttpPath(const std::string &path, HttpHandler &&handler)
        {
            registerRoute("HttpService", path, std::forward<HttpHandler>(handler));
        }

        template <typename TaskHandler>
        static void registerTaskRoutes(const std::string &path, TaskHandler &&pkg)
        {
            auto pos = path.find("::");
            if (pos == std::string::npos)
            {
                throw std::invalid_argument("Invalid path format. Expected 'ClassName::FunctionName'");
            }
            registerRoute(path.substr(0, pos), path.substr(pos + 2), std::forward<TaskHandler>(pkg));
        }
        template <typename QmlHandler>
        static void registerQmlPath(int qmlPath, QmlHandler &&func)
        {
            registerRoute("QmlPrivateEngine", qmlPath, std::forward<QmlHandler>(func));
        }

        static std::unordered_map<RegisterKey, std::list<std::any>> &getKeyRoutes(const std::string &className)
        {
            if (getRoutes().find(className) == getRoutes().end())
            {
                static std::unordered_map<RegisterKey, std::list<std::any>> other;
                return other;
            }
            return getRoutes().at(className);
        }

      private:
        static void registerRoute(const std::string &className, const RegisterKey &funcName, std::any &&func)
        {
            getRoutes()[className][funcName].push_back(std::move(func));
        }

        static std::unordered_map<std::string, std::unordered_map<RegisterKey, std::list<std::any>>> &getRoutes()
        {
            static auto routes = std::unordered_map<std::string, std::unordered_map<RegisterKey, std::list<std::any>>>();
            return routes;
        }
    };

} // namespace _Kits
