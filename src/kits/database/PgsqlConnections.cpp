#include "PgsqlConnections.h"
#include <QFile>
#include <QRegularExpression>

namespace _Kits
{
PgsqlConnections::PgsqlConnections(const QString &host,
                                   quint16 port,
                                   const QString &dbName,
                                   const QString &user,
                                   const QString &password,
                                   int maxConnections)
    : DatabaseConnections(host, port, dbName, user, password, maxConnections)
{
}

bool PgsqlConnections::checkAndCreateDatabase()
{
    QString name = QString("pgsql_connection_init");
    {
        auto db = QSqlDatabase::addDatabase("QPSQL", name);
        // 临时使用管理员权限连接 PostgreSQL
        db.setHostName(host_);
        db.setDatabaseName("postgres");
        db.setPort(port_);
        db.setUserName("postgres");
        db.setPassword("~postgres@");

        if (!db.open())
        {
            qDebug() << "Failed to connect to PostgreSQL:"
                     << db.lastError().text();
            return false;
        }

        QSqlQuery query(db);
        query.prepare("SELECT 1 FROM pg_database WHERE datname = :dbName");
        query.bindValue(":dbName", dbName_);

        if (!query.exec())
        {
            qDebug() << "Failed to query database existence:"
                     << query.lastError().text();
            return false;
        }

        if (!query.next())
        {
            // 数据库不存在，创建它
            if (!query.exec(QString("CREATE DATABASE \"%1\"").arg(dbName_)))
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
    }
    QSqlDatabase::removeDatabase(name);
    return true;
}

bool PgsqlConnections::initializeDatabaseSchema()
{
    // 创建数据库连接
    auto db = createConnection();
    if (!db.isOpen() || !db.isValid())
    {
        qDebug() << "Failed to connect to target database for initialization.";
        return false;
    }

    // 打开 SQL 脚本文件
    QFile sqlFile("./config/pginit.sql");
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

    // 使用正则表达式移除注释部分（单行和多行注释）
    QRegularExpression commentRegex(
        R"((--[^\n]*|/\*.*?\*/))",
        QRegularExpression::DotMatchesEverythingOption);
    script.remove(commentRegex);

    // 按分号分割 SQL 语句
    QStringList statements = script.split(";", Qt::SkipEmptyParts);

    for (const QString &statement : statements)
    {
        QString trimmedStatement = statement.trimmed();
        if (trimmedStatement.isEmpty())
            continue;

        // 执行每条 SQL 语句
        QSqlQuery initQuery(db);
        if (!initQuery.exec(trimmedStatement))
        {
            qDebug() << "Failed to execute statement:" << trimmedStatement
                     << "Error:" << initQuery.lastError().text();
        }
    }

    qDebug() << "Database initialized with SQL script successfully.";
    connectionPool_.push_back(std::move(db));
    return true;
}

// 初始化数据库连接池
bool PgsqlConnections::initializeConnectionPool()
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
QSqlDatabase PgsqlConnections::createConnection()
{

    QString name = QString("pgsql_connection_%1").arg(count_++);

    auto db = QSqlDatabase::addDatabase("QPSQL", name);
    m_poolNames.push_back(name);
    db.setHostName(host_);
    db.setPort(port_);
    db.setDatabaseName(dbName_);
    db.setUserName(user_);
    db.setPassword(password_);
    if (!db.open() || !db.isValid())
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