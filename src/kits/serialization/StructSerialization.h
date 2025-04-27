// kits_struct_serialization.h
#pragma once

#include <QDateTime>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <json/json.h>
#include <type_traits>

namespace _Kits
{

    template <typename T>
    struct struct_serialization;

    template <typename T>
    struct struct_serialization
    {
        static T fromMap(const QVariantMap &)
        {
            static_assert(sizeof(T) == 0, "struct_serialization<T> not specialized for this type. Please use STRUCT_SERIALIZATION.");
        }
        static QVariantMap toMap(const T &)
        {
            static_assert(sizeof(T) == 0, "struct_serialization<T> not specialized for this type. Please use STRUCT_SERIALIZATION.");
        }
        static Json::Value toJson(const T &)
        {
            static_assert(sizeof(T) == 0, "struct_serialization<T> not specialized for this type. Please use STRUCT_SERIALIZATION.");
        }
        static T fromJson(const Json::Value &)
        {
            static_assert(sizeof(T) == 0, "struct_serialization<T> not specialized for this type. Please use STRUCT_SERIALIZATION.");
        }
    };

// 辅助宏，展开每个字段的赋值、转换
#define _KITS_STRUCT_FIELD_FROM_MAP(field)                                                                                                                                                             \
    if (map.contains(#field))                                                                                                                                                                          \
    {                                                                                                                                                                                                  \
        obj.field = map[#field].value<std::decay_t<decltype(obj.field)>>();                                                                                                                            \
    }

#define _KITS_STRUCT_FIELD_TO_MAP(field) map[#field] = QVariant::fromValue(obj.field);

#define _KITS_STRUCT_FIELD_TO_JSON(field)                                                                                                                                                              \
    value[#field] = Json::Value::null;                                                                                                                                                                 \
    _Kits::detail::setJsonValue(value[#field], obj.field);

#define _KITS_STRUCT_FIELD_FROM_JSON(field)                                                                                                                                                            \
    if (value.isMember(#field))                                                                                                                                                                        \
    {                                                                                                                                                                                                  \
        _Kits::detail::getJsonValue(value[#field], obj.field);                                                                                                                                         \
    }

    namespace detail
    {
        // 针对 toJson
        inline void setJsonValue(Json::Value &jsonVal, const QString &val)
        {
            jsonVal = val.toStdString();
        }
        inline void setJsonValue(Json::Value &jsonVal, const std::string &val)
        {
            jsonVal = val;
        }
        inline void setJsonValue(Json::Value &jsonVal, const int &val)
        {
            jsonVal = val;
        }
        inline void setJsonValue(Json::Value &jsonVal, const double &val)
        {
            jsonVal = val;
        }
        inline void setJsonValue(Json::Value &jsonVal, const bool &val)
        {
            jsonVal = val;
        }

        // 针对 fromJson
        inline void getJsonValue(const Json::Value &jsonVal, QString &val)
        {
            if (jsonVal.isString())
                val = QString::fromStdString(jsonVal.asString());
        }
        inline void getJsonValue(const Json::Value &jsonVal, std::string &val)
        {
            if (jsonVal.isString())
                val = jsonVal.asString();
        }
        inline void getJsonValue(const Json::Value &jsonVal, int &val)
        {
            if (jsonVal.isInt())
                val = jsonVal.asInt();
        }
        inline void getJsonValue(const Json::Value &jsonVal, double &val)
        {
            if (jsonVal.isDouble())
                val = jsonVal.asDouble();
        }
        inline void getJsonValue(const Json::Value &jsonVal, bool &val)
        {
            if (jsonVal.isBool())
                val = jsonVal.asBool();
        }
    } // namespace detail

#define STRUCT_SERIALIZATION(StructType, ...)                                                                                                                                                          \
    namespace _Kits                                                                                                                                                                                    \
    {                                                                                                                                                                                                  \
        template <>                                                                                                                                                                                    \
        struct struct_serialization<StructType>                                                                                                                                                        \
        {                                                                                                                                                                                              \
            static StructType fromMap(const QVariantMap &map)                                                                                                                                          \
            {                                                                                                                                                                                          \
                StructType obj;                                                                                                                                                                        \
                (void)map;                                                                                                                                                                             \
                _KITS_FOREACH(_KITS_STRUCT_FIELD_FROM_MAP, __VA_ARGS__)                                                                                                                                \
                return obj;                                                                                                                                                                            \
            }                                                                                                                                                                                          \
            static QVariantMap toMap(const StructType &obj)                                                                                                                                            \
            {                                                                                                                                                                                          \
                QVariantMap map;                                                                                                                                                                       \
                (void)obj;                                                                                                                                                                             \
                _KITS_FOREACH(_KITS_STRUCT_FIELD_TO_MAP, __VA_ARGS__)                                                                                                                                  \
                return map;                                                                                                                                                                            \
            }                                                                                                                                                                                          \
            static Json::Value toJson(const StructType &obj)                                                                                                                                           \
            {                                                                                                                                                                                          \
                Json::Value value(Json::objectValue);                                                                                                                                                  \
                (void)obj;                                                                                                                                                                             \
                _KITS_FOREACH(_KITS_STRUCT_FIELD_TO_JSON, __VA_ARGS__)                                                                                                                                 \
                return value;                                                                                                                                                                          \
            }                                                                                                                                                                                          \
            static StructType fromJson(const Json::Value &value)                                                                                                                                       \
            {                                                                                                                                                                                          \
                StructType obj;                                                                                                                                                                        \
                (void)value;                                                                                                                                                                           \
                _KITS_FOREACH(_KITS_STRUCT_FIELD_FROM_JSON, __VA_ARGS__)                                                                                                                               \
                return obj;                                                                                                                                                                            \
            }                                                                                                                                                                                          \
        };                                                                                                                                                                                             \
    }                                                                                                                                                                                                  \
    static_assert(true, "STRUCT_SERIALIZATION for " #StructType) inline constexpr bool _kits_struct_serialization_dummy_##StructType = true;

// 辅助宏
#define _KITS_FOREACH(macro, ...)                _KITS_FOREACH_IMPL(macro, __VA_ARGS__)
#define _KITS_FOREACH_IMPL(macro, a1, ...)       macro(a1) IF_HAS_ARGS(__VA_ARGS__)(_KITS_FOREACH_IMPL_AGAIN(macro, __VA_ARGS__))
#define _KITS_FOREACH_IMPL_AGAIN(macro, a1, ...) macro(a1) IF_HAS_ARGS(__VA_ARGS__)(_KITS_FOREACH_IMPL_AGAIN(macro, __VA_ARGS__))

#define HAS_ARGS(...)                            BOOL(HAS_ARGS_HELPER(__VA_ARGS__, 1, 0))
#define HAS_ARGS_HELPER(x, y, ...)               y
#define BOOL(x)                                  BOOL_##x
#define BOOL_1                                   1
#define BOOL_0                                   0
#define IF_HAS_ARGS(...)                         IF_HAS_ARGS_HELPER(HAS_ARGS(__VA_ARGS__))
#define IF_HAS_ARGS_HELPER(x)                    IF_HAS_ARGS_##x
#define IF_HAS_ARGS_1(x)                         x
#define IF_HAS_ARGS_0(x)

} // namespace _Kits
