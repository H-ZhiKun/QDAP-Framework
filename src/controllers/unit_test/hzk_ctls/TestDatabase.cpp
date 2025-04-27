#include "TestDatabase.h"
#include "kits/database/CppBatis.h"
#include "kits/orm/TableStructs.h"
#include "kits/required/log/CRossLogger.h"
#include <json/value.h>
#include <qdebug.h>

namespace _Controllers
{
    using namespace _Kits;
    void TestDatabase::testCURD(const QVariant &)
    {
        // insert();
        select();
    }
    void TestDatabase::insert()
    {
        // 插入
        std::vector<radar_data> lvObj;
        int batchSize = 10;
        lvObj.reserve(batchSize);
        for (int i = 0; i < batchSize; i++)
        {
            radar_data data;
            Json::Value jsRoot;
            Json::Value jsdata;
            jsdata["x"] = i;
            jsdata["y"] = i;
            jsRoot["points"].append(jsdata);
            data.location_id = i;
            data.points = jsRoot.toStyledString().c_str();
            lvObj.emplace_back(std::move(data));
        }
        _Kits::LogDebug("sql insert begin");
        _Kits::SqlInsert<radar_data> insert;
        insert.insert(lvObj).exec();
        int nums = insert.getNumAffected();
        _Kits::LogDebug("sql insert end {}", nums);
    }
    void TestDatabase::select()
    {

        // 获取第1页，每页1条记录，按时间倒序，也就是获取最新的一条记录
        auto selector = _Kits::SqlSelect<radar_data>();
        selector.select({"id", "points"}).where("id", OperatorComparison::LessThan, 1000).orderBy("id", false).paginate(1, 10).exec();

        // 处理查询结果
        auto datas = selector.getResults();
        for (const auto &data : datas)
        {
            qDebug() << "id:" << data.id << data.points;
        }
    }
} // namespace _Controllers