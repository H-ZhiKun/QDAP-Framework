#pragma once
#include "DatabaseManager.h"
#include <QSqlQuery>

namespace _Kits
{

template <typename T>
class SqlQuery
{
  public:
    SqlQuery() : m_query(m_dbGuard.get()) {};
    virtual ~SqlQuery() {};
    virtual bool exec() = 0;

  protected:
    void innerError()
    {
        qDebug() << "SQL执行失败: " << m_sql << m_query.lastError().text();
    }
    QString m_sql;
    bool m_success = false;
    DBConnectionGuard m_dbGuard;
    QSqlQuery m_query;
};

} // namespace _Kits
