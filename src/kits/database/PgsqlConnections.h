#pragma once
#include "DatabaseConnections.h"
#include <QSqlDatabase>
namespace _Kits
{
class PgsqlConnections : public DatabaseConnections
{
    Q_OBJECT
  public:
    explicit PgsqlConnections(const QString &host,
                              quint16 port,
                              const QString &dbName,
                              const QString &user,
                              const QString &password,
                              int maxConnections = 5);
    virtual ~PgsqlConnections() = default;

  public:
    virtual bool checkAndCreateDatabase() override;
    virtual bool initializeDatabaseSchema() override;
    virtual bool initializeConnectionPool() override;

  protected:
    virtual QSqlDatabase createConnection() override;
};
} // namespace _Kits