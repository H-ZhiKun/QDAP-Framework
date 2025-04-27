#include "ConcurrentPool.h"
#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <fmt/base.h>
#include <fmt/core.h>
#include <qlogging.h>
#include <utility>

using namespace _Kits;
ConcurrentPool::ConcurrentPool() : QObject(nullptr)
{
    start();
}

ConcurrentPool::~ConcurrentPool()
{
    stop();
}

void ConcurrentPool::start()
{
    auto threadPool = QThreadPool::globalInstance();
    threadSize_ = threadPool->maxThreadCount();
}

void ConcurrentPool::stop()
{
    bRun_ = false;
    for (auto [name, timer] : mapTimers_)
    {
        timer->stop();
    }
}

bool ConcurrentPool::runAfter(std::function<void(void)> &&task,
                              uint16_t milliseconds)
{
    if (milliseconds > 0)
    {
        QTimer::singleShot(milliseconds,
                           [this, task = std::move(task)]() mutable {
                               runTask(std::move(task));
                           });
    }
    else
    {
        return runTask(std::move(task));
    }
    return true;
}
void ConcurrentPool::runEvery(const std::string &funcName,
                              std::function<void(void)> &&task,
                              uint16_t milliseconds)
{
    QTimer *timer = new QTimer(this);
    mapTimers_[funcName] = timer;
    mapTasks_[funcName] = task;

    QObject::connect(
        timer, &QTimer::timeout, [this, funcName]() { timerTask(funcName); });

    timer->setInterval(milliseconds);
    timer->start();
}

bool ConcurrentPool::runTask(std::function<void(void)> &&func)
{
    if (!bRun_)
        return false;
    auto future = QtConcurrent::run(std::move(func));
    if (!future.isRunning())
    {
        qDebug() << "\nasync thread task start error.";
        return false;
    }
    return true;
}
void ConcurrentPool::timerTask(const std::string &funcName)
{
    if (!bRun_)
        return;
    auto iterTimer = mapTimers_.find(funcName);
    if (iterTimer == mapTimers_.end())
    {
        qDebug() << "\ntimer for task not found.";
        return;
    }

    auto timer = iterTimer->second;
    auto iterTask = mapTasks_.find(funcName);
    if (iterTask == mapTasks_.end())
    {
        qDebug() << "\ntimer task not found.";
        return;
    }

    // 停止计时器后执行任务
    timer->stop();
    auto future = QtConcurrent::run(
        [this, timer, funcName, task = iterTask->second]() -> void {
            task(); // 执行任务
            QMetaObject::invokeMethod(this, [timer]() {
                if (timer)
                    timer->start(); // 完成任务后重新启动计时器
            });
        });
    if (!future.isRunning())
    {
        qDebug() << "\nasync timer task start error: " << funcName;
    }
}