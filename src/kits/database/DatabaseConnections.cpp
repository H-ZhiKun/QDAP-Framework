#include "DatabaseConnections.h"
#include <QCoreApplication>
#include <chrono>
#include <qobject.h>
#include <qtmetamacros.h>
#include <thread>

namespace _Kits
{

    DatabaseConnections::DatabaseConnections(const QString &host,
                                             quint16 port,
                                             QString dbName,
                                             const QString &user,
                                             const QString &password,
                                             int maxConnections,
                                             QObject *parent)
        : QObject(parent), host_(host), port_(port), dbName_(dbName),
          user_(user), password_(password), maxConnections_(maxConnections),
          m_bInit(false)
    {
    }

    DatabaseConnections::~DatabaseConnections()
    {
        m_bInit = false;
        if (m_thInit.joinable())
            m_thInit.join();
    }
    bool DatabaseConnections::init()
    {
        QObject::connect(this, &DatabaseConnections::already, [this]() {
            if (checkAndCreateDatabase() && initializeDatabaseSchema() &&
                initializeConnectionPool())
            {
                m_bInit = true;
            }
        });
        m_thInit = std::thread([this] {
            while (true)
            {
                if (QCoreApplication::instance())
                {
                    emit already();
                    return;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        });
        return true;
    }

    QSqlDatabase DatabaseConnections::getConnection()
    {
        if (!m_bInit)
        {
            return {};
        }
        std::lock_guard locker(mutex_);
        if (!connectionPool_.empty())
        {
            auto db = std::move(connectionPool_.front());
            connectionPool_.pop_front();
            return db;
        }
        else
        {
            return createConnection();
        }
    }

    void DatabaseConnections::restoreConnection(QSqlDatabase &&db)
    {
        std::lock_guard locker(mutex_);
        if (db.isOpen() && db.isValid())
            connectionPool_.push_back(std::move(db));
    }

} // namespace _Kits