#pragma once

#include "SourceLocation.h"
#include "spdlog/spdlog.h"
#include <fmt/format.h>
#include <spdlog/common.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace _Kits
{
class CRossLogger final
{
  public:
    static CRossLogger &getLogger()
    {
        static CRossLogger logger;
        return logger;
    }
    ~CRossLogger()
    {
        spdlog::shutdown();
        spdlog::drop_all();
    }

    spdlog::level::level_enum getLogLevelFromString(std::string_view level);
    std::string_view getDefaultLogPattern();
    spdlog::source_loc getLogSourceLocation(const SourceLocation &location);
    bool initLogger();
    int getLogLevel()
    {
        return m_level;
    }

  protected:
    CRossLogger()
    {
        initLogger();
    }

  private:
    spdlog::level::level_enum m_level;
    std::shared_ptr<spdlog::logger> m_logger;
};

// trace
template <typename... Args>
struct LogTrace
{
    LogTrace(fmt::format_string<Args...> fmt,
             Args &&...args,
             SourceLocation location = {})
    {
        spdlog::log(CRossLogger::getLogger().getLogSourceLocation(location),
                    spdlog::level::trace,
                    fmt,
                    std::forward<Args>(args)...);
    }
};

template <typename... Args>
LogTrace(fmt::format_string<Args...> fmt, Args &&...args) -> LogTrace<Args...>;

// debug
template <typename... Args>
struct LogDebug
{
    LogDebug(fmt::format_string<Args...> fmt,
             Args &&...args,
             SourceLocation location = {})
    {
        spdlog::log(CRossLogger::getLogger().getLogSourceLocation(location),
                    spdlog::level::debug,
                    fmt,
                    std::forward<Args>(args)...);
    }
};

template <typename... Args>
LogDebug(fmt::format_string<Args...> fmt, Args &&...args) -> LogDebug<Args...>;

// info
template <typename... Args>
struct LogInfo
{
    LogInfo(fmt::format_string<Args...> fmt,
            Args &&...args,
            SourceLocation location = {})
    {
        spdlog::log(CRossLogger::getLogger().getLogSourceLocation(location),
                    spdlog::level::info,
                    fmt,
                    std::forward<Args>(args)...);
    }
};

template <typename... Args>
LogInfo(fmt::format_string<Args...> fmt, Args &&...args) -> LogInfo<Args...>;

// warn
template <typename... Args>
struct LogWarn
{
    LogWarn(fmt::format_string<Args...> fmt,
            Args &&...args,
            SourceLocation location = {})
    {
        spdlog::log(CRossLogger::getLogger().getLogSourceLocation(location),
                    spdlog::level::warn,
                    fmt,
                    std::forward<Args>(args)...);
        auto logger = spdlog::get("CrossLogger");
        if (logger)
        {
            logger->flush();
        }
    }
};

template <typename... Args>
LogWarn(fmt::format_string<Args...> fmt, Args &&...args) -> LogWarn<Args...>;

// error
template <typename... Args>
struct LogError
{
    LogError(fmt::format_string<Args...> fmt,
             Args &&...args,
             SourceLocation location = {})
    {
        spdlog::log(CRossLogger::getLogger().getLogSourceLocation(location),
                    spdlog::level::err,
                    fmt,
                    std::forward<Args>(args)...);
        auto logger = spdlog::get("CrossLogger");
        if (logger)
        {
            logger->flush();
        }
    }
};

template <typename... Args>
LogError(fmt::format_string<Args...> fmt, Args &&...args) -> LogError<Args...>;

// critical
template <typename... Args>
struct LogCritical
{
    LogCritical(fmt::format_string<Args...> fmt,
                Args &&...args,
                SourceLocation location = {})
    {
        spdlog::log(CRossLogger::getLogger().getLogSourceLocation(location),
                    spdlog::level::critical,
                    fmt,
                    std::forward<Args>(args)...);
    }
};

template <typename... Args>
LogCritical(fmt::format_string<Args...> fmt, Args &&...args)
    -> LogCritical<Args...>;

} // namespace _Kits
