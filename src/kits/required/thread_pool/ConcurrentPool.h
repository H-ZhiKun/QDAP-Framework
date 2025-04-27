#pragma once
#include <QTimer>
#include <cstdint>
#include <functional>
#include <qobject.h>
#include <qtmetamacros.h>
#include <string>
#include <unordered_map>


namespace _Kits
{
class ConcurrentPool : QObject
{
    Q_OBJECT
  public:
    inline static ConcurrentPool &instance()
    {
        static ConcurrentPool pool_;
        return pool_;
    }
    virtual ~ConcurrentPool();
    bool runAfter(std::function<void(void)> &&task, uint16_t milliseconds = 0);
    void runEvery(const std::string &funcName,
                  std::function<void(void)> &&task,
                  uint16_t milliseconds);
    void start();
    void stop();

  protected:
    explicit ConcurrentPool();
    bool runTask(std::function<void(void)> &&func);
    void timerTask(const std::string &funcName);

  private:
    uint32_t threadSize_ = 0;
    bool bRun_ = true;
    std::unordered_map<std::string, QTimer *> mapTimers_;
    std::unordered_map<std::string, std::function<void(void)>> mapTasks_;
};
inline ConcurrentPool &threadPool()
{
    return ConcurrentPool::instance();
}
} // namespace _Kits