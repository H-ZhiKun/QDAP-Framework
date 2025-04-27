// Auto-generated template implementations, 不要自己改，有问题问我！
#pragma once
#include "TableStructs.h"
#include <unordered_map>
#include <QVariant>
#include <QSqlRecord>

// === 通用 ORM 宏定义 ===
namespace _Kits {

#define ORM_FIELD_TO_MAP(field) {#field, obj.field},
#define ORM_FIELD_FROM_MAP(field) obj.field = map.at(#field).value<std::decay_t<decltype(obj.field)>>();
#define ORM_FIELD_FROM_RECORD(field) obj.field = record.value(#field).value<std::decay_t<decltype(obj.field)>>();

#define DECLARE_ORM_MAPPER(TYPE, FIELD_LIST) \
template <> struct OrmMapper<TYPE> { \
    static std::unordered_map<std::string, QVariant> toMap(const TYPE& obj) { \
        return { FIELD_LIST(ORM_FIELD_TO_MAP) }; \
    } \
    static TYPE fromMap(const std::unordered_map<std::string, QVariant>& map) { \
        TYPE obj; \
        FIELD_LIST(ORM_FIELD_FROM_MAP) \
        return obj; \
    } \
    static TYPE fromRecord(const QSqlRecord& record) { \
        TYPE obj; \
        FIELD_LIST(ORM_FIELD_FROM_RECORD) \
        return obj; \
    } \
};

template <typename T>
struct OrmMapper;

// === 字段宏: device_status ===
#define DEVICE_STATUS_FIELDS(F) \
    F(id) \
    F(tag) \
    F(details_json) \
    F(created_time) \
    F(updated_time)

DECLARE_ORM_MAPPER(device_status, DEVICE_STATUS_FIELDS)

// === 字段宏: mvb_line_data ===
#define MVB_LINE_DATA_FIELDS(F) \
    F(id) \
    F(tag) \
    F(details_json) \
    F(created_time) \
    F(updated_time)

DECLARE_ORM_MAPPER(mvb_line_data, MVB_LINE_DATA_FIELDS)

// === 字段宏: location_data ===
#define LOCATION_DATA_FIELDS(F) \
    F(id) \
    F(station_name) \
    F(maoduan_name) \
    F(pole_name) \
    F(train_move_dis) \
    F(speed) \
    F(created_time) \
    F(updated_time)

DECLARE_ORM_MAPPER(location_data, LOCATION_DATA_FIELDS)

// === 字段宏: radar_data ===
#define RADAR_DATA_FIELDS(F) \
    F(id) \
    F(location_id) \
    F(task_id) \
    F(points) \
    F(created_time) \
    F(updated_time)

DECLARE_ORM_MAPPER(radar_data, RADAR_DATA_FIELDS)

// === 字段宏: radar_over_data ===
#define RADAR_OVER_DATA_FIELDS(F) \
    F(id) \
    F(location_id_start) \
    F(location_id_end) \
    F(task_id) \
    F(points) \
    F(direction) \
    F(overrun_time) \
    F(point_count) \
    F(created_time) \
    F(updated_time)

DECLARE_ORM_MAPPER(radar_over_data, RADAR_OVER_DATA_FIELDS)

// === 字段宏: task_data ===
#define TASK_DATA_FIELDS(F) \
    F(id) \
    F(task_name) \
    F(line_name) \
    F(line_dir) \
    F(direction) \
    F(start_station) \
    F(end_station) \
    F(start_pole) \
    F(created_time) \
    F(updated_time)

DECLARE_ORM_MAPPER(task_data, TASK_DATA_FIELDS)

// === 字段宏: line_data ===
#define LINE_DATA_FIELDS(F) \
    F(id) \
    F(tag_id) \
    F(station_name) \
    F(line_name) \
    F(maoduan_name) \
    F(kilo_meter) \
    F(pole_name) \
    F(line_dir) \
    F(created_time) \
    F(updated_time)

DECLARE_ORM_MAPPER(line_data, LINE_DATA_FIELDS)

// === 字段宏: arc_data ===
#define ARC_DATA_FIELDS(F) \
    F(id) \
    F(arc_count) \
    F(arc_time) \
    F(arc_pulse) \
    F(arc_timestamp) \
    F(arcvideo_path) \
    F(created_time) \
    F(updated_time)

DECLARE_ORM_MAPPER(arc_data, ARC_DATA_FIELDS)

} // namespace _Kits
