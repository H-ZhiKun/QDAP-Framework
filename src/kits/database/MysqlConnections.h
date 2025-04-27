#pragma once
#include "DatabaseConnections.h"
#include <QSqlDatabase>
namespace _Kits
{
class MysqlConnections : public DatabaseConnections
{
    Q_OBJECT
  public:
    explicit MysqlConnections(const QString &host,
                              quint16 port,
                              const QString &dbName,
                              const QString &user,
                              const QString &password,
                              int maxConnections = 5);
    virtual ~MysqlConnections() = default;

  public:
    virtual bool checkAndCreateDatabase() override;
    virtual bool initializeDatabaseSchema() override;
    virtual bool initializeConnectionPool() override;

  protected:
    virtual QSqlDatabase createConnection() override;
};
} // namespace _Kits