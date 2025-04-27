#pragma once
#include <QMetaEnum>
#include <QObject>
#include <QVariant>
#include <qmetaobject.h>
// #include <qqmlintegration.h>
#include <unordered_map>

namespace _Modules
{
    /**
     * @class QmlCommunication
     * @brief 处理 C++ 与 QML 之间的信号和数据传输。
     */
    class QmlCommunication : public QObject
    {
        Q_OBJECT

      public:
        /**
         * @enum QmlActions
         * @brief 定义 QML 触发的操作类型，供cpp端使用。
         */
        enum class QmlActions
        {
            PageChange = 0,           // 页面切换：在切换不同业务页面时调用
            ImageCapture = 1,         // 图片抓取
            LocationRecv = 2,         // 定位数据接收
            MVBTest = 3,              // MVB测试
            TaskSend = 4,             // 任务测试
            TaskControllerTest = 5,   // 任务控制
            CorrugationGet = 6,       // 波磨数据获取
            VideoGet = 7,             // 录像保存
            ArcUVGet = 8,             // 燃弧数据获取
            offlineRadarTaskidGet = 9, // 离线雷达任务id获取
            TaskCSVSend = 10,       // 任务csv文件发送
            LineNameSend = 11,       // 线路名称发送
            LineBtnChoose = 12       // 线路选择按钮
        };
        Q_ENUM(QmlActions) ///< 注册枚举以支持 Qt 元对象系统。
        /**
         * @enum ForQmlSignals
         * @brief 定义 cpp 触发的信号，供qml中绑定使用。
         * @note 0-20为页面信号，21-40为页面中得操作
         */
        enum class ForQmlSignals
        {
            main_page = 0,
            mvb_page = 1,
            location_page = 2,
            task_page = 3,
            video_page = 4,
            corrugation_page = 5,
            arc_page = 6,
            radar_page = 7,
            taskcontroller_page = 8,
            offlineradar_page = 9,
            tasktable_recv = 21, //燃弧界面获取任务信息表信号
            locationdata_recv = 22, //燃弧界面定位信息表获取
            linedata_recv = 23,//线路信息表获取
            lineNameList_recv = 24,
            taskdata_recv = 25,//任务信息表获取,打开CSV方式
        };
        Q_ENUM(ForQmlSignals)

        explicit QmlCommunication(QObject *parent = nullptr);
        virtual ~QmlCommunication();
        bool start();
        /**
         * @brief 处理 QML 传递的行为。
         * @param action QML 触发的操作。
         * @param data 可选的附加数据，默认为 QVariant()。
         */
        Q_INVOKABLE void behaviorFromQml(QmlActions action,
                                         const QVariant &data = QVariant());

      signals:
        /**
         * @brief 由 QML 调用的信号。
         * @param title 信号名称。
         * @param value 传递的数据。
         */
        void callFromQml(int qml, const QVariant &value);

        // 发送到qml的页面信号 begin
        // 从mainPageSignal->main_pageFromCpp 信号，其他页面类似
        void main_pageFromCpp(const QVariant &value);
        void mvb_pageFromCpp(const QVariant &value);
        void location_pageFromCpp(const QVariant &value);
        void taskcontroller_pageFromCpp(const QVariant &value);                          // 任务控制页面信号
        void task_pageFromCpp(const QVariant &value);        // 任务页面信号
        void video_pageFromCpp(const QVariant &value);       // 录像界面1信号
        void corrugation_pageFromCpp(const QVariant &value); // 波磨页面信号
        void arc_pageFromCpp(const QVariant &value);         // 燃弧页面信号
        void radar_pageFromCpp(const QVariant &value);       // 限界检测页面信号
        void offlineradar_pageFromCpp(const QVariant &value); // 离线雷达页面信号
        void tasktable_recvFromCpp(const QVariant &value);   // 任务信息表获取信号
        void locationdata_recvFromCpp(const QVariant &value); // 定位信息表获取信号
        void linedata_recvFromCpp(const QVariant &value); // 线路信息表获取信号
        void lineNameList_recvFromCpp(const QVariant &value);
        void taskdata_recvFromCpp(const QVariant &value); // 任务信息表获取信号

        // 发送到qml的页面信号 end

      public slots:
        /**
         * @brief 处理来自 C++ 端的数据，并根据不同的页面选择合适的信号发送到
         * QML。
         * @param value 传递的数据。
         */
        void dataFromCpp(const QVariant &value);

      protected:
        void storePageSignal(int pageName, bool pageIn);

      public:
        /**
         * @brief 将T枚举值转换为对应的字符串。
         * @param 枚举值。
         * @return QString 对应的字符串名称。
         */
        template <typename T>
        static QString enumToString(T value)
        {
            // 获取枚举类型的元信息
            QMetaEnum metaEnum = QMetaEnum::fromType<T>();
            // 将枚举值转换为字符串
            const char *key = metaEnum.valueToKey(static_cast<int>(value));
            return key ? QString(key) : QString();
        }
        template <typename T>
        static int keyToValue(const char *key)
        {
            QMetaEnum metaEnum = QMetaEnum::fromType<T>();
            return metaEnum.keysToValue(key);
        }

      private:
        void tidyMetaMethod();
        QmlCommunication(const QmlCommunication &) = delete;
        QmlCommunication &operator=(const QmlCommunication &) = delete;

        std::unordered_map<int, QMetaMethod> m_mapPageSignal;
        std::unordered_map<int, QMetaMethod> m_mapEnumSignals;
    };
} // namespace _Modules
