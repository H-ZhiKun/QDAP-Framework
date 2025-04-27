#pragma once
#include "DatabaseConnections.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <memory>

namespace _Kits
{
class DatabaseManager
{
  public:
    ~DatabaseManager() = default;
    static bool start();
    static QSqlDatabase getConnection();
    static void restoreConnection(QSqlDatabase &&db);
    static bool isReady(){return m_dbPools->isReady();};

  protected:
    DatabaseManager() = default;
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;
    DatabaseManager(DatabaseManager &&) = delete;

  private:
    inline static std::unique_ptr<DatabaseConnections> m_dbPools;
    inline static bool m_bSelfCreator = [] {
        DatabaseManager::start();
        return true;
    }();
};

class DBConnectionGuard
{
  public:
    DBConnectionGuard() : m_db(DatabaseManager::getConnection())
    {
    }

    ~DBConnectionGuard()
    {
        DatabaseManager::restoreConnection(std::move(m_db));
    }

    // 获取连接
    QSqlDatabase &get()
    {
        return m_db;
    }

  private:
    QSqlDatabase m_db;
};
} // namespace _Kits
