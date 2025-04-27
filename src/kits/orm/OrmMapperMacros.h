#pragma once
#include <QSqlRecord>
#include <QVariant>

namespace _Kits
{

#define DEFINE_ORM_MAPPER(TableName, ...)                                                                                                  \
    template <>                                                                                                                            \
    struct OrmMapper<TableName>                                                                                                            \
    {                                                                                                                                      \
        static std::unordered_map<std::string, QVariant> toMap(const TableName &obj)                                                       \
        {                                                                                                                                  \
            return {EXPAND_FIELDS(obj, obj)};                                                                                              \
        }                                                                                                                                  \
        static TableName fromMap(const std::unordered_map<std::string, QVariant> &map)                                                     \
        {                                                                                                                                  \
            TableName obj;                                                                                                                 \
            EXPAND_FIELDS_FROM_MAP(obj, map);                                                                                              \
            return obj;                                                                                                                    \
        }                                                                                                                                  \
        static TableName fromRecord(const QSqlRecord &record)                                                                              \
        {                                                                                                                                  \
            TableName obj;                                                                                                                 \
            EXPAND_FIELDS_FROM_RECORD(obj, record);                                                                                        \
            return obj;                                                                                                                    \
        }                                                                                                                                  \
    };

#define EXPAND_FIELDS(obj, ...)               EXPAND_FIELDS_HELPER(obj, __VA_ARGS__)
#define EXPAND_FIELDS_HELPER(obj, first, ...) {#first, obj.first} IF_HAS_ARGS(__VA_ARGS__)(, EXPAND_FIELDS_HELPER(obj, __VA_ARGS__))

#define EXPAND_FIELDS_FROM_MAP(obj, map, ...) EXPAND_FIELDS_FROM_MAP_HELPER(obj, map, __VA_ARGS__)
#define EXPAND_FIELDS_FROM_MAP_HELPER(obj, map, first, ...)                                                                                \
    obj.first = map.at(#first).value<decltype(obj.first)>();                                                                               \
    IF_HAS_ARGS(__VA_ARGS__)(EXPAND_FIELDS_FROM_MAP_HELPER(obj, map, __VA_ARGS__))

#define EXPAND_FIELDS_FROM_RECORD(obj, record, ...) EXPAND_FIELDS_FROM_RECORD_HELPER(obj, record, __VA_ARGS__)
#define EXPAND_FIELDS_FROM_RECORD_HELPER(obj, record, first, ...)                                                                          \
    obj.first = record.value(#first).value<decltype(obj.first)>();                                                                         \
    IF_HAS_ARGS(__VA_ARGS__)(EXPAND_FIELDS_FROM_RECORD_HELPER(obj, record, __VA_ARGS__))

#define IF_HAS_ARGS(...)             _IF_HAS_ARGS(__VA_ARGS__, 1, 0)
#define _IF_HAS_ARGS(_0, _1, N, ...) N
} // namespace _Kits
