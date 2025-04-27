#include "DatabaseManager.h"
#include "MysqlConnections.h"
#include "PgsqlConnections.h"
#include <filesystem>
#include <memory>
#include <utility>
#include <yaml-cpp/yaml.h>
namespace _Kits
{
bool DatabaseManager::start()
{
    std::string filePath =
        std::filesystem::current_path().string() + "/config/config.yaml";
    auto config = YAML::LoadFile(filePath);

    if (!config["database"])
    {
        qDebug() << "Invalid or missing database configuration.";
        return false;
    }
    const auto &item = config["database"];

    if (!item["rdbms"] || !item["host"] || !item["port"] || !item["db_name"] ||
        !item["user"])
    {
        qDebug() << "Incomplete database configuration."
                 << item["rdbms"].as<std::string>();
        return false;
    }

    QString rdbms = QString::fromStdString(item["rdbms"].as<std::string>());
    QString host = QString::fromStdString(item["host"].as<std::string>());
    quint16 port = static_cast<quint16>(item["port"].as<int>());
    QString dbName = QString::fromStdString(item["db_name"].as<std::string>());
    QString user = QString::fromStdString(item["user"].as<std::string>());
    QString password;
    if (rdbms == "postgresql")
    {
        password = QString("~postgres@");
        m_dbPools = std::make_unique<PgsqlConnections>(
            host, port, dbName, user, password);
    }
    else if (rdbms == "mysql")
    {
        password = QString("123456");
        m_dbPools = std::make_unique<MysqlConnections>(
            host, port, dbName, user, password);
    }
    else
    {
        qDebug() << "Unsupported RDBMS type:" << rdbms;
        return false;
    }

    if (m_dbPools == nullptr)
    {
        qDebug() << "error: database can not load; " << rdbms << "," << dbName;
        return false;
    }
    m_dbPools->init();
    return true;
}
QSqlDatabase DatabaseManager::getConnection()
{
    return m_dbPools->getConnection();
}
void DatabaseManager::restoreConnection(QSqlDatabase &&db)
{
    return m_dbPools->restoreConnection(std::move(db));
}
} // namespace _Kits
