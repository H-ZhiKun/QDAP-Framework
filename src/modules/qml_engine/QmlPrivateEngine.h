#pragma once
#include "kits/required/module_base/ModuleBase.h"
#include <functional>
#include <json/json.h>
#include <qtmetamacros.h>
#include <qvariant.h>

namespace _Modules
{

    /**
     * @class QmlPrivateEngine
     * @brief QmlPrivateEngine 继承自 ModuleBase，用于处理 QML 交互。
     */
    class QmlPrivateEngine : public _Kits::ModuleBase
    {
        Q_OBJECT
        DECLARE_MODULE(QmlPrivateEngine)

      public:
        explicit QmlPrivateEngine(QObject *parent = nullptr);
        virtual ~QmlPrivateEngine();

        /**
         * @brief 启动 QmlPrivateEngine 模块。
         * @param config 配置文件的 YAML 节点。
         * @return 启动成功返回 true，否则返回 false。
         */
        virtual bool start(const YAML::Node &config) override;

        virtual bool stop() override;
        virtual bool customization(const std::unordered_map<_Kits::RegisterKey, std::list<std::any>> &lvTasks) override;
      signals:

        /**
         * @brief 通过 invokeModule 触发的信号。
         *
         * 在 controller 中使用 QVariantMap 作为参数调用此信号，格式如下：
         * @code
         * QVariantMap mapData;
         * mapData["pageName"] = "mainPage";
         * mapData["pageData"] = obj;
         * @endcode
         *
         * - `mainPage` 和其他页面名称在 QmlCommunication 构造函数中定义。
         * - `obj` 可以是任意 QVariant 类型支持的对象。
         * - 以上格式必须遵循，才可以正确解析。
         *
         * @param value 传递的数据。
         */
        void callFromCpp(const QVariant &value);
      private slots:
        void afterQmlActions(int qml, const QVariant &value);
        std::string testArgs(int n, std::string s, float f); // 测试阻塞调用

      private:
        std::unique_ptr<QObject> m_qmlEngine = nullptr;     /**< QML 引擎实例。*/
        bool m_bRunning = true;                             /**< 运行状态标志。*/
        std::unique_ptr<QObject> m_communication = nullptr; /**< QML 交互对象。*/
        std::unordered_map<int, std::vector<std::function<void(const QVariant &)>>> m_storeQmlTask;
    };

} // namespace _Modules
