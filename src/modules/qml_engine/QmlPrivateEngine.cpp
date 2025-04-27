#include "QmlPrivateEngine.h"
#include "QmlCommunication.h"
#include "kits/qml_kits/ImagePainter.h"
#include "kits/required/log/CRossLogger.h"
#include "kits/required/thread_pool/ConcurrentPool.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <any>
#include <json/value.h>
#include <memory>
#include <qdir.h>
#include <qdiriterator.h>
#include <qglobal.h>
#include <qlogging.h>
#include <qmetaobject.h>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qthread.h>
#include <qtmetamacros.h>

using namespace _Modules;

QmlPrivateEngine::QmlPrivateEngine(QObject *parent) : ModuleBase(parent)
{
}
QmlPrivateEngine::~QmlPrivateEngine()
{
}

bool QmlPrivateEngine::start(const YAML::Node &config)
{
    m_bRunning = true;
    // 查看资源列表
    QDirIterator it(":/", QDirIterator::Subdirectories);
    //     while (it.hasNext())
    //     {
    //         qDebug() << "it:" << it.next() << "\n";
    //     }

    QQuickStyle::setStyle("FluentWinUI3"); // 强制设置 Material 风格
    qDebug() << QQuickStyle::name() << "\n";
    // 注册 C++ 类型到 QML
    qmlRegisterType<QmlCommunication>("InfinityStation", 1, 0, "QmlCommunication");
    qmlRegisterType<_Kits::ImagePainter>("InfinityStation", 1, 0, "ImagePainter");
    // 创建通信对象
    auto communication = std::make_unique<QmlCommunication>();
    communication->start();
    QObject::connect(communication.get(), &QmlCommunication::callFromQml, this, &QmlPrivateEngine::afterQmlActions);
    QObject::connect(this, &QmlPrivateEngine::callFromCpp, communication.get(), &QmlCommunication::dataFromCpp);
    // 创建Qml引擎
    auto engine = std::make_unique<QQmlApplicationEngine>();

    engine->addImportPath(":/infinity_station");
    engine->addImportPath(QCoreApplication::applicationDirPath());

    qDebug() << "QML import paths:" << engine->importPathList();
    engine->rootContext()->setContextProperty("qmlCommunication", communication.get());

    const QUrl mainQmlUrl(QStringLiteral("qrc:/infinity_station/res/qml/electricbus_qml/main.qml"));
    engine->load(mainQmlUrl);
    m_qmlEngine = std::move(engine);
    m_communication = std::move(communication);
    return true;
}

bool QmlPrivateEngine::stop()
{
    m_bRunning = false;
    if (m_qmlEngine)
    {
        auto engine = qobject_cast<QQmlApplicationEngine *>(m_qmlEngine.get());
        engine->rootContext()->setContextProperty("qmlCommunication", nullptr);
        // 清空组件缓存，确保 QML 内部组件被提前释放
        engine->clearComponentCache();

        // 遍历所有 QML 根对象，使用 deleteLater() 让它们延迟删除
        const auto roots = engine->rootObjects();
        for (QObject *root : roots)
        {
            if (root)
            {
                root->deleteLater();
            }
        }

        // 可选：处理一下事件，确保 deleteLater() 操作被执行
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

        // 清理 QML 引擎对象
        m_qmlEngine.reset();
        m_communication.reset();
    }
    return true;
}

bool QmlPrivateEngine::customization(const std::unordered_map<_Kits::RegisterKey, std::list<std::any>> &lvTasks)
{
    for (auto &[key, tasks] : lvTasks)
    {
        int nKey = std::get<1>(key);
        for (auto &task : tasks)
        {
            m_storeQmlTask[nKey].push_back(std::any_cast<std::function<void(const QVariant &)>>(task));
        }
    }
    return true;
}

void QmlPrivateEngine::afterQmlActions(int qmlAction, const QVariant &value)
{
    auto finder = m_storeQmlTask.find(qmlAction);
    if (finder == m_storeQmlTask.end())
    {
        qDebug() << "qml call not register: " << QmlCommunication::enumToString(static_cast<QmlCommunication::QmlActions>(qmlAction));
        return;
    }
    auto &lvFuncs = finder->second;
    for (auto &func : lvFuncs)
    {
        if (!_Kits::threadPool().runAfter([func, value]() mutable { func(value); }))
        {
            _Kits::LogError("controller task run error: {}", qmlAction);
        }
    }
}
std::string QmlPrivateEngine::testArgs(int n, std::string s, float f)
{
    _Kits::LogInfo("n = {}, s = {}, f = {}", n, s, f);
    return s;
}