#pragma once
#include "kits/required/module_base/ModuleBase.h"
#include <QHttpserver>
#include <qtcpserver.h>
#include <qtypes.h>

namespace _Modules
{

    class HttpService : public _Kits::ModuleBase
    {
        Q_OBJECT
        DECLARE_MODULE(HttpService)
      public:
        explicit HttpService(QObject *parent = nullptr);
        virtual ~HttpService() noexcept;

        virtual bool start(const YAML::Node &config) override;
        virtual bool stop() override;
        virtual bool customization(
            const std::unordered_map<_Kits::RegisterKey, std::list<std::any>>
                &lvTasks) override;

      private:
        QHttpServer *m_http = nullptr;
        QTcpServer *m_tcpServer = nullptr;
        quint16 m_port = 0;
    };

} // namespace _Modules
