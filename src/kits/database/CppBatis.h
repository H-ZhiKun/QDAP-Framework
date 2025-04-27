#pragma once
#include "SqlInsert.h"
#include "SqlSelect.h"
#include "SqlTypes.h"
#include "kits/orm/TableStructs.h"
#include <qcontainerfwd.h>


namespace _Kits
{
class CppBatis
{
  public:
    QVariantList execSql(const QString &sql)
    {
        DBConnectionGuard guard;
        QSqlQuery query(guard.get());

        if (!query.prepare(sql))
        {
            qDebug() << "SQL准备失败: " << query.lastError().text();
            qDebug() << sql;
            return {};
        }

        if (!query.exec())
        {
            qDebug() << "SQL执行失败: " << query.lastError().text();
            return {};
        }

        QVariantList resultList;

        // 检查是否有结果集（即是否为SELECT查询）
        if (query.isSelect())
        {
            while (query.next())
            {
                QVariantMap recordMap;
                QSqlRecord record = query.record();
                for (int i = 0; i < record.count(); ++i)
                {
                    recordMap.insert(record.fieldName(i), record.value(i));
                }
                resultList.append(recordMap);
            }
        }
        else
        {
            // 非查询操作，返回受影响的行数
            QVariantMap result;
            result.insert("rowsAffected", query.numRowsAffected());
            resultList.append(result);
        }

        return resultList;
    }
};
} // namespace _Kits
