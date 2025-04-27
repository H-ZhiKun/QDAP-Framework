
#pragma once
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
namespace _Kits
{
template <typename Mutex>
class HtmlFormatSink final : public spdlog::sinks::base_sink<Mutex>
{
  public:
    HtmlFormatSink(spdlog::filename_t baseFileName,
                   std::size_t maxSize,
                   std::size_t maxFiles,
                   bool rotateOnOpen = false,
                   const spdlog::file_event_handlers &eventHandlers = {})
        : m_base_filename(std::move(baseFileName)), m_max_size(maxSize),
          m_max_files(maxFiles), m_file_helper(eventHandlers)
    {
        if (maxSize == 0)
        {
            spdlog::throw_spdlog_ex(
                "rotating sink constructor: max_size arg cannot be zero");
        }

        if (maxFiles > 200000)
        {
            spdlog::throw_spdlog_ex("rotating sink constructor: max_files arg "
                                    "cannot exceed 200000");
        }
        m_file_helper.open(calc_filename(m_base_filename, 0));

        // 写入html头
        spdlog::memory_buf_t htmlHeader;
        const char *pHtmlHeader =
            R"(<html>
                <head>
                <meta http-equiv="content-type" content="text/html; charset-gb2312">
                <title>Html Output</title>
                </head>
                <body>
                <font face="Fixedsys" size="2" color="#0000FF">)";
        htmlHeader.append(pHtmlHeader, pHtmlHeader + std::strlen(pHtmlHeader));
        m_file_helper.write(htmlHeader);

        m_current_size = m_file_helper.size();
        if (rotateOnOpen && m_current_size > 0)
        {
            rotate_();
            m_current_size = 0;
        }
    }

    static spdlog::filename_t calc_filename(const spdlog::filename_t &fileName,
                                            std::size_t index)
    {
        spdlog::filename_t basename;
        spdlog::filename_t ext;
        std::tie(basename, ext) =
            spdlog::details::file_helper::split_by_extension(fileName);
        std::time_t timeVar = 0;
        std::time(&timeVar);
        char pTimeStr[64] = {0};
        std::strftime(pTimeStr,
                      sizeof(pTimeStr),
                      "%Y%m%d_%H%M%S",
                      std::localtime(&timeVar));
        return spdlog::fmt_lib::format(
            SPDLOG_FILENAME_T("{}_{}{}"), basename, pTimeStr, ext);
    }

    spdlog::filename_t filename()
    {
        std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::mutex_);
        return m_file_helper.filename();
    }

  protected:
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        spdlog::memory_buf_t formatted;
        const char *pPrefix = GetLogLevelHtmlPrefix(msg.level);
        // 填充html前缀
        formatted.append(pPrefix, pPrefix + std::strlen(pPrefix));

        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

        // 填充后缀
        const char *pSuffix = R"(<br></font>)";
        formatted.append(pSuffix, pSuffix + std::strlen(pSuffix));

        auto newSize = m_current_size + formatted.size();

        if (newSize > m_max_size)
        {
            m_file_helper.flush();
            if (m_file_helper.size() > 0)
            {
                rotate_();
                newSize = formatted.size();
            }
        }
        m_file_helper.write(formatted);
        m_current_size = newSize;
    }

    void flush_() override
    {
        m_file_helper.flush();
    }

  private:
    void rotate_()
    {
        using namespace spdlog;
        using details::os::filename_to_str;
        using details::os::path_exists;

        m_file_helper.close();
        for (auto i = m_max_files; i > 0; --i)
        {
            filename_t src = calc_filename(m_base_filename, i - 1);
            if (!path_exists(src))
            {
                continue;
            }
            filename_t target = calc_filename(m_base_filename, i);

            if (!rename_file_(src, target))
            {
                details::os::sleep_for_millis(100);
                if (!rename_file_(src, target))
                {
                    m_file_helper.reopen(true);
                    m_current_size = 0;
                    throw_spdlog_ex("rotating_file_sink: failed renaming " +
                                        filename_to_str(src) + " to " +
                                        filename_to_str(target),
                                    errno);
                }
            }
        }
        m_file_helper.reopen(true);

        // 写入html头
        spdlog::memory_buf_t htmlHeader;
        const char *pHtmlHeader =
            R"(<html>
                <head>
                <meta http-equiv="content-type" content="text/html; charset-gb2312">
                <title>Html Output</title>
                </head>
                <body>
                <font face="Fixedsys" size="2" color="#0000FF">)";

        htmlHeader.append(pHtmlHeader, pHtmlHeader + std::strlen(pHtmlHeader));
        m_file_helper.write(htmlHeader);
    }

    bool rename_file_(const spdlog::filename_t &srcFileName,
                      const spdlog::filename_t &targetFileName)
    {
        (void)spdlog::details::os::remove(targetFileName);
        return spdlog::details::os::rename(srcFileName, targetFileName) == 0;
    }

    constexpr const char *GetLogLevelHtmlPrefix(spdlog::level::level_enum level)
    {
        const char *pPrefix = "";
        switch (level)
        {
        case spdlog::level::trace:
            pPrefix = R"(<font color=" #DCDFE4">)";
            break;
        case spdlog::level::debug:
            pPrefix = R"(<font color=" #56B6C2">)";
            break;
        case spdlog::level::info:
            pPrefix = R"(<font color=" #1a85fd">)";
            break;
        case spdlog::level::warn:
            pPrefix = R"(<font color=" #E5C07B">)";
            break;
        case spdlog::level::err:
            pPrefix = R"(<font color=" #E06C75">)";
            break;
        case spdlog::level::critical:
            pPrefix =
                R"(<font color=" #DCDFE4" style="background-color:#E06C75;">)";
            break;
        case spdlog::level::off:
        case spdlog::level::n_levels:
            break;
        }

        return pPrefix;
    }

  private:
    spdlog::filename_t m_base_filename;
    std::size_t m_max_size;
    std::size_t m_max_files;
    std::size_t m_current_size;
    spdlog::details::file_helper m_file_helper;
};

using HtmlFormatSink_mt = HtmlFormatSink<std::mutex>;
using HtmlFormatSink_st = HtmlFormatSink<spdlog::details::null_mutex>;
} // namespace _Kits