#pragma once
#include "SqlQuery.h"
#include "SqlTypes.h"
#include "kits/orm/OrmMapperImpl.h"
#include <QDebug>
#include <vector>

namespace _Kits
{

    template <typename T>
    class SqlSelect : public SqlQuery<T>
    {
      public:
        SqlSelect() = default;
        ~SqlSelect() = default;

        // 设置要查询的字段列表
        SqlSelect<T> &select(const QStringList &fields)
        {
            m_fields = fields;
            return *this;
        }

        // 添加查询条件
        SqlSelect<T> &where(const QString &field, OperatorComparison op, const QVariant &value, OperatorLogical logicOperator = OperatorLogical::And)
        {
            if (m_conditions.isEmpty())
            {
                m_conditions.append(Condition(field, op, value));
            }
            else
            {
                m_conditions.append(Condition(field, op, value, logicOperator));
            }
            return *this;
        }

        // 添加排序规则
        SqlSelect<T> &orderBy(const QString &field, bool ascending = true)
        {
            m_orderByClauses.append(QString("%1 %2").arg(field, ascending ? "ASC" : "DESC"));
            return *this;
        }

        // 去重
        SqlSelect<T> &distinct(bool enable = true)
        {
            m_distinct = enable;
            return *this;
        }

        // 设置分页参数
        SqlSelect<T> &paginate(int page, int pageSize)
        {
            m_page = page;
            m_pageSize = pageSize;
            return *this;
        }

        // 执行查询并获取结果集
        virtual bool exec() override
        {
            this->m_sql = buildSelectStatement();
            if (!this->m_query.prepare(this->m_sql))
            {
                this->innerError();
                return false;
            }

            for (const auto &condition : m_conditions)
            {
                this->m_query.bindValue(":" + condition.field, condition.value);
            }

            if (!this->m_query.exec())
            {
                this->innerError();
                return false;
            }
            this->m_success = true;
            return true;
        }
        std::vector<T> getResults()
        {
            if (!this->m_success)
                return {};
            std::vector<T> results;
            while (this->m_query.next())
            {
                auto record = this->m_query.record();
                auto obj = OrmMapper<T>::fromRecord(record);
                results.push_back(std::move(obj));
            }
            return results;
        }

      private:
        struct Condition
        {
            QString field;
            OperatorComparison op;
            QVariant value;
            OperatorLogical logicOperator;

            Condition(const QString &f, OperatorComparison o, const QVariant &v, OperatorLogical lo = OperatorLogical::And) : field(f), op(o), value(v), logicOperator(lo)
            {
            }
        };

        QString buildSelectStatement() const
        {
            QString fields = m_fields.isEmpty() ? "*" : m_fields.join(", ");
            QString distinctClause = m_distinct ? "DISTINCT " : "";
            QString sql = QString("SELECT %1%2 FROM %3").arg(distinctClause, fields, T::tableName());

            QString whereClause = buildWhereClause();
            if (!whereClause.isEmpty())
            {
                sql.append(" WHERE ").append(whereClause);
            }
            if (!m_orderByClauses.isEmpty())
            {
                sql.append(" ORDER BY ").append(m_orderByClauses.join(", "));
            }
            if (m_page > 0 && m_pageSize > 0)
            {
                int offset = (m_page - 1) * m_pageSize;
                sql.append(QString(" LIMIT %1 OFFSET %2").arg(m_pageSize).arg(offset));
            }

            sql.append(";");
            return sql;
        }

        QString buildWhereClause() const
        {
            QStringList clauses;
            for (const auto &condition : m_conditions)
            {
                QString clause = QString("%1 %2 :%1").arg(condition.field, SqlOperators::comparisonOperatorMap.at(condition.op));
                if (!clauses.isEmpty())
                {
                    clause = SqlOperators::logicalOperatorMap.at(condition.logicOperator) + " " + clause;
                }
                clauses.append(clause);
            }
            return clauses.join(' ');
        }

        QStringList m_fields;
        QList<Condition> m_conditions;
        QStringList m_orderByClauses;
        int m_page = 0;
        int m_pageSize = 0;
        bool m_distinct = false; // 是否去重
    };

} // namespace _Kits
