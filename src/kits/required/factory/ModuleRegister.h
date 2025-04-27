#pragma once
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace _Kits
{
    // 定义工厂类，支持注册和创建模块
    class ModuleRegister
    {
      public:
        template <typename T>
        static void registerCreator(const std::string &className)
        {
            std::string name = className;
            if (className.find(":") != std::string::npos)
            {
                name = className.substr(className.find_last_of(":") + 1);
            }
            getCreators()[name] = []() -> void * {
                return new T(); // 只创建对象，不转换
            };
        }
        template <typename T>
        static std::unique_ptr<T> createModule(const std::string &className)
        {
            auto &creators = getCreators();
            auto it = creators.find(className);
            if (it != creators.end())
            {
                void *obj = it->second(); // 先创建对象，得到 void*
                return std::unique_ptr<T>(
                    static_cast<T *>(obj)); // 包装成 unique_ptr<T>
            }
            return nullptr;
        }
        static void registerGlobalConnect(const std::string &signalName,
                                          const std::string &slotName)
        {
            getGlobalConnections()[signalName].push_back(slotName);
        }

        static std::unordered_map<std::string, std::list<std::string>> &
        getGlobalConnections()
        {
            static auto globalConnections =
                std::unordered_map<std::string, std::list<std::string>>();
            return globalConnections;
        }

      private:
        static std::unordered_map<std::string, std::function<void *()>> &
        getCreators()
        {
            static auto creators =
                std::unordered_map<std::string, std::function<void *()>>();
            return creators;
        }
    };

} // namespace _Kits
// 注册模块到工厂
#define DECLARE_MODULE(ModuleType)                                             \
  private:                                                                     \
    inline static const bool registered_ = []() {                              \
        _Kits::ModuleRegister::registerCreator<ModuleType>(#ModuleType);       \
        return true;                                                           \
    }();

// 与QObject::connect功能和性能保持一致，通过TIS_ClassInfo查找信号与槽进行连接。
#define TIS_CONNECT(signalName, slotName)                                      \
  private:                                                                     \
    inline static const bool registerConnection_ = []() {                      \
        _Kits::ModuleRegister::registerGlobalConnect(signalName, slotName);    \
        return true;                                                           \
    }();