#pragma once
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <deque>
#include <mutex>
#include <qobject.h>
#include <qtmetamacros.h>
#include <thread>
namespace _Kits
{

class DatabaseConnections : public QObject
{
    Q_OBJECT
  public:
    explicit DatabaseConnections(const QString &host,
                                 quint16 port,
                                 QString dbName,
                                 const QString &user,
                                 const QString &password,
                                 int maxConnections = 20,
                                 QObject *parent = nullptr);
    virtual ~DatabaseConnections();
    bool init();
    QSqlDatabase getConnection();
    void restoreConnection(QSqlDatabase &&db);
    bool isReady(){return m_bInit;};
  signals:
    void already();

  protected:
    virtual bool checkAndCreateDatabase() = 0;
    virtual bool initializeDatabaseSchema() = 0;
    virtual bool initializeConnectionPool() = 0;
    virtual QSqlDatabase createConnection() = 0;
    std::vector<QString> m_poolNames;
    std::deque<QSqlDatabase> connectionPool_;
    std::vector<QString> dbNameList_;
    std::mutex mutex_;
    int maxConnections_;
    int count_ = 0;
    int idleTimeout_;
    QString host_;
    quint16 port_;
    QString dbName_;
    QString user_;
    QString password_;
    bool m_bInit;
    std::thread m_thInit;
};
} // namespace _Kits