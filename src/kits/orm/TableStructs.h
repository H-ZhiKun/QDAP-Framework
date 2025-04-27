// Auto-generated template implementations, 不要自己改，有问题问我！
#pragma once
#include<QString>
#include<QJsonObject>
#include <QDateTime>
namespace _Kits{
struct device_status {
    int id = -1;
    QString tag = {};
    QString details_json = {};
    QString created_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString updated_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    static QString tableName() { return "device_status"; }
};

struct mvb_line_data {
    int id = -1;
    QString tag = {};
    QString details_json = {};
    QString created_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString updated_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    static QString tableName() { return "mvb_line_data"; }
};

struct location_data {
    int id = -1;
    QString station_name = {};
    QString maoduan_name = {};
    QString pole_name = {};
    float train_move_dis = {};
    float speed = {};
    QString created_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString updated_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    static QString tableName() { return "location_data"; }
};

struct radar_data {
    int id = -1;
    int location_id = -1;
    int task_id = -1;
    QString points = {};
    QString created_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString updated_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    static QString tableName() { return "radar_data"; }
};

struct radar_over_data {
    int id = -1;
    int location_id_start = -1;
    int location_id_end = -1;
    int task_id = -1;
    QString points = {};
    QString direction = {};
    int overrun_time = -1;
    int point_count = -1;
    QString created_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString updated_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    static QString tableName() { return "radar_over_data"; }
};

struct task_data {
    int id = -1;
    QString task_name = {};
    QString line_name = {};
    int line_dir = -1;
    int direction = -1;
    QString start_station = {};
    QString end_station = {};
    QString start_pole = {};
    QString created_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString updated_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    static QString tableName() { return "task_data"; }
};

struct line_data {
    int id = -1;
    int tag_id = -1;
    QString station_name = {};
    QString line_name = {};
    QString maoduan_name = {};
    float kilo_meter = {};
    QString pole_name = {};
    int line_dir = -1;
    QString created_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString updated_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    static QString tableName() { return "line_data"; }
};

struct arc_data {
    int id = -1;
    int arc_count = -1;
    float arc_time = {};
    int arc_pulse = -1;
    qint64 arc_timestamp = {};
    QString arcvideo_path = {};
    QString created_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString updated_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    static QString tableName() { return "arc_data"; }
};


}