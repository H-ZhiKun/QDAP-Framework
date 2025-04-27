#include "CRossLogger.h"
#include "PlainTextSink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <filesystem>
#include <qlogging.h>
#include <yaml-cpp/yaml.h>

namespace _Kits
{
    spdlog::level::level_enum CRossLogger::getLogLevelFromString(
        std::string_view level)
    {
        if (level == "trace")
        {
            return spdlog::level::trace;
        }
        else if (level == "debug")
        {
            return spdlog::level::debug;
        }
        else if (level == "info")
        {
            return spdlog::level::info;
        }
        else if (level == "warn" || level == "warning")
        {
            return spdlog::level::warn;
        }
        else if (level == "err" || level == "error")
        {
            return spdlog::level::err;
        }
        else if (level == "critical")
        {
            return spdlog::level::critical;
        }
        else if (level == "off")
        {
            return spdlog::level::off;
        }
        // 默认级别为 info
        return spdlog::level::info;
    }
    spdlog::source_loc CRossLogger::getLogSourceLocation(
        const SourceLocation &location)
    {
        return spdlog::source_loc{location.FileName(),
                                  static_cast<int>(location.LineNum()),
                                  location.FuncName()};
    }

    std::string_view CRossLogger::getDefaultLogPattern()
    {
        return "%^[%Y-%m-%d "
               "%T.%e][%s|%!|%#][PID:%P,%t]%l: "
               "%v%$";
    }

    bool CRossLogger::initLogger()
    {
        std::string strfilePath =
            std::filesystem::current_path().string() + "/config/config.yaml";
        auto config = YAML::LoadFile(strfilePath);
        // 初始化日志记录器

        std::string strRootPath = "/log/";
        std::string strLogLevel = "debug";

        if (config["app"]["save_path"] && config["log"]["log_level"])
        {
            strRootPath =
                config["app"]["save_path"].as<std::string>() + "/log/";
            strLogLevel = config["log"]["log_level"].as<std::string>();
        }

        std::filesystem::path logPath(strRootPath);
        std::filesystem::path directory = logPath.parent_path();
        if (!directory.empty() && !std::filesystem::exists(directory))
        {
            if (!std::filesystem::create_directories(directory))
            {
                throw std::runtime_error(
                    fmt::format("Create logger path failed: {}", strRootPath));
            }
        }

        size_t maxFileSize = 50;
        std::string_view pattern = getDefaultLogPattern();
        auto fileSink = std::make_shared<PlainTextSink_mt>(
            std::string(strRootPath), maxFileSize * 1024 * 1024);
        m_level = getLogLevelFromString(strLogLevel);
        fileSink->set_level(m_level);
        fileSink->set_pattern(std::string(pattern));

        auto consoleSink =
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_level(m_level);
        consoleSink->set_pattern(std::string(pattern));

        std::vector<spdlog::sink_ptr> sinks{fileSink, consoleSink};
        m_logger = std::make_shared<spdlog::logger>(
            "CRossLogger", std::begin(sinks), std::end(sinks));
        m_logger->set_level(m_level);

        spdlog::set_default_logger(m_logger);
        return true;
    }
} // namespace _Kits