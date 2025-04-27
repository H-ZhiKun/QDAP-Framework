#include "MysqlConnections.h"
#include <QFile>
namespace _Kits
{
MysqlConnections::MysqlConnections(const QString &host,
                                   quint16 port,
                                   const QString &dbName,
                                   const QString &user,
                                   const QString &password,
                                   int maxConnections)
    : DatabaseConnections(host, port, dbName, user, password, maxConnections)
{
}
bool MysqlConnections::checkAndCreateDatabase()
{
    QString name = QString("mysql_connection_init");
    auto db = std::make_unique<QSqlDatabase>(
        QSqlDatabase::addDatabase("QMYSQL", name));

    // 使用管理员权限连接 MySQL
    db->setHostName(host_);
    db->setPort(port_);
    db->setUserName(user_);
    db->setPassword(password_);

    if (!db->open())
    {
        qDebug() << "Failed to connect to MySQL:" << db->lastError().text();
        return false;
    }

    QSqlQuery query(*db);
    query.prepare("SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE "
                  "SCHEMA_NAME = :dbName");
    query.bindValue(":dbName", dbName_);

    if (!query.exec())
    {
        qDebug() << "Failed to query database existence:"
                 << query.lastError().text();
        return false;
    }

    if (!query.next())
    {
        if (!query.exec(QString("CREATE DATABASE `%1`").arg(dbName_)))
        {
            qDebug() << "Failed to create database:"
                     << query.lastError().text();
            return false;
        }
        qDebug() << "Database created successfully:" << dbName_;
    }
    else
    {
        qDebug() << "Database already exists:" << dbName_;
    }

    db->close();
    QSqlDatabase::removeDatabase(name);
    return true;
}

bool MysqlConnections::initializeDatabaseSchema()
{
    auto db = createConnection();
    if (!db.isOpen() || !db.isValid())
    {
        qDebug() << "Failed to connect to target database for initialization.";
        return false;
    }

    QFile sqlFile("./config/mysqlinit.sql");
    if (!sqlFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open SQL file:" << sqlFile.errorString();
        return false;
    }

    QTextStream stream(&sqlFile);
    QString script = stream.readAll();
    sqlFile.close();

    if (script.isEmpty())
    {
        qDebug() << "SQL script is empty, nothing to execute.";
        return false;
    }

    QStringList statements = script.split(";", Qt::SkipEmptyParts);
    for (const QString &statement : statements)
    {
        if (statement.trimmed().isEmpty())
            continue;

        QSqlQuery initQuery(db);
        if (!initQuery.exec(statement.trimmed()))
        {
            qDebug() << "Failed to execute statement:" << statement.trimmed()
                     << "Error:" << initQuery.lastError().text();
            return false;
        }
    }

    qDebug() << "Database initialized with SQL script successfully.";
    return true;
}

bool MysqlConnections::initializeConnectionPool()
{
    for (size_t i = 0; i < 5; i++)
    {
        auto connection = createConnection();
        if (!connection.isOpen() || !connection.isValid())
        {
            qDebug() << "Failed to create database connection for pool.";
            return false;
        }
        connectionPool_.push_back(std::move(connection));
    }

    qDebug() << "Database connection pool initialized successfully.";
    return true;
}

QSqlDatabase MysqlConnections::createConnection()
{

    QString name = QString("mysql_connection_%1").arg(count_++);
    auto db = QSqlDatabase::addDatabase("QMYSQL", name);
    db.setHostName(host_);
    db.setPort(port_);
    db.setDatabaseName(dbName_);
    db.setUserName(user_);
    db.setPassword(password_);
    if (!db.open())
    {
        QSqlError error = db.lastError();
        QSqlDatabase::removeDatabase(name);
        qDebug() << "create sql connection error, " << error;
        return {};
    }
    qDebug() << "create sql connection size=" << count_;
    return db;
}
} // namespace _Kits