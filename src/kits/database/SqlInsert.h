#pragma once
#include "SqlQuery.h"
#include "kits/orm/OrmMapperImpl.h"
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <qsqlquery.h>
#include <unordered_map>

namespace _Kits
{

    template <typename T>
    class SqlInsert : public SqlQuery<T>
    {
      public:
        SqlInsert() = default;
        virtual ~SqlInsert() = default;
        virtual bool exec() override
        {
            if (!m_isBatch)
            {
                return execSingle();
            }
            else
            {
                return execBatch();
            }
        }
        int getNumAffected()
        {
            if (!this->m_success)
            {
                qDebug() << "数据库插入失败。";
                return 0;
            }
            return m_batchSize;
        }

        template <typename U, typename std::enable_if<std::is_same<T, U>::value, int>::type = 0>
        SqlInsert &insert(const U &object)
        {
            m_isBatch = false;
            qDebug() << object.created_time;
            auto map = OrmMapper<T>::toMap(object);
            buildSqlStatement(map);
            for (const auto &[key, value] : map)
            {
                qDebug() << value;
                if (key != "id")
                    m_singleValues[":" + key] = std::move(value);
            }
            return *this;
        }

        template <typename Container, typename std::enable_if<!std::is_same<T, Container>::value, int>::type = 0>
        SqlInsert &insert(const Container &objects)
        {
            if (objects.empty())
                return *this;
            m_isBatch = true;
            bool firstUse = true;
            for (const auto &object : objects)
            {
                auto objMap = OrmMapper<T>::toMap(object);
                if (firstUse)
                {
                    firstUse = false;
                    buildSqlStatement(objMap);
                }
                for (const auto &[key, value] : objMap)
                {
                    if (key != "id")
                        m_batchValues[":" + key].append(std::move(value));
                }
            }
            m_batchSize = objects.size();
            return *this;
        }

      protected:
        void buildSqlStatement(const std::unordered_map<std::string, QVariant> &map)
        {
            QStringList fields, placeholders;
            for (const auto &[key, _] : map)
            {
                if (key != "id")
                {
                    fields << QString::fromStdString(key);
                    placeholders << ":" + QString::fromStdString(key);
                }
            }
            // 不再使用 RETURNING id，影响批量性能
            this->m_sql =
                QString("INSERT INTO %1 (%2) VALUES (%3)").arg(T::tableName()).arg(fields.join(", ")).arg(placeholders.join(", "));
        }

        bool execSingle()
        {
            if (!this->m_query.prepare(this->m_sql))
            {
                this->innerError();
                return false;
            }
            for (const auto &[key, value] : m_singleValues)
            {
                this->m_query.bindValue(QString::fromStdString(key), value);
            }
            return this->m_query.exec();
        }

        bool execBatch()
        {
            if (!this->m_query.prepare(this->m_sql))
            {
                this->innerError();
                return false;
            }
            for (const auto &[key, values] : m_batchValues)
            {
                this->m_query.bindValue(QString::fromStdString(key), values);
            }

            auto &db = this->m_dbGuard.get();

            db.transaction(); // 开始事务
            this->m_success = this->m_query.execBatch();
            if (!this->m_success)
            {
                this->innerError();
                db.rollback(); // 失败回滚
                return false;
            }
            this->m_success = db.commit();
            return this->m_success; // 成功提交
        }

      private:
        bool m_isBatch = false;
        int m_batchSize = 0;
        std::unordered_map<std::string, QVariant> m_singleValues;
        std::unordered_map<std::string, QVariantList> m_batchValues;
    };

} // namespace _Kits
