#pragma once
#include <QString>
#include <unordered_map>
namespace _Kits
{
// 比较操作符枚举
enum class OperatorType
{
    none = 0,
    select = 1,
    insert = 2
};
enum class OperatorComparison
{
    Equal,        // 等于
    NotEqual,     // 不等于
    GreaterThan,  // 大于
    LessThan,     // 小于
    GreaterEqual, // 大于等于
    LessEqual     // 小于等于
};

// 逻辑操作符枚举
enum class OperatorLogical
{
    And, // 逻辑与
    Or   // 逻辑或
};

class SqlOperators
{
  public:
    inline static const std::unordered_map<OperatorComparison, QString>
        comparisonOperatorMap = {{OperatorComparison::Equal, "="},
                                 {OperatorComparison::NotEqual, "!="},
                                 {OperatorComparison::GreaterThan, ">"},
                                 {OperatorComparison::LessThan, "<"},
                                 {OperatorComparison::GreaterEqual, ">="},
                                 {OperatorComparison::LessEqual, "<="}};

    // 定义逻辑操作符与字符串的映射
    inline static const std::unordered_map<OperatorLogical, QString>
        logicalOperatorMap = {{OperatorLogical::And, "AND"},
                              {OperatorLogical::Or, "OR"}};
};
} // namespace _Kits
