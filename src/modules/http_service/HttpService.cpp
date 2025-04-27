#include "HttpService.h"
#include "kits/required/module_base/ModuleBase.h"
#include <QtConcurrent/QtConcurrent>
#include <json/json.h>
#include <qhttpserver.h>
#include <qhttpserverresponse.h>
#include <qlogging.h>
#include <qtcpserver.h>
#include <qtypes.h>

namespace _Modules
{
    HttpService::HttpService(QObject *parent) : _Kits::ModuleBase(parent)
    {
    }
    HttpService::~HttpService() noexcept
    {
    }

    bool HttpService::start(const YAML::Node &config)
    {
        m_port = config["port"].as<quint16>();
        m_http = new QHttpServer(this);
        m_tcpServer = new QTcpServer(this);

        if (!m_tcpServer->listen(QHostAddress::Any, m_port) ||
            !m_http->bind(m_tcpServer))
        {

            return false;
        }
        qDebug() << "Listening on port" << m_tcpServer->serverPort();
        return true;
    }
    bool HttpService::stop()
    {
        return true;
    }

    bool HttpService::customization(
        const std::unordered_map<_Kits::RegisterKey, std::list<std::any>>
            &lvTasks)
    {
        for (const auto &[key, funcs] : lvTasks)
        {
            auto path = std::get<0>(key); // 根据实际情况获取路径
            for (const auto &func : funcs)
            {
                try
                {
                    auto handler =
                        std::any_cast<std::function<QHttpServerResponse(
                            const QHttpServerRequest &)>>(func);
                    // 绑定路由
                    m_http->route(
                        path.c_str(),
                        [handler = std::move(handler)](
                            const QHttpServerRequest &request) {
                            return QtConcurrent::run(
                                [handler = std::move(handler), &request]() {
                                    return handler(request);
                                });
                        });
                }
                catch (const std::bad_any_cast &)
                {
                    // 处理转换失败的情况
                    qDebug() << "Invalid handler type for path:" << path;
                    return false;
                }
            }
        }

        return true;
    }
} // namespace _Modules
