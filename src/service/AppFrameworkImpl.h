#pragma once
#include "AppFramework.h"
#include <qglobal.h>
#include <qmetaobject.h>
#include <qobject.h>
#include <string_view>
#include <yaml-cpp/yaml.h>
namespace _Kits
{
    class ModuleBase;
}
namespace _Service
{
    class AppFrameworkImpl final : public AppFramework
    {
      public:
        [[nodiscard]] static AppFrameworkImpl &instance() noexcept;
        virtual ~AppFrameworkImpl() noexcept;

        virtual int run(int argc, char *argv[]) override;
        virtual void stop() noexcept override;
        virtual void notify(const std::string_view &type, const QVariant &) noexcept override;

      protected:
        AppFrameworkImpl() noexcept = default;
        // 子类化接口区
        YAML::Node loadConfig();
        void cmdCheck();
        void createModules(const YAML::Node &yaml);
        void connectModules();
        void isValidModule(const std::string &name);

      private:
        std::unordered_map<std::string,
                           std::vector<_Kits::ModuleBase *>> m_notifyModules; // 使用类型索引存储模块
    };

} // namespace _Service